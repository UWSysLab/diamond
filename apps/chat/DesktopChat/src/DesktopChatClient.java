import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import edu.washington.cs.diamond.Diamond;

public class DesktopChatClient {
	
	static final int MESSAGE_LIST_SIZE = 10;
	static final int NUM_ACTIONS = 1000;
	static final String MESSAGE = "Help, I'm trapped in a Diamond benchmark";
	
	static final int INITIAL_CAPACITY = 20000;
	
	static final int ACTION_READ = 0;
	static final int ACTION_WRITE = 1;
	static final String RUN_TIMED = "timed";
	static final String RUN_FIXED = "fixed";
	
	static String chatroomName = "defaultroom";
	static String userName = "defaultclient";
	static String serverName = "coldwater.cs.washington.edu";
	static boolean verbose;
	
	private static Diamond.DStringList messageList;
	
	public static double[] writeMessageTransaction(String msg) {
		String fullMsg = userName + ": " + msg;
		int committed = 0;
		int numAborts = 0;
		long writeTimeStart = 0;
		long writeTimeEnd = 1000 * 1000 * 1000;
		while(committed == 0) {
			writeTimeStart = System.nanoTime();
			Diamond.DObject.TransactionBegin();
			messageList.Append(fullMsg);
			if (messageList.Size() > MESSAGE_LIST_SIZE) {
				messageList.Erase(0);
			}
			committed = Diamond.DObject.TransactionCommit();
			if (committed == 0) {
				numAborts++;
			}
		}
		writeTimeEnd = System.nanoTime();
		double time = ((double)(writeTimeEnd - writeTimeStart)) / (1000 * 1000);
		double[] ret = new double[2];
		ret[0] = time;
		ret[1] = numAborts;
		return ret;
	}
	
	public static double writeMessageAtomic(String msg) {
		String fullMsg = userName + ": " + msg;
		long writeTimeStart = 0;
		long writeTimeEnd = 1000 * 1000 * 1000;
		writeTimeStart = System.nanoTime();
		messageList.Append(fullMsg);
		if (messageList.Size() > MESSAGE_LIST_SIZE) {
			messageList.Erase(0);
		}
		writeTimeEnd = System.nanoTime();
		double time = ((double)(writeTimeEnd - writeTimeStart)) / (1000 * 1000);
		return (time);
	}
	
	public static double[] readMessagesTransaction() {
		List<String> result = null;
		int committed = 0;
		int numAborts = 0;
		long readTimeStart = 0;
		long readTimeEnd = 1000 * 1000 * 1000;
		while (committed == 0) {
			readTimeStart = System.nanoTime();
			Diamond.DObject.TransactionBegin();
			result = messageList.Members();
			if (result.get(0).indexOf(MESSAGE) == -1) {
				System.err.println("Error: first item of chat log is " + result.get(0));
				System.exit(1);
			}
			committed = Diamond.DObject.TransactionCommit();
			if (committed == 0) {
				numAborts++;
			}
		}
		readTimeEnd = System.nanoTime();
		double time = ((double)(readTimeEnd - readTimeStart)) / (1000 * 1000);
		double[] ret = new double[2];
		ret[0] = time;
		ret[1] = numAborts;
		return ret;
	}
	
	public static double readMessagesAtomic() {
		List<String> result = null;
		long readTimeStart = 0;
		long readTimeEnd = 1000 * 1000 * 1000;
		readTimeStart = System.nanoTime();
		result = messageList.Members();
		if (result.get(0).indexOf(MESSAGE) == -1) {
			System.err.println("Error: first item of chat log is " + result.get(0));
			System.exit(1);
		}
		readTimeEnd = System.nanoTime();
		double time = ((double)(readTimeEnd - readTimeStart)) / (1000 * 1000);
		return time;
	}
	
	public static void main(String[] args) {
		String usage = "usage: java Main run_type run_number read_fraction concurrency verbosity server_url client_name chatroom_name staleness stalelimit warmup_time\n"
					 + "    run_type: timed or fixed\n"
					 + "    run_number: the number of seconds (if timed) or the number of actions (if fixed)\n"
		 			 + "    read_fraction: decimal between 0 and 1 giving proportion of reads\n"
		 			 + "    concurrency: transaction or atomic\n"
		 			 + "    verbosity: concise or verbose\n"
		 			 + "    staleness: stale or nostale\n"
		 			 + "    stalelimit: stale read allowance in ms (0 means no limit)\n"
		 			 + "    warmup_time: warmup time in ms\n";
		if (args.length < 11) {
			System.err.print(usage);
			System.exit(0);
		}
		String runType = args[0];
		int runNumber = Integer.parseInt(args[1]);
		double readFraction = Double.parseDouble(args[2]);
		String concurrency = args[3];
		String verbosity = args[4];
		serverName = args[5];
		userName = args[6];
		chatroomName = args[7];
		String staleness = args[8];
		long stalelimit = Long.parseLong(args[9]);
		int warmupTime = Integer.parseInt(args[10]);
		if (!(runType.equals(RUN_TIMED) || runType.equals(RUN_FIXED))) {
			System.err.println("Error: run_type must be timed or fixed");
			System.exit(0);
		}
		if (readFraction > 1.0 || readFraction < 0.0) {
			System.err.println("Error: read_fraction must be between 0 and 1");
			System.exit(0);
		}
		if (!(concurrency.equals("transaction") || concurrency.equals("atomic"))) {
			System.err.println("Error: concurrency must be transaction or atomic");
			System.exit(0);
		}
		if (!(verbosity.equals("verbose") || verbosity.equals("concise"))) {
			System.err.println("Error: verbosity must be concise or verbose");
			System.exit(0);
		}
		if (!(staleness.equals("stale") || staleness.equals("nostale"))) {
			System.err.println("Error: staleness must be stale or nostale");
			System.exit(0);
		}
		
		verbose = (verbosity.equals("verbose"));
		
		Diamond.DiamondInit(serverName);
		Diamond.DObject.SetGlobalRedisWait(true, 1, 3);
		Diamond.DObject.SetGlobalStaleness(staleness.equals("stale"));
		Diamond.DObject.SetGlobalMaxStaleness(stalelimit);
		
		String chatLogKey = "dimessage:" + chatroomName + ":chatlog";
		messageList = new Diamond.DStringList();
		Diamond.DObject.Map(messageList, chatLogKey);
		
		Random rand = new Random();
		
		//warm up the JVM
		long warmupStartTime = System.nanoTime();
		while (true) {
			int action = rand.nextDouble() < readFraction ? ACTION_READ : ACTION_WRITE;
			if (concurrency.equals("transaction")) {
				if (action == ACTION_READ) {
					readMessagesTransaction();
				}
				else {
					writeMessageTransaction(MESSAGE);
				}
			}
			else if (concurrency.equals("atomic")) {
				if (action == ACTION_READ) {
					readMessagesAtomic();
				}
				else {
					writeMessageAtomic(MESSAGE);
				}
			}
			long currentTime = System.nanoTime();
			double elapsedTimeMillis = ((double)(currentTime - warmupStartTime)) / (1000 * 1000);
			if (elapsedTimeMillis >= warmupTime) {
				break;
			}
		}		

		long startTime = System.nanoTime();
		
		long numActions = 0;
		double totalTime = 0;
		double totalNumAborts = 0;
		
		List<Double> times = new ArrayList<Double>(INITIAL_CAPACITY);
		List<String> actions = new ArrayList<String>(INITIAL_CAPACITY);
		List<Double> numAborts = new ArrayList<Double>(INITIAL_CAPACITY);

		while (true) {
			int action = rand.nextDouble() < readFraction ? ACTION_READ : ACTION_WRITE;
			if (concurrency.equals("transaction")) {
				if (action == ACTION_READ) {
					double[] ret = readMessagesTransaction();
					times.add(ret[0]);
					actions.add("read");
					numAborts.add(ret[1]);
					totalTime += ret[0];
					totalNumAborts += ret[1];
				}
				else {
					double[] ret = writeMessageTransaction(MESSAGE);
					times.add(ret[0]);
					actions.add("write");
					numAborts.add(ret[1]);
					totalTime += ret[0];
					totalNumAborts += ret[1];
				}
			}
			else if (concurrency.equals("atomic")) {
				if (action == ACTION_READ) {
					double time = readMessagesAtomic();
					times.add(time);
					actions.add("read");
					totalTime += time;
				}
				else {
					double time = writeMessageAtomic(MESSAGE);
					times.add(time);
					actions.add("write");
					totalTime += time;
				}
			}
			numActions++;
			long currentTime = System.nanoTime();
			double elapsedTimeMillis = ((double)(currentTime - startTime)) / (1000 * 1000);
			if (runType.equals(RUN_TIMED) && elapsedTimeMillis / 1000 >= runNumber) {
				break;
			}
			if (runType.equals(RUN_FIXED) && numActions >= runNumber) {
				break;
			}
		}
		
		long endTime = System.nanoTime();
		double elapsedTimeMillis = ((double)(endTime - startTime)) / (1000 * 1000);
		
		double averageTime = ((double)totalTime) / numActions;
		
		//kill time at the end for the sake of the throughput experiment
		int cooldownTime = warmupTime;
		long cooldownStartTime = System.nanoTime();
		while (true) {
			int action = rand.nextDouble() < readFraction ? ACTION_READ : ACTION_WRITE;
			if (concurrency.equals("transaction")) {
				if (action == ACTION_READ) {
					readMessagesTransaction();
				}
				else {
					writeMessageTransaction(MESSAGE);
				}
			}
			else if (concurrency.equals("atomic")) {
				if (action == ACTION_READ) {
					readMessagesAtomic();
				}
				else {
					writeMessageAtomic(MESSAGE);
				}
			}
			long currentTime = System.nanoTime();
			double cooldownElapsedTimeMillis = ((double)(currentTime - cooldownStartTime)) / (1000 * 1000);
			if (cooldownElapsedTimeMillis >= cooldownTime) {
				break;
			}
		}
		
		if (verbose) {
			for (int i = 0; i < times.size(); i++) {
				System.out.print(userName + "\t" + chatroomName + "\t" + actions.get(i) + "\t" + times.get(i) + "\t" + concurrency);
				if (concurrency.equals("transaction")) {
					System.out.print("\t" + numAborts.get(i));
				}
				System.out.println();
			}
		}
		
		System.out.print("Summary: " + userName + "\t" + chatroomName + "\t" + numActions + "\t" + averageTime + "\t" + elapsedTimeMillis + "\t" + concurrency);
		if (concurrency.equals("transaction")) {
			double averageAborts = ((double)totalNumAborts) / numActions;
			System.out.print("\t" + averageAborts);
		}
		System.out.println();
	}
}