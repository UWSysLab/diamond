package com.example.androidchatbenchmark;

import android.support.v7.app.ActionBarActivity;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

import android.app.AlertDialog;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;
import edu.washington.cs.diamond.Diamond;

public class MainActivity extends ActionBarActivity {
	
	static final int MESSAGE_LIST_SIZE = 100;
	static final int NUM_ACTIONS = 1000;
	static final String MESSAGE = "Help, I'm trapped in a Diamond benchmark";
	
	static final int INITIAL_CAPACITY = NUM_ACTIONS;
	
	static String chatroomName = "androidbenchmark";
	static String userName = "android";
	static String serverName = "moranis.cs.washington.edu";
	
	private Diamond.DStringList messageList;
	
	public double[] writeMessageTransaction(String msg) {
		String fullMsg = userName + ": " + msg;
		int committed = 0;
		int numAborts = 0;
		long writeTimeStart = 0;
		long writeTimeEnd = 1000 * 1000 * 1000;
		while(committed == 0) {
			writeTimeStart = System.nanoTime();
			Diamond.DObject.TransactionBegin();
			messageList.Append(fullMsg);
			if (messageList.Size() > MESSAGE_LIST_SIZE) {
				messageList.Erase(0);
			}
			committed = Diamond.DObject.TransactionCommit();
			if (committed == 0) {
				numAborts++;
			}
		}
		writeTimeEnd = System.nanoTime();
		double time = ((double)(writeTimeEnd - writeTimeStart)) / (1000 * 1000);
		double[] ret = new double[2];
		ret[0] = time;
		ret[1] = numAborts;
		return ret;
	}
	
	public double writeMessageAtomic(String msg) {
		String fullMsg = userName + ": " + msg;
		long writeTimeStart = 0;
		long writeTimeEnd = 1000 * 1000 * 1000;
		writeTimeStart = System.nanoTime();
		messageList.Append(fullMsg);
		if (messageList.Size() > MESSAGE_LIST_SIZE) {
			messageList.Erase(0);
		}
		writeTimeEnd = System.nanoTime();
		double time = ((double)(writeTimeEnd - writeTimeStart)) / (1000 * 1000);
		return (time);
	}
	
	public double[] readMessagesTransaction() {
		List<String> result = null;
		int committed = 0;
		int numAborts = 0;
		long readTimeStart = 0;
		long readTimeEnd = 1000 * 1000 * 1000;
		while (committed == 0) {
			readTimeStart = System.nanoTime();
			Diamond.DObject.TransactionBegin();
			result = messageList.Members();
			committed = Diamond.DObject.TransactionCommit();
			if (committed == 0) {
				numAborts++;
			}
		}
		readTimeEnd = System.nanoTime();
		if (result.get(0).indexOf(MESSAGE) == -1) {
			Log.i("BENCHMARK", "Error: first item of chat log is " + result.get(0));
		}
		double time = ((double)(readTimeEnd - readTimeStart)) / (1000 * 1000);
		double[] ret = new double[2];
		ret[0] = time;
		ret[1] = numAborts;
		return ret;
	}
	
	public double readMessagesAtomic() {
		List<String> result = null;
		long readTimeStart = 0;
		long readTimeEnd = 1000 * 1000 * 1000;
		readTimeStart = System.nanoTime();
		result = messageList.Members();
		readTimeEnd = System.nanoTime();
		if (result.get(0).indexOf(MESSAGE) == -1) {
			Log.i("BENCHMARK", "Error: first item of chat log is " + result.get(0));
		}
		double time = ((double)(readTimeEnd - readTimeStart)) / (1000 * 1000);
		return time;
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		//setContentView(R.layout.activity_main);
		
		TextView textBox = new TextView(this.getBaseContext());
		textBox.setTextColor(Color.BLACK);
		setContentView(textBox);
		
		Diamond.DiamondInit(serverName);
		String chatLogKey = "dimessage:" + chatroomName + ":chatlog";
		messageList = new Diamond.DStringList();
		Diamond.DObject.Map(messageList, chatLogKey);
		
		// Warm up JVM and fill chat log
		Log.i("BENCHMARK", "Progress: warming up JVM");
		for (int i = 0; i < NUM_ACTIONS; i++) {
			writeMessageTransaction(MESSAGE);
		}
		
		// Transactional reads
		Log.i("BENCHMARK", "Progress: starting transactional reads");
		double totalTimeReadTrans = 0;
		double totalNumAbortsReadTrans = 0;
		List<Double> timesReadTrans = new ArrayList<Double>(INITIAL_CAPACITY);
		List<Double> numAbortsReadTrans = new ArrayList<Double>(INITIAL_CAPACITY);
		for (int i = 0; i < NUM_ACTIONS; i++) {
			double[] ret = readMessagesTransaction();
			totalTimeReadTrans += ret[0];
			timesReadTrans.add(ret[0]);
			totalNumAbortsReadTrans += ret[1];
			numAbortsReadTrans.add(ret[1]);
		}
		double averageTimeReadTrans = ((double)totalTimeReadTrans) / NUM_ACTIONS;
		double averageAbortsReadTrans = ((double)totalNumAbortsReadTrans) / NUM_ACTIONS;
		
		// Transactional writes
		Log.i("BENCHMARK", "Progress: starting transactional writes");
		double totalTimeWriteTrans = 0;
		double totalNumAbortsWriteTrans = 0;
		List<Double> timesWriteTrans = new ArrayList<Double>(INITIAL_CAPACITY);
		List<Double> numAbortsWriteTrans = new ArrayList<Double>(INITIAL_CAPACITY);
		
		for (int i = 0; i < NUM_ACTIONS; i++) {
			double[] ret = writeMessageTransaction(MESSAGE);
			totalTimeWriteTrans += ret[0];
			timesWriteTrans.add(ret[0]);
			totalNumAbortsWriteTrans += ret[1];
			numAbortsWriteTrans.add(ret[1]);
		}
		double averageTimeWriteTrans = ((double)totalTimeWriteTrans) / NUM_ACTIONS;
		double averageAbortsWriteTrans = ((double)totalNumAbortsWriteTrans) / NUM_ACTIONS;
		
		Diamond.DObject.SetGlobalStaleness(true);
		Diamond.DObject.SetGlobalMaxStaleness(100);
		
		// Stale reads
		Log.i("BENCHMARK", "Progress: starting stale reads");
		double totalTimeReadStale = 0;
		double totalNumAbortsReadStale = 0;
		List<Double> timesReadStale = new ArrayList<Double>(INITIAL_CAPACITY);
		List<Double> numAbortsReadStale = new ArrayList<Double>(INITIAL_CAPACITY);
		for (int i = 0; i < NUM_ACTIONS; i++) {
			double[] ret = readMessagesTransaction();
			totalTimeReadStale += ret[0];
			timesReadStale.add(ret[0]);
			totalNumAbortsReadStale += ret[1];
			numAbortsReadStale.add(ret[1]);
		}
		double averageTimeReadStale = ((double)totalTimeReadStale) / NUM_ACTIONS;
		double averageAbortsReadStale = ((double)totalNumAbortsReadStale) / NUM_ACTIONS;
		
		// Stale writes
		Log.i("BENCHMARK", "Progress: starting stale writes");
		double totalTimeWriteStale = 0;
		double totalNumAbortsWriteStale = 0;
		List<Double> timesWriteStale = new ArrayList<Double>(INITIAL_CAPACITY);
		List<Double> numAbortsWriteStale = new ArrayList<Double>(INITIAL_CAPACITY);
		
		for (int i = 0; i < NUM_ACTIONS; i++) {
			double[] ret = writeMessageTransaction(MESSAGE);
			totalTimeWriteStale += ret[0];
			timesWriteStale.add(ret[0]);
			totalNumAbortsWriteStale += ret[1];
			numAbortsWriteStale.add(ret[1]);
		}
		double averageTimeWriteStale = ((double)totalTimeWriteStale) / NUM_ACTIONS;
		double averageAbortsWriteStale = ((double)totalNumAbortsWriteStale) / NUM_ACTIONS;
		
		// Atomic reads
		Log.i("BENCHMARK", "Progress: starting atomic reads");
		long totalTimeReadAtomic = 0;
		List<Double> timesReadAtomic = new ArrayList<Double>(INITIAL_CAPACITY);
		
		for (int i = 0; i < NUM_ACTIONS; i++) {
				double time = readMessagesAtomic();
				totalTimeReadAtomic += time;
				timesReadAtomic.add(time);
		}
		double averageTimeReadAtomic = ((double)totalTimeReadAtomic) / NUM_ACTIONS;

		// Atomic writes
		Log.i("BENCHMARK", "Progress: starting atomic writes");
		long totalTimeWriteAtomic = 0;
		List<Double> timesWriteAtomic = new ArrayList<Double>(INITIAL_CAPACITY);
		for (int i = 0; i < NUM_ACTIONS; i++) {
			double time = writeMessageAtomic(MESSAGE);
			totalTimeWriteAtomic += time;
			timesWriteAtomic.add(time);
		}
		double averageTimeWriteAtomic = ((double)totalTimeWriteAtomic) / NUM_ACTIONS;
		
		for (int i = 0; i < timesWriteTrans.size(); i++) {
			Log.i("BENCHMARK", "data:\twrite\ttransaction\t" + timesWriteTrans.get(i));
			try {Thread.sleep(1);} catch (InterruptedException e) {}
		}
		for (int i = 0; i < timesReadTrans.size(); i++) {
			Log.i("BENCHMARK", "data:\tread\ttransaction\t" + timesReadTrans.get(i));
			try {Thread.sleep(1);} catch (InterruptedException e) {}
		}
		for (int i = 0; i < timesWriteStale.size(); i++) {
			Log.i("BENCHMARK", "data:\twrite\tstale\t" + timesWriteStale.get(i));
			try {Thread.sleep(1);} catch (InterruptedException e) {}
		}
		for (int i = 0; i < timesReadStale.size(); i++) {
			Log.i("BENCHMARK", "data:\tread\tstale\t" + timesReadStale.get(i));
			try {Thread.sleep(1);} catch (InterruptedException e) {}
		}
		for (int i = 0; i < timesWriteAtomic.size(); i++) {
			Log.i("BENCHMARK", "data:\twrite\tatomic\t" + timesWriteAtomic.get(i));
			try {Thread.sleep(1);} catch (InterruptedException e) {}
		}
		for (int i = 0; i < timesReadAtomic.size(); i++) {
			Log.i("BENCHMARK", "data:\tread\tatomic\t" + timesReadAtomic.get(i));
			try {Thread.sleep(1);} catch (InterruptedException e) {}
		}
		
		Log.i("BENCHMARK", "Done with Diamond experiment");
		
		textBox.setText("Transaction read: " + averageTimeReadTrans + "\n"
						+ "Transaction write: " + averageTimeWriteTrans + "\n"
						+ "Stale read: " + averageTimeReadStale + "\n"
						+ "Stale write: " + averageTimeWriteStale + "\n"
						+ "Atomic read: " + averageTimeReadAtomic + "\n"
						+ "Atomic write: " + averageTimeWriteAtomic + "\n");
		
		Log.i("BENCHMARK", "Why doesn't this show up");
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
