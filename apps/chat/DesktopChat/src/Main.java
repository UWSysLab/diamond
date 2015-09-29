import java.util.List;
import java.util.Random;

import edu.washington.cs.diamond.Diamond;

public class Main {
	
	static final int MESSAGE_LIST_SIZE = 100;
	static final int NUM_ACTIONS = 1000;
	static final String MESSAGE = "Help, I'm trapped in a Diamond benchmark";
	
	static final int ACTION_READ = 0;
	static final int ACTION_WRITE = 1;
	static final String RUN_TIMED = "timed";
	static final String RUN_FIXED = "fixed";
	
	static String chatroomName = "defaultroom";
	static String userName = "defaultclient";
	static String serverName = "coldwater.cs.washington.edu";
	
	private static Diamond.DStringList messageList;
	private static Diamond.DLong updateTime;
	
	private static long numActions = 0;
	private static long totalTime = 0;
	
	public static long writeMessage(String msg) {
		String fullMsg = userName + ": " + msg;
		int committed = 0;
		long writeTimeStart = 0;
		long writeTimeEnd = 1000 * 1000 * 1000;
		while(committed == 0) {
			writeTimeStart = System.currentTimeMillis();
			Diamond.DObject.TransactionBegin();
			messageList.Append(fullMsg);
			if (messageList.Size() > MESSAGE_LIST_SIZE) {
				messageList.Erase(0);
			}
			updateTime.Set(System.currentTimeMillis());
			committed = Diamond.DObject.TransactionCommit();
		}
		writeTimeEnd = System.currentTimeMillis();
		return (writeTimeEnd - writeTimeStart);
	}
	
	public static long readMessages() {
		List<String> result = null;
		int committed = 0;
		long readTimeStart = 0;
		long readTimeEnd = 1000 * 1000 * 1000;
		while (committed == 0) {
			readTimeStart = System.currentTimeMillis();
			Diamond.DObject.TransactionBegin();
			result = messageList.Members();
			committed = Diamond.DObject.TransactionCommit();
		}
		readTimeEnd = System.currentTimeMillis();
		return (readTimeEnd - readTimeStart);
	}
	
	public static void main(String[] args) {
		String usage = "usage: java Main run_type run_number read_fraction [client_name] [chatroom_name]\n"
					 + "    run_type: timed or fixed\n"
					 + "    run_number: the number of seconds (if timed) or the number of actions (if fixed)\n"
		 			 + "    read_fraction: decimal between 0 and 1 giving proportion of reads";
		if (args.length < 3) {
			System.err.println(usage);
			System.exit(0);
		}
		String runType = args[0];
		int runNumber = Integer.parseInt(args[1]);
		double readFraction = Double.parseDouble(args[2]);
		if (args.length >= 4) {
			userName = args[3];
		}
		if (args.length >= 5) {
			chatroomName = args[4];
		}
		if (!(runType.equals(RUN_TIMED) || runType.equals(RUN_FIXED))) {
			System.err.println(usage);
			System.exit(0);
		}
		if (readFraction > 1.0 || readFraction < 0.0) {
			System.err.println(usage);
			System.exit(0);
		}
		
		Diamond.DiamondInit(serverName);
		
		String chatLogKey = "dimessage:" + chatroomName + ":chatlog";
		String updateTimeKey = "dimessage:" + chatroomName + ":updatetime";
		
		messageList = new Diamond.DStringList();
		updateTime = new Diamond.DLong();
		Diamond.DObject.Map(messageList, chatLogKey);
		Diamond.DObject.Map(updateTime, updateTimeKey);
		
		Random rand = new Random();
		
		// Take 200 initial actions to warm up the JVM
		for (int i = 0; i < 200; i++) {
			int action = rand.nextDouble() < readFraction ? ACTION_READ : ACTION_WRITE;
			if (action == ACTION_READ) {
				 readMessages();
			}
			else {
				writeMessage(MESSAGE);
			}
		}
		
		long startTime = System.currentTimeMillis();
		numActions = 0;
		while (true) {
			int action = rand.nextDouble() < readFraction ? ACTION_READ : ACTION_WRITE;
			if (action == ACTION_READ) {
				totalTime += readMessages();
			}
			else {
				totalTime += writeMessage(MESSAGE);
			}
			numActions++;
			long currentTime = System.currentTimeMillis();
			if (runType.equals(RUN_TIMED) && (currentTime - startTime) / 1000 > runNumber) {
				break;
			}
			if (runType.equals(RUN_FIXED) && numActions >= runNumber) {
				break;
			}
		}
		
		double averageTime = ((double)totalTime) / numActions;
		System.out.println(userName + "\t" + chatroomName + "\t" + numActions + "\t" + averageTime);
	}
}