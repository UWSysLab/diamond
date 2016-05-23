package edu.washington.cs.diamond;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;


public class RetwisClient extends Client {
	boolean ready = false;
	double alpha = -1;
	double[] zipf;
	
	public RetwisClient(String configFile, String keyFile, int numKeys) {
		super(configFile, keyFile, numKeys);
	}
	
	public void runTxnOnServer(int txnNum) {
		int responseCode = -1;
		String response = null;
		try {
			URL serverURL = new URL("http://" + server + "/txn" + txnNum);
			HttpURLConnection connection = (HttpURLConnection)serverURL.openConnection();
			BufferedReader in = new BufferedReader(new InputStreamReader(connection.getInputStream()));
			response = in.readLine();
			in.close();
			responseCode = connection.getResponseCode();
		} catch (MalformedURLException e) {
			System.err.println("Error: malformed URL");
			System.exit(1);
		} catch (IOException e) {
			System.err.println("Error connecting to server: " + e);
			System.exit(1);
		}
		if (responseCode != 200 || !response.equals("OK")) {
			System.err.println("HTTP error: response code " + responseCode + ", response " + response);
			System.exit(1);
		}
	}
	
	public void start(int seconds, boolean printKeys) {
		double t0;
		double t1;
		double t2;
		int nTransactions = 0;
		int ttype;
		List<Integer> keyIdx = new ArrayList<Integer>();
		
		t0 = System.currentTimeMillis() / 1000.0;
		
		while(true) {
			keyIdx.clear();
			
			t1 = System.currentTimeMillis() / 1000.0;
			
			ttype = random.nextInt(100);
			
			if (ttype < 1) {
				// 1% - Add user transaction. 1,3
				runTxnOnServer(1);
				ttype = 1;
			}
			else if (ttype < 6) {
				// 5% - Follow/Unfollow transaction. 2,2
				runTxnOnServer(2);
				ttype = 2;
			}
			else if (ttype < 30) {
				// 24% - Post tweet transaction. 3,5
				runTxnOnServer(3);
				ttype = 3;
			}
			else if (ttype < 80) {
				// 50% - Get followers/timeline transaction. rand(1,10),0
				runTxnOnServer(4);
				ttype = 4;
			}
			else {
				// 20% - Like transaction. 1,1
				runTxnOnServer(5);
				ttype = 5;
			}
			
			t2 = System.currentTimeMillis() / 1000.0;
			
			long latency = (long)((t2 - t1) * 1000000);
			
			System.out.printf("%d %f %f %d %d %d\n", ++nTransactions, t1, t2, latency, 1, ttype);
			
			if ((t2 - t0) > seconds) {
				break;
			}
		}
		System.out.println("# Client exiting..");
	}

	public static void main(String[] args) {
		if (args.length < 4) {
			System.err.println("usage: java RetwisClient configFile keyFile numKeys numSeconds");
			System.exit(1);
		}
		new RetwisClient(args[0], args[1], Integer.parseInt(args[2])).start(Integer.parseInt(args[3]), false);
	}
}
