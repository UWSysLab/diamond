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
	
	private TextView chatBox;
	private Diamond.DStringList messageList;
	private String userName;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_chat);
		
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
		int numLines = 12;
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
	
	private class MessageSender implements Runnable {
		String message;
		
		public MessageSender(String msg) {
			message = msg;
		}
		
		public void run() {
			String fullMsg = userName + ": " + message;
			Log.i(this.getClass().getName(), "ms start");
			messageList.Lock();
			Log.i(this.getClass().getName(), "ms locked");
			messageList.Append(fullMsg);
			if (messageList.Size() > 100) {
				messageList.Erase(0);
			}
			Log.i(this.getClass().getName(), "ms stuff done");
			messageList.Broadcast();
			Log.i(this.getClass().getName(), "ms broadcast");
			messageList.Unlock();
			Log.i(this.getClass().getName(), "ms unlock");
			refreshChatBox();
		}
	}
	
	private class BackgroundRefresher implements Runnable {
		private int internalSize = 0;
		
		public void run() {
			while (true) {
				Log.i(this.getClass().getName(), "br start");
				messageList.Lock();
				Log.i(this.getClass().getName(), "br locked");
				if (messageList.Size() == internalSize) {
					Log.i(this.getClass().getName(), "br waiting");
					messageList.Wait();
				}
				Log.i(this.getClass().getName(), "br done waiting");
				chatBox.post(new Runnable() {
					public void run() {
						refreshChatBox();
					}
				});
				Log.i(this.getClass().getName(), "br finished refresh");
				internalSize = messageList.Size();
				messageList.Unlock();
				Log.i(this.getClass().getName(), "br unlock");

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
