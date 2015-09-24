import java.util.List;

import edu.washington.cs.diamond.Diamond;

public class Main {
	
	static final int MESSAGE_LIST_SIZE = 100;
	static final int NUM_ACTIONS = 1000;
	
	static final int ACTION_READ = 0;
	static final int ACTION_WRITE = 1;
	
	static String chatLogKey = "desktopchat:defaultroom:chatlog";
	static String readCountKey = "desktopchat:defaultroom:readcount";
	static String userName = "defaultclient";
	static String serverName = "coldwater.cs.washington.edu";
	
	private static Diamond.DStringList messageList;
	private static Diamond.DCounter readCount;
	private static long internalCount;
	
	public static void writeMessage(String msg) {
		String fullMsg = userName + ": " + msg;
		int committed = 0;
		while(committed == 0) {
			Diamond.DObject.TransactionBegin();
			messageList.Append(fullMsg);
			if (messageList.Size() > MESSAGE_LIST_SIZE) {
				messageList.Erase(0);
			}
			committed = Diamond.DObject.TransactionCommit();
		}
	}
	
	public static List<String> readMessages() {
		List<String> result = null;
		boolean didRead = false;
		while (!didRead) {
			Diamond.DObject.TransactionBegin();
			if (readCount.Value() == internalCount) {
				Diamond.DObject.TransactionRetry();
				continue;
			}
			result = messageList.Members();
			readCount.Increment();
			didRead = true;
			internalCount = readCount.Value();
			Diamond.DObject.TransactionCommit();
		}
		return result;
	}
	
	public static void main(String[] args) {
		Diamond.DiamondInit(serverName);
		
		messageList = new Diamond.DStringList();
		readCount = new Diamond.DCounter();
		Diamond.DObject.Map(messageList, chatLogKey);
		Diamond.DObject.Map(readCount, readCountKey);
		
		for (int i = 0; i < NUM_ACTIONS; i++) {
			int action = ACTION_WRITE;
			if (action == ACTION_READ) {
				readMessages();
			}
			else {
				writeMessage("Hello " + System.currentTimeMillis());
			}
		}
	}
}