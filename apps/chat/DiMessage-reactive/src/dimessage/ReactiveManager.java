package dimessage;

import java.util.HashSet;
import java.util.Set;

public class ReactiveManager {
	private static ReactiveManager singleton = null;
	
	public static ReactiveManager getReactiveManager() {
		if (singleton == null) {
			singleton = new ReactiveManager();
			singleton.start();
		}
		return singleton;
	}
	public static void addTransaction(ReactiveTransaction tx) {
		getReactiveManager().add(tx);
	}
	
	private Set<ReactiveTransaction> txSet;
	public ReactiveManager() {
		txSet = new HashSet<ReactiveTransaction>();
	}
	public void add(ReactiveTransaction tx) {
		txSet.add(tx);
	}
	public void start() {
		new Thread(new Runnable() {
			public void run() {
				while (true) {
					try {
						Thread.sleep(1000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					for (ReactiveTransaction tx : txSet) {
						tx.react();
					}
				}
			}
		}).start();
	}

}
