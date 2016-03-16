package main.java.edu.washington.cs.diamond;

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
	
	Map<Long, TxnFunction> idFuncMap;
	Map<TxnFunction, Long> funcIdMap;
	Map<TxnFunction, Object[]> funcArgMap;

	long nextId;
	
	public ReactiveManager() {
		idFuncMap = new HashMap<Long, TxnFunction>();
		funcIdMap = new HashMap<TxnFunction, Long>();
		funcArgMap = new HashMap<TxnFunction, Object[]>();
		nextId = 0;
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
	
	public void ReactiveTxn(final TxnFunction func, final Object... args) {
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
	
	public void ExecuteTxn(final TxnFunction func, final TxnCallback callback, final Object... args) {
		new Thread(new Runnable() {
			public void run() {
				DObject.TransactionBegin();
				func.func(args);
				int committed = DObject.TransactionCommit();
				callback.callback(committed);
			}
		}).start();
	}
}
