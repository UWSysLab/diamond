package dimessage;

import android.view.KeyEvent;
import android.widget.EditText;
import android.widget.TextView;

public class EntryActionListener implements EditText.OnEditorActionListener {

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