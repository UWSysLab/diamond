package dimessage;

import com.example.dimessage.R;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.TextView;
import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.Diamond.DString;

public class ChatActivity extends ActionBarActivity {
	
	final int NUM_LINES = 11;
	final int MESSAGE_LIST_SIZE = 100;
	
	private TextView chatBox;
	private Diamond.DStringList messageList;
	private String userName;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_chat);
		
		Diamond.DiamondInit("moranis.cs.washington.edu");
		
		messageList = new Diamond.DStringList("dimessage:messagelist");
		
		chatBox = (TextView)findViewById(R.id.chatTextBox);
		
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this.getBaseContext());
		userName = prefs.getString(LoginActivity.PREFS_SCREENNAME, "AnonymousUser");
				
		EditText entryTextBox = (EditText)findViewById(R.id.entryTextBox);		
		entryTextBox.setOnEditorActionListener(new EntryActionListener(this));
		
		refreshChatBox();
		
		new Thread(new BackgroundRefresher()).start();
	}
	
	private void refreshChatBox() {
		StringBuilder sb = new StringBuilder();
		int numLines = NUM_LINES;
		int minLine = messageList.Size() - numLines;
		if (minLine < 0) {
			minLine = 0;
		}
		for (int i = minLine; i < messageList.Size(); i++) {
			sb.append(messageList.Value(i) + "\n");
		}
		chatBox.setText(sb.toString());
	}
	
	public void sendMessage(String msg) {
		new Thread(new MessageSender(msg)).start();
	}
	
	@Override
	public void onBackPressed() {
		
	}
	
	private class MessageSender implements Runnable {
		String message;
		
		public MessageSender(String msg) {
			message = msg;
		}
		
		public void run() {
			String fullMsg = userName + ": " + message;
			int committed = 0;
			while(committed == 0) {
				Diamond.DObject.TransactionBegin();
				messageList.Append(fullMsg);
				if (messageList.Size() > MESSAGE_LIST_SIZE) {
					messageList.Erase(0);
				}
				committed = Diamond.DObject.TransactionCommit();
			}
		}
	}
	
	private class BackgroundRefresher implements Runnable {
		private int internalSize = 0;
		
		public void run() {
			while (true) {
				Diamond.DObject.TransactionBegin();
				if (messageList.Size() == internalSize) {
					Diamond.DObject.TransactionRetry();
					continue;
				}
				chatBox.post(new Runnable() {
					public void run() {
						refreshChatBox();
					}
				});
				internalSize = messageList.Size();
				Diamond.DObject.TransactionCommit();
			}
		}
	}
	
	private class EntryActionListener implements EditText.OnEditorActionListener {
		private ChatActivity activity;
		
		public EntryActionListener(ChatActivity ca) {
			activity = ca;
		}
		
		@Override
		public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
			EditText ev = (EditText)v;
			activity.sendMessage(ev.getText().toString());
			ev.setText("");
			return true;
		}
	}
}
