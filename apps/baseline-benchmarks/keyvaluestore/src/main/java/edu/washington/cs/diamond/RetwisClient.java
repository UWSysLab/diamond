package edu.washington.cs.diamond;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

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
				keyIdx.add(rand_key());
				keyIdx.add(rand_key());
				keyIdx.add(rand_key());
				Collections.sort(keyIdx);
				
				String value = get(keys.get(keyIdx.get(0)));
				
				for (int i = 0; i < 3; i++) {
					put(keys.get(keyIdx.get(i)), keys.get(keyIdx.get(i)));
				}
				ttype = 1;
			}
			else if (ttype < 20) {
				// 30% - Post tweet transaction. 3,5
				keyIdx.add(rand_key());
				keyIdx.add(rand_key());
				Collections.sort(keyIdx);
				
				for (int i = 0; i < 2; i++) {
					String value = get(keys.get(keyIdx.get(i)));
					put(keys.get(keyIdx.get(i)), keys.get(keyIdx.get(i)));
				}
				ttype = 2;
			}
			else if (ttype < 50) {
				// 30% - Post tweet transaction. 3,5
				keyIdx.add(rand_key());
				keyIdx.add(rand_key());
				keyIdx.add(rand_key());
				keyIdx.add(rand_key());
				keyIdx.add(rand_key());
				Collections.sort(keyIdx);
				
				for (int i = 0; i < 3; i++) {
					String value = get(keys.get(keyIdx.get(i)));
					put(keys.get(keyIdx.get(i)), keys.get(keyIdx.get(i)));
				}
				for (int i = 0; i < 2; i++) {
					put(keys.get(keyIdx.get(i+3)), keys.get(keyIdx.get(i+3)));
				}
				ttype = 3;
			}
			else {
				// 50% - Get followers/timeline transaction. rand(1,10),0
				int nGets = 1 + random.nextInt(10);
				for (int i = 0; i < nGets; i++) {
					keyIdx.add(rand_key());
				}
				
				Collections.sort(keyIdx);
				for (int i = 0; i < nGets; i++) {
					String value = get(keys.get(keyIdx.get(i)));
				}
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
