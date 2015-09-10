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
		
		new Thread(new Runnable() {
			public void run() {
				while (true) {
					try {
						Thread.sleep(100);
					}
					catch (InterruptedException e) {
						Log.e(this.getClass().getName(), "Thread sleep interrupted");
					}
					chatBox.post(new Runnable() {
						public void run() {
							refreshChatBox();
						}
					});
				}
			}
		}).start();
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
		String fullMsg = userName + ": " + msg;
		messageList.Append(fullMsg);
		if (messageList.Size() > 100) {
			messageList.Erase(0);
		}
		refreshChatBox();
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
