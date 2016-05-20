package diamondchat;

import java.util.Date;

import com.example.dimessage.R;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.EditText;
import android.widget.TextView;
import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.ReactiveManager;

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
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this.getBaseContext());
		
		Diamond.DiamondInit("128.208.6.132", "12444");
		ReactiveManager.StartManager();
		ReactiveManager.RegisterLogger(new ReactiveManager.Logger() {
			public void onLog(String message) {
				Log.i("DiMessage", message);
			}
		});
		
		chatBox = (TextView)findViewById(R.id.chatTextBox);
		messageList = new Diamond.DStringList("dimessage:messagelist");
		userName = prefs.getString(LoginActivity.PREFS_SCREENNAME, "AnonymousUser");
		
		EditText entryTextBox = (EditText)findViewById(R.id.entryTextBox);
		entryTextBox.setOnEditorActionListener(new EditText.OnEditorActionListener() {
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
				EditText ev = (EditText)v;
				new Thread(new MessageSender(ev.getText().toString())).start();
				ev.setText("");
				return true;
			}
		});
		
		DiamondTextView lastUserBox = (DiamondTextView)findViewById(R.id.lastUserTextBox);
		lastUserBox.bindDString("dimessage:lastuser");
		
		DiamondTextView timestampBox = (DiamondTextView)findViewById(R.id.timestampTextBox);
		timestampBox.bindDString("dimessage:lasttimestamp");
		
		DiamondCheckBox checkBox = (DiamondCheckBox)findViewById(R.id.checkBox);
		checkBox.bindDBoolean("dimessage:globalboolean");
		
		ReactiveManager.reactive_txn(new ReactiveManager.TxnFunction() {
			public void func(Object...objects) {
				//Perform Diamond accesses
				StringBuilder sb = new StringBuilder();
				int minLine = messageList.Size() - NUM_LINES;
				if (minLine < 0) {
					minLine = 0;
				}
				for (int i = minLine; i < messageList.Size(); i++) {
					sb.append(messageList.Value(i) + "\n");
				}
				final String text = sb.toString();
				
				//Update UI
				runOnUiThread(new Runnable() {
					public void run() {
						chatBox.setText(text);
					}
				});
			}
		});
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
				Diamond.DString timestampString = new Diamond.DString();
				Diamond.DObject.Map(timestampString, "dimessage:lasttimestamp");
				timestampString.Set(new Date().toString());
				committed = Diamond.DObject.TransactionCommit();
			}
		}
	}
	
	@Override
	public void onResume() {
		super.onResume();
		Diamond.DString lastUserString = new Diamond.DString();
		int committed = 0;
		while (committed == 0) {
			Diamond.DObject.TransactionBegin();
			Diamond.DObject.Map(lastUserString, "dimessage:lastuser");
			lastUserString.Set(userName);
			committed = Diamond.DObject.TransactionCommit();
		}
	}
	
	@Override
	public void onBackPressed() {}
}
