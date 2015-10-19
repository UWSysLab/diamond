import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;

public class BaselineChatClient {
	
	static final String MESSAGE = "Help, I'm trapped in a Diamond benchmark";
	static final int PORT = 9000;
	
	static final int INITIAL_CAPACITY = 20000;
	
	static final int ACTION_READ = 0;
	static final int ACTION_WRITE = 1;
	static final String RUN_TIMED = "timed";
	static final String RUN_FIXED = "fixed";
	
	static String serverURLString;
	static String userName;
	static boolean verbose;
	
	public static double writeMessage(String msg) {
		String fullMsg = userName + ": " + msg;
		int responseCode = -1;
		long startTime = System.nanoTime();
		try {
			URL serverURL = new URL(serverURLString);
			HttpURLConnection connection = (HttpURLConnection)serverURL.openConnection();
			connection.setRequestMethod("POST");
			connection.setDoOutput(true);
			OutputStreamWriter out = new OutputStreamWriter(connection.getOutputStream());
			out.write(fullMsg);
			out.close();
			responseCode = connection.getResponseCode();
		} catch (MalformedURLException e) {
			System.out.println("Error: malformed URL");
			System.exit(1);
		} catch (IOException e) {
			System.out.println("Error connecting to server: " + e);
			System.exit(1);
		}
		if (responseCode != 200) {
			System.out.println("HTTP error: " + responseCode);
			System.exit(1);
		}
		long endTime = System.nanoTime();
		double time = ((double)(endTime - startTime)) / (1000 * 1000);
		return time;
	}
	public static double readMessages() {
		int responseCode = -1;
		long startTime = System.nanoTime();
		try {
			URL serverURL = new URL(serverURLString);			
			HttpURLConnection connection = (HttpURLConnection)serverURL.openConnection();
			BufferedReader in = new BufferedReader(new InputStreamReader(connection.getInputStream()));
			String jsonStr = in.readLine();
			JsonParser parser = new JsonParser();
			JsonArray jsonArray = parser.parse(jsonStr).getAsJsonArray();
			List<String> result = new ArrayList<String>();
			for (int i = 0; i < jsonArray.size(); i++) {
				JsonElement item = jsonArray.get(i);
				result.add(item.getAsString());
			}
			in.close();
			responseCode = connection.getResponseCode();
		} catch (MalformedURLException e) {
			System.out.println("Error: malformed URL");
			System.exit(1);
		} catch (IOException e) {
			System.out.println("Error connecting to server");
			System.exit(1);
		}
		if (responseCode != 200) {
			System.out.println("HTTP error: " + responseCode);
			System.exit(1);
		}
		long endTime = System.nanoTime();
		double time = ((double)(endTime - startTime)) / (1000 * 1000);
		return time;
	}

	
	public static void main(String[] args) {
		String usage = "usage: java BaselineChatClient run_type run_number read_fraction verbosity server_url user_name warmup_time\n"
				 + "    run_type: timed or fixed\n"
				 + "    run_number: the number of seconds (if timed) or the number of actions (if fixed)\n"
	 			 + "    read_fraction: decimal between 0 and 1 giving proportion of reads\n"
	 			 + "    verbosity: concise or verbose\n"
	 			 + "    warmup_time: warmup time in ms";
		if (args.length < 7) {
			System.err.println(usage);
			System.exit(0);
		}
		String runType = args[0];
		int runNumber = Integer.parseInt(args[1]);
		double readFraction = Double.parseDouble(args[2]);
		String verbosity = args[3];
		String serverName = args[4];
		userName = args[5];
		int warmupTime = Integer.parseInt(args[6]);
		
		if (!(runType.equals(RUN_TIMED) || runType.equals(RUN_FIXED))) {
			System.err.println(usage);
			System.exit(0);
		}
		if (readFraction > 1.0 || readFraction < 0.0) {
			System.err.println(usage);
			System.exit(0);
		}
		if (!(verbosity.equals("verbose") || verbosity.equals("concise"))) {
			System.err.println(usage);
			System.exit(0);
		}
		
		verbose = (verbosity.equals("verbose"));
		serverURLString = "http://" + serverName + ":" + PORT + "/chat";

		Random rand = new Random();
		
		//warm up the JVM
		long warmupStartTime = System.nanoTime();
		while (true) {
			int action = rand.nextDouble() < readFraction ? ACTION_READ : ACTION_WRITE;
			if (action == ACTION_READ) {
				readMessages();
			}
			else {
				writeMessage(MESSAGE);
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
		
		List<Double> times = new ArrayList<Double>(INITIAL_CAPACITY);
		List<String> actions = new ArrayList<String>(INITIAL_CAPACITY);

		while (true) {
			int action = rand.nextDouble() < readFraction ? ACTION_READ : ACTION_WRITE;
			if (action == ACTION_READ) {
				double time = readMessages();
				times.add(time);
				actions.add("read");
				totalTime += time;
			}
			else {
				double time = writeMessage(MESSAGE);
				times.add(time);
				actions.add("write");
				totalTime += time;
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
		
		if (verbose) {
			for (int i = 0; i < times.size(); i++) {
				System.out.println(userName + "\t" + actions.get(i) + "\t" + times.get(i));
			}
		}
		
		System.out.print("Summary: " + userName + "\t" + numActions + "\t" + averageTime + "\t" + elapsedTimeMillis);
		System.out.println();
	}
}
