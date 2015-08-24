package dimessage;

import android.view.KeyEvent;
import android.widget.EditText;
import android.widget.TextView;

public class EntryActionListener implements EditText.OnEditorActionListener {

	private ChatManager manager;
	
	public EntryActionListener(ChatManager cm) {
		manager = cm;
	}
	
	public EntryActionListener() {
		manager = null;
	}
	
	@Override
	public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
		EditText ev = (EditText)v;
		manager.sendMessage(ev.getText().toString());
		ev.setText("");
		return true;
	}

}
