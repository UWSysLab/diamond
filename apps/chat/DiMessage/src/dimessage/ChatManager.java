package dimessage;

import java.util.ArrayList;
import java.util.List;

import android.widget.TextView;

public class ChatManager {
	private TextView chatBox;
	private List<String> messageList;
	String userName;
	
	public ChatManager(TextView tv, String name) {
		chatBox = tv;
		userName = name;
		
		messageList = new ArrayList<String>();
	}
	
	private void addToMessages(String msg) {
		messageList.add(msg);
		if (messageList.size() > 100) {
			messageList.remove(0);
		}
	}
	
	private void refreshChatBox() {
		StringBuilder sb = new StringBuilder();
		//int numLines = chatBox.getLineCount();
		int numLines = 12;
		int minLine = messageList.size() - numLines;
		if (minLine < 0) {
			minLine = 0;
		}
		for (int i = minLine; i < messageList.size(); i++) {
			sb.append(userName + ": " + messageList.get(i) + "\n");
		}
		chatBox.setText(sb.toString());
	}
	
	public void sendMessage(String msg) {
		addToMessages(msg);
		refreshChatBox();
	}
}
