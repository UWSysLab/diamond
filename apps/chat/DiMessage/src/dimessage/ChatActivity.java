package dimessage;

import com.example.dimessage.R;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.ActionBarActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.EditText;
import android.widget.TextView;
import edu.washington.cs.diamond.Diamond.DString;

public class ChatActivity extends ActionBarActivity {
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_chat);
				
		TextView chatTextBox = (TextView)findViewById(R.id.chatTextBox);
		
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this.getBaseContext());
		String userName = prefs.getString(LoginActivity.PREFS_SCREENNAME, "AnonymousUser");
		
		ChatManager manager = new ChatManager(chatTextBox, userName);
		
		EditText entryTextBox = (EditText)findViewById(R.id.entryTextBox);		
		entryTextBox.setOnEditorActionListener(new EntryActionListener(manager));
	}
}
