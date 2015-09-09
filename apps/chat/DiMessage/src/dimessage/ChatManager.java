package dimessage;

import android.widget.TextView;
import edu.washington.cs.diamond.Diamond;

public class ChatManager {
	private TextView chatBox;
	private Diamond.DStringList messageList;
	String userName;
	
	public ChatManager(TextView tv, String name) {
		chatBox = tv;
		userName = name;
		messageList = new Diamond.DStringList("dimessage:messagelist");
		refreshChatBox();
	}
	
	private void addToMessages(String msg) {
		messageList.Append(msg);
		if (messageList.Size() > 100) {
			messageList.Erase(0);
		}
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
		addToMessages(userName + ": " + msg);
		refreshChatBox();
	}
}
