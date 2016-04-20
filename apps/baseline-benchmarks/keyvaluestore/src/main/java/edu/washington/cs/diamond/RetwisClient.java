package edu.washington.cs.diamond;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

public class RetwisClient extends Client {
	boolean ready = false;
	double alpha = -1;
	double[] zipf;
	
	public RetwisClient(String configFile, String keyFile, int numKeys) {
		super(configFile, keyFile, numKeys);
	}
	
	int rand_key() {
		int nKeys = keys.size();
		
		if (alpha < 0) {
			return this.random.nextInt(nKeys);
		}
		else {
			if (!ready) {
				zipf = new double[nKeys];
				
				double c = 0.0;
				for (int i = 1; i < nKeys; i++) {
					c = c + (1.0 / Math.pow(i, alpha));
				}
				c = 1.0 / c;
				
				double sum = 0.0;
				for (int i = 1; i <= nKeys; i++) {
					sum += (c / Math.pow(i,  alpha));
					zipf[i-1] = sum;
				}
				ready = true;
			}
			
			double random = 0.0;
			while (random == 0.0 || random == 1.0) {
				random = this.random.nextDouble(); 
			}
			
			int l = 0;
			int r = nKeys;
			int mid = 0;
			while (l < r) {
				mid = (l + r) / 2;
				if (random > zipf[mid]) {
					l = mid + 1;
				}
				else if (random < zipf[mid]) {
					r = mid - 1;
				}
				else {
					break;
				}
			}
			return mid;
		}
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
			System.out.println("Error: malformed URL");
			System.exit(1);
		} catch (IOException e) {
			System.out.println("Error connecting to server: " + e);
			System.exit(1);
		}
		if (responseCode != 200 || !response.equals("OK")) {
			System.out.println("HTTP error: response code " + responseCode + ", response " + response);
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
			
			if (ttype < 5) {
				// 15% - Follow/Unfollow transaction. 2,2
				runTxnOnServer(1);
				ttype = 1;
			}
			else if (ttype < 20) {
				// 30% - Post tweet transaction. 3,5
				runTxnOnServer(2);
				ttype = 2;
			}
			else if (ttype < 50) {
				// 30% - Post tweet transaction. 3,5
				runTxnOnServer(3);
				ttype = 3;
			}
			else {
				// 50% - Get followers/timeline transaction. rand(1,10),0
				runTxnOnServer(4);
				ttype = 4;
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
