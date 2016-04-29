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
	
	List<String> readKeys;
	List<String> writeKeys;
	List<String> incrementKeys;
	
	RandomIndexGen readRandom;
	RandomIndexGen writeRandom;
	RandomIndexGen incrementRandom;
	
	private class RandomIndexGen { // encapsulate zipf code
		Random random;
		double[] zipf;
		int nKeys;
		double alpha;
		
		public RandomIndexGen(int nKeys, double alpha) {
			random = new Random();
			this.nKeys = nKeys;
			this.alpha = alpha;
			zipf = new double[nKeys];
			
			//initialize zipf dist
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
		}
		
		public int rand_key() {
			if (alpha < 0) {
				return this.random.nextInt(nKeys);
			}
			else {
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
	}
	
	class TxnHandler extends AbstractHandler {

		public void doReads(Jedis jedis, int numReads) {
			List<Integer> readKeyIdx = new ArrayList<Integer>();
			for (int i = 0; i < numReads; i++) {
				readKeyIdx.add(readRandom.rand_key());
			}
			Collections.sort(readKeyIdx);
			
			for (int i = 0; i < numReads; i++) {
				String value = jedis.get(readKeys.get(readKeyIdx.get(0)));
			}
		}
		
		public void doWrites(Jedis jedis, int numWrites) {
			List<Integer> writeKeyIdx = new ArrayList<Integer>();
			for (int i = 0; i < numWrites; i++) {
				writeKeyIdx.add(writeRandom.rand_key());
			}
			Collections.sort(writeKeyIdx);
			
			for (int i = 0; i < numWrites; i++) {
				jedis.set(writeKeys.get(writeKeyIdx.get(i)), PUT_VALUE);
				jedis.waitReplicas(numSlaves - numFailures, TIMEOUT);
			}
		}
		
		public void doIncrements(Jedis jedis, int numIncrements) {
			List<Integer> incrementKeyIdx = new ArrayList<Integer>();
			for (int i = 0; i < numIncrements; i++) {
				incrementKeyIdx.add(incrementRandom.rand_key());
			}
			Collections.sort(incrementKeyIdx);
			
			for (int i = 0; i < numIncrements; i++) {
				String value = jedis.get(incrementKeys.get(incrementKeyIdx.get(i)));
				jedis.set(incrementKeys.get(incrementKeyIdx.get(i)), PUT_VALUE);
				jedis.waitReplicas(numSlaves - numFailures, TIMEOUT);
			}
		}
		
		@Override
		public void handle(String target, Request baseRequest, HttpServletRequest request, HttpServletResponse response)
				throws IOException, ServletException {

			int responseCode = HttpServletResponse.SC_OK;
			
			try(Jedis jedis = pool.getResource()) {
				if (target.equals("/txn1")) {
					doIncrements(jedis, 1);
					doWrites(jedis, 2);
				}
				else if (target.equals("/txn2")) {
					doIncrements(jedis, 2);
				}
				else if (target.equals("/txn3")) {
					doReads(jedis, 1);
					doIncrements(jedis, 5);
					doWrites(jedis, 1);
				}
				else if (target.equals("/txn4")) {
					Random random = new Random();
					int nGets = 1 + random.nextInt(10);
					doReads(jedis, nGets);
				}
				else if (target.equals("/txn5")) {
					doIncrements(jedis, 1);
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
		
		List<String> allKeys = Utils.parseKeys(keyFile, numKeys);
		int numReadKeys = allKeys.size() / 10;
		int numIncrementKeys = 2 * (allKeys.size() / 10);
		this.readKeys = allKeys.subList(0, numReadKeys);
		this.incrementKeys = allKeys.subList(numReadKeys, numReadKeys + numIncrementKeys);
		this.writeKeys = allKeys.subList(numReadKeys + numIncrementKeys, allKeys.size());
		
		readRandom = new RandomIndexGen(readKeys.size(), zipfCoeff);
		writeRandom = new RandomIndexGen(writeKeys.size(), zipfCoeff);
		incrementRandom = new RandomIndexGen(incrementKeys.size(), zipfCoeff);
		
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
