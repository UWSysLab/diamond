package edu.washington.cs.diamond;

import java.util.HashMap;
import java.util.Map;

import edu.washington.cs.diamond.Diamond.DObject;

public class ReactiveManager {
	
	public interface TxnFunction {
		public void func(Object... args);
	}
	
	public interface TxnCallback {
		public void callback(int committed);
	}
	
	public interface Logger {
		public void onLog(String message);
	}
	
	public static ReactiveManager singleton = null;
	
	Map<Long, TxnFunction> idFuncMap;
	Map<TxnFunction, Long> funcIdMap;
	Map<TxnFunction, Object[]> funcArgMap;

	long nextId;
	
	private Logger logger = null;
	
	private static ReactiveManager getManager() {
		if (singleton == null) {
			singleton = new ReactiveManager();
		}
		return singleton;
	}
	
	public static void StartManager() {
		getManager().Start();
	}
	
	public static void RegisterLogger(Logger l) {
		getManager().SetLogger(l);
	}
	
	public static long reactive_txn(TxnFunction func, Object...args) {
		return getManager().ReactiveTxn(func, args);
	}
	
	public static void reactive_stop(long reactiveId) {
		getManager().ReactiveStop(reactiveId);
	}

	public static void execute_txn(TxnFunction func, Object...args) {
		getManager().ExecuteTxn(func, args);
	}
	
	public static void execute_txn(TxnFunction func, TxnCallback callback, Object...args) {
		getManager().ExecuteTxn(func, callback, args);
	}
	
	public ReactiveManager() {
		idFuncMap = new HashMap<Long, TxnFunction>();
		funcIdMap = new HashMap<TxnFunction, Long>();
		funcArgMap = new HashMap<TxnFunction, Object[]>();
		nextId = 0;
	}
	
	public void SetLogger(Logger l) {
		logger = l;
	}
	
	public void Log(String message) {
		if (logger != null) {
			logger.onLog(message);
		}
	}
	
	public long generateId() {
		long id = nextId;
		nextId++;
		return id;
	}

	public void Start() {
		new Thread(new Runnable() {
			public void run() {
				ReactiveLoop();
			}
		}).start();
	}
	
	public long ReactiveTxn(final TxnFunction func, final Object... args) {
		final long reactiveId = generateId();
		idFuncMap.put(reactiveId, func);
		funcIdMap.put(func, reactiveId);
		funcArgMap.put(func, args);
		new Thread(new Runnable() {
			public void run() {
				DObject.BeginReactive(reactiveId);
				func.func(args);
				DObject.TransactionCommit();
			}
		}).start();
		return reactiveId;
	}
	
	public void ReactiveLoop() {
		while(true) {
			long reactiveId = DObject.GetNextNotification();
			TxnFunction func = idFuncMap.get(reactiveId);
			DObject.BeginReactive(reactiveId);
			func.func(funcArgMap.get(func));
			DObject.TransactionCommit();
		}
	}
	
	public void ReactiveStop(long reactiveId) {
		TxnFunction func = idFuncMap.get(reactiveId);
		idFuncMap.remove(reactiveId);
		funcIdMap.remove(func);
		funcArgMap.remove(func);
	}
	
	public void ExecuteTxn(final TxnFunction func, final Object...args) {
		ExecuteTxn(func, null, args);
	}
	
	public void ExecuteTxn(final TxnFunction func, final TxnCallback callback, final Object... args) {
		new Thread(new Runnable() {
			public void run() {
				DObject.TransactionBegin();
				func.func(args);
				int committed = DObject.TransactionCommit();
				if (callback != null) {
					callback.callback(committed);
				}
			}
		}).start();
	}
}
