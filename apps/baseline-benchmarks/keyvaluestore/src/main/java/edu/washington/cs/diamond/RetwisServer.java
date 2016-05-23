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
	
	JedisPool[] pools;
	int[] numSlaves;
	int[] numFailures;
	int numShards;
	
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

		public void doReads(int numReads) {
			List<Integer> readKeyIdx = new ArrayList<Integer>();
			for (int i = 0; i < numReads; i++) {
				readKeyIdx.add(readRandom.rand_key());
			}
			Collections.sort(readKeyIdx);
			
			for (int i = 0; i < numReads; i++) {
				int shardNum = keyToShard(readKeys.get(readKeyIdx.get(i)), numShards);
				Jedis jedis = pools[shardNum].getResource();
				String value = jedis.get(readKeys.get(readKeyIdx.get(i)));
				jedis.close();
			}
		}
		
		public void doWrites(int numWrites) {
			List<Integer> writeKeyIdx = new ArrayList<Integer>();
			for (int i = 0; i < numWrites; i++) {
				writeKeyIdx.add(writeRandom.rand_key());
			}
			Collections.sort(writeKeyIdx);
			
			for (int i = 0; i < numWrites; i++) {
				int shardNum = keyToShard(writeKeys.get(writeKeyIdx.get(i)), numShards);
				Jedis jedis = pools[shardNum].getResource();
				jedis.set(writeKeys.get(writeKeyIdx.get(i)), PUT_VALUE);
				jedis.waitReplicas(numSlaves[shardNum] - numFailures[shardNum], TIMEOUT);
				jedis.close();
			}
		}
		
		public void doIncrements(int numIncrements) {
			List<Integer> incrementKeyIdx = new ArrayList<Integer>();
			for (int i = 0; i < numIncrements; i++) {
				incrementKeyIdx.add(incrementRandom.rand_key());
			}
			Collections.sort(incrementKeyIdx);
			
			for (int i = 0; i < numIncrements; i++) {
				int shardNum = keyToShard(incrementKeys.get(incrementKeyIdx.get(i)), numShards);
				Jedis jedis = pools[shardNum].getResource();
				String value = jedis.get(incrementKeys.get(incrementKeyIdx.get(i)));
				jedis.set(incrementKeys.get(incrementKeyIdx.get(i)), PUT_VALUE);
				jedis.waitReplicas(numSlaves[shardNum] - numFailures[shardNum], TIMEOUT);
				jedis.close();
			}
		}
		
		@Override
		public void handle(String target, Request baseRequest, HttpServletRequest request, HttpServletResponse response)
				throws IOException, ServletException {

			int responseCode = HttpServletResponse.SC_OK;
			
			if (target.equals("/txn1")) {
				doIncrements(1);
				doWrites(2);
			}
			else if (target.equals("/txn2")) {
				doIncrements(2);
			}
			else if (target.equals("/txn3")) {
				doReads(1);
				doIncrements(5);
				doWrites(1);
			}
			else if (target.equals("/txn4")) {
				Random random = new Random();
				int nGets = 1 + random.nextInt(10);
				doReads(nGets);
			}
			else if (target.equals("/txn5")) {
				doIncrements(1);
			}
			else {
				responseCode = HttpServletResponse.SC_BAD_REQUEST;
			}
			
			response.setStatus(responseCode);
			PrintWriter out = response.getWriter();
			out.print("OK");

			baseRequest.setHandled(true);
		}
	}
	
	private int keyToShard(String key, int nShards) {
		long hash = 5381;
		for (int i = 0; i < key.length(); i++) {
			hash = ((hash << 5) + hash) + (long)key.charAt(i);
		}
		hash = Math.abs(hash);
		return (int)(hash % nShards);
	}
	
	private void parseBackendConfigs(String configPrefix, int numShards, String[] redisHostnames, int[] redisPorts,
			int[] numSlaves, int[] numFailures) {
		for (int shardNum = 0; shardNum < numShards; shardNum++) {
			String configFileName = configPrefix + shardNum + ".config";
			int shardNumSlaves = 0;
			int shardNumFailures = 0;
			String shardRedisHostname = null;
			int shardRedisPort = 0;
			
			int numReplicas = 0;
			BufferedReader reader;
			try {
				reader = new BufferedReader(new FileReader(configFileName));
				String line;
				while((line = reader.readLine()) != null) {
					String[] lineSplit = line.split("\\s+");
					if (lineSplit[0].equals("f")) {
						shardNumFailures = Integer.parseInt(lineSplit[1]);
					}
					else if (lineSplit[0].equals("replica")) {
						numReplicas++;
						if (shardRedisHostname == null) {
							String[] replicaSplit = lineSplit[1].split(":");
							shardRedisHostname = replicaSplit[0];
							shardRedisPort = Integer.parseInt(replicaSplit[1]);
						}
					}
				}
				shardNumSlaves = numReplicas - 1;
			} catch (FileNotFoundException e) {
				System.err.println("Error: config file " + configFileName + " not found");
				System.exit(1);
			} catch (IOException e) {
				System.err.println(e);
				System.exit(1);
			}
			
			redisHostnames[shardNum] = shardRedisHostname;
			redisPorts[shardNum] = shardRedisPort;
			numSlaves[shardNum] = shardNumSlaves;
			numFailures[shardNum] = shardNumFailures;
		}
	}

	public void start(int port, String configPrefix, int numShards, String keyFile, int numKeys,
			double zipfCoeff) {
		this.numShards = numShards;
		List<String> allKeys = Utils.parseKeys(keyFile, numKeys);
		int numReadKeys = allKeys.size() / 10;
		int numIncrementKeys = 2 * (allKeys.size() / 10);
		this.readKeys = allKeys.subList(0, numReadKeys);
		this.incrementKeys = allKeys.subList(numReadKeys, numReadKeys + numIncrementKeys);
		this.writeKeys = allKeys.subList(numReadKeys + numIncrementKeys, allKeys.size());
		
		readRandom = new RandomIndexGen(readKeys.size(), zipfCoeff);
		writeRandom = new RandomIndexGen(writeKeys.size(), zipfCoeff);
		incrementRandom = new RandomIndexGen(incrementKeys.size(), zipfCoeff);
		
		String[] redisHostnames = new String[numShards];
		int[] redisPorts = new int[numShards];
		numSlaves = new int[numShards];
		numFailures = new int[numShards];
		parseBackendConfigs(configPrefix, numShards, redisHostnames, redisPorts, numSlaves, numFailures);
				
		pools = new JedisPool[numShards];
		for (int i = 0; i < pools.length; i++) {
			pools[i] = new JedisPool(new JedisPoolConfig(), redisHostnames[i], redisPorts[i]);
		}
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
		for (int i = 0; i < pools.length; i++) {
			pools[i].destroy();
		}
	}

	public static void main(String[] args) {
		if (args.length < 6) {
			System.err.println("usage: java RetwisServer port config-prefix num-shards key-file num-keys zipf");
			System.exit(1);
		}
		int port = Integer.parseInt(args[0]);
		String configPrefix = args[1];
		int numShards = Integer.parseInt(args[2]);
		String keyFile = args[3];
		int numKeys = Integer.parseInt(args[4]);
		double zipfCoeff = Double.parseDouble(args[5]);
		
		new RetwisServer().start(port, configPrefix, numShards, keyFile, numKeys, zipfCoeff);
	}
}
