package edu.washington.cs.diamond;

import java.io.*;
import java.util.*;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.eclipse.jetty.server.Request;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.server.handler.AbstractHandler;

import redis.clients.jedis.*;

public class RetwisServer {
	final int TIMEOUT = 10000;
	final String PUT_VALUE = "1";
	
	JedisPool pool;
	int numSlaves;
	int numFailures;

	boolean ready = false;
	double alpha = -1;
	double[] zipf;
	boolean readyUnwriteable = false;
	double[] zipfUnwriteable;
	
	List<String> keys;
	List<String> unwriteableKeys;
	Random random;
	
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
	
	int rand_unwriteable_key() {
		int nKeys = unwriteableKeys.size();
		
		if (alpha < 0) {
			return this.random.nextInt(nKeys);
		}
		else {
			if (!readyUnwriteable) {
				zipfUnwriteable = new double[nKeys];
				
				double c = 0.0;
				for (int i = 1; i < nKeys; i++) {
					c = c + (1.0 / Math.pow(i, alpha));
				}
				c = 1.0 / c;
				
				double sum = 0.0;
				for (int i = 1; i <= nKeys; i++) {
					sum += (c / Math.pow(i,  alpha));
					zipfUnwriteable[i-1] = sum;
				}
				readyUnwriteable = true;
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
				if (random > zipfUnwriteable[mid]) {
					l = mid + 1;
				}
				else if (random < zipfUnwriteable[mid]) {
					r = mid - 1;
				}
				else {
					break;
				}
			}
			return mid;
		}
	}
	
	class TxnHandler extends AbstractHandler {

		@Override
		public void handle(String target, Request baseRequest, HttpServletRequest request, HttpServletResponse response)
				throws IOException, ServletException {

			int responseCode = HttpServletResponse.SC_OK;
			
			List<Integer> keyIdx = new ArrayList<Integer>();
			
			try(Jedis jedis = pool.getResource()) {
				if (target.equals("/txn1")) {
					keyIdx.add(rand_key());
					keyIdx.add(rand_key());
					keyIdx.add(rand_key());
					Collections.sort(keyIdx);
					
					String value = jedis.get(keys.get(keyIdx.get(0)));
					
					for (int i = 0; i < 3; i++) {
						jedis.set(keys.get(keyIdx.get(i)), PUT_VALUE);
						jedis.waitReplicas(numSlaves - numFailures, TIMEOUT);
					}
				}
				else if (target.equals("/txn2")) {
					keyIdx.add(rand_key());
					keyIdx.add(rand_key());
					Collections.sort(keyIdx);
					
					for (int i = 0; i < 2; i++) {
						String value = jedis.get(keys.get(keyIdx.get(i)));
						jedis.set(keys.get(keyIdx.get(i)), PUT_VALUE);
						jedis.waitReplicas(numSlaves - numFailures, TIMEOUT);
					}
				}
				else if (target.equals("/txn3")) {
					keyIdx.add(rand_key());
					keyIdx.add(rand_key());
					keyIdx.add(rand_key());
					keyIdx.add(rand_key());
					keyIdx.add(rand_key());
					keyIdx.add(rand_key());
					Collections.sort(keyIdx);
					
					int unwriteableKeyIdx = rand_unwriteable_key();
					String unwriteableValue = jedis.get(unwriteableKeys.get(unwriteableKeyIdx));
					
					for (int i = 0; i < 5; i++) {
						String value = jedis.get(keys.get(keyIdx.get(i)));
						jedis.set(keys.get(keyIdx.get(i)), PUT_VALUE);
						jedis.waitReplicas(numSlaves - numFailures, TIMEOUT);
					}
					jedis.set(keys.get(keyIdx.get(5)), PUT_VALUE);
					jedis.waitReplicas(numSlaves - numFailures, TIMEOUT);
				}
				else if (target.equals("/txn4")) {
					int nGets = 1 + random.nextInt(10);
					for (int i = 0; i < nGets; i++) {
						keyIdx.add(rand_key());
					}
					
					Collections.sort(keyIdx);
					for (int i = 0; i < nGets; i++) {
						String value = jedis.get(keys.get(keyIdx.get(i)));
					}
				}
				else if (target.equals("/txn5")) {
					keyIdx.add(rand_key());
					Collections.sort(keyIdx);
					
					String value = jedis.get(keys.get(keyIdx.get(0)));
					jedis.set(keys.get(keyIdx.get(0)), PUT_VALUE);
					jedis.waitReplicas(numSlaves - numFailures, TIMEOUT);
				}
				else {
					responseCode = HttpServletResponse.SC_BAD_REQUEST;
				}
			}
			
			response.setStatus(responseCode);
			PrintWriter out = response.getWriter();
			out.print("OK");

			baseRequest.setHandled(true);
		}
	}

	public void start(int port, String redisHostname, int redisPort, int numSlaves, int numFailures, String keyFile, int numKeys,
			double zipfCoeff) {
		this.numSlaves = numSlaves;
		this.numFailures = numFailures;
		this.alpha = zipfCoeff;
		this.random = new Random();
		
		List<String> allKeys = Utils.parseKeys(keyFile, numKeys);
		int numUnwriteableKeys = allKeys.size() / 10;
		this.unwriteableKeys = allKeys.subList(0, numUnwriteableKeys);
		this.keys = allKeys.subList(numUnwriteableKeys, allKeys.size());
		
		pool = new JedisPool(new JedisPoolConfig(), redisHostname, redisPort);
		Server server = null;

		try {
			server = new Server(port);
			server.setHandler(new TxnHandler());
			server.start();
			server.join();
		}
		catch (Exception e) {
			e.printStackTrace();
			System.exit(1);
		}
		pool.destroy();
	}

	public static void main(String[] args) {
		if (args.length < 8) {
			System.err.println("usage: java RetwisServer port redis-hostname redis-port num-slaves num-failures key-file num-keys zipf");
			System.exit(1);
		}
		new RetwisServer().start(Integer.parseInt(args[0]), args[1], Integer.parseInt(args[2]),
				Integer.parseInt(args[3]), Integer.parseInt(args[4]), args[5], Integer.parseInt(args[6]),
				Double.parseDouble(args[7]));
	}
}
