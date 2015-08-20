package dimessage;

import android.widget.EditText;
import android.widget.TextView;

public class ChatManager {
	private TextView chatBox;
	private EditText entryBox;
	
	public ChatManager(TextView tv, EditText et) {
		chatBox = tv;
		entryBox = et;
	}
	
	public void sendMessage(String msg) {
		chatBox.setText(msg);
	}
}
