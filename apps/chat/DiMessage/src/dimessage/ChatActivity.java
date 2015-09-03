package dimessage;

import com.example.dimessage.R;

import android.os.Bundle;
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
		DString str1 = new DString("String1", "a");
		DString str2 = new DString("String2", "a");
		str1.Set("Testing Diamond");
		chatTextBox.setText(str2.Value());
		
		String userName = "DefaultUser";
		
		ChatManager manager = new ChatManager(chatTextBox, userName);
		
		EditText entryTextBox = (EditText)findViewById(R.id.entryTextBox);		
		entryTextBox.setOnEditorActionListener(new EntryActionListener(manager));

	}
}
