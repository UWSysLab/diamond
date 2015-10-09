package com.example.androidchatbenchmark;

import android.support.v7.app.ActionBarActivity;
import android.util.Log;

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
	
	static final int ACTION_READ = 0;
	static final int ACTION_WRITE = 1;
	
	static String chatroomName = "androidbenchmark";
	static String userName = "android";
	static String serverName = "coldwater.cs.washington.edu";
	
	private Diamond.DStringList messageList;
	
	public long[] writeMessageTransaction(int roundNum, String msg) {
		String fullMsg = userName + ": " + msg;
		int committed = 0;
		int numAborts = 0;
		long writeTimeStart = 0;
		long writeTimeEnd = 1000 * 1000 * 1000;
		while(committed == 0) {
			writeTimeStart = System.currentTimeMillis();
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
		writeTimeEnd = System.currentTimeMillis();
		long[] ret = new long[2];
		ret[0] = writeTimeEnd - writeTimeStart;
		ret[1] = numAborts;
		return ret;
	}
	
	public long writeMessageAtomic(int roundNum, String msg) {
		String fullMsg = userName + ": " + msg;
		long writeTimeStart = 0;
		long writeTimeEnd = 1000 * 1000 * 1000;
		writeTimeStart = System.currentTimeMillis();
		messageList.Append(fullMsg);
		if (messageList.Size() > MESSAGE_LIST_SIZE) {
			messageList.Erase(0);
		}
		writeTimeEnd = System.currentTimeMillis();
		return writeTimeEnd - writeTimeStart;
	}
	
	public long[] readMessagesTransaction(int roundNum) {
		List<String> result = null;
		int committed = 0;
		int numAborts = 0;
		long readTimeStart = 0;
		long readTimeEnd = 1000 * 1000 * 1000;
		while (committed == 0) {
			readTimeStart = System.currentTimeMillis();
			Diamond.DObject.TransactionBegin();
			result = messageList.Members();
			committed = Diamond.DObject.TransactionCommit();
			if (committed == 0) {
				numAborts++;
			}
		}
		readTimeEnd = System.currentTimeMillis();
		long[] ret = new long[2];
		ret[0] = readTimeEnd - readTimeStart;
		ret[1] = numAborts;
		return ret;
	}
	
	public long readMessagesAtomic(int roundNum) {
		List<String> result = null;
		long readTimeStart = 0;
		long readTimeEnd = 1000 * 1000 * 1000;
		readTimeStart = System.currentTimeMillis();
		result = messageList.Members();
		readTimeEnd = System.currentTimeMillis();
		return readTimeEnd - readTimeStart;
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		//setContentView(R.layout.activity_main);
		
		TextView textBox = new TextView(this.getBaseContext());
		textBox.setTextColor(Color.BLACK);
		setContentView(textBox);
		
		Diamond.DiamondInit("coldwater.cs.washington.edu");
		
		String chatLogKey = "dimessage:" + chatroomName + ":chatlog";
		String updateTimeKey = "dimessage:" + chatroomName + ":updatetime";
		
		messageList = new Diamond.DStringList(chatLogKey);
		//Diamond.DObject.Map(messageList, chatLogKey);
		
		// Transactional reads
		long totalTimeReadTrans = 0;
		int numRepsReadTrans = 0;
		long numAbortsReadTrans = 0;
		
		for (int i = 0; i < NUM_ACTIONS; i++) {
			if (i >= 200 && i <= 800) {
				long[] ret = readMessagesTransaction(i);
				totalTimeReadTrans += ret[0];
				numAbortsReadTrans += ret[1];
				numRepsReadTrans++;
			}
		}
		double averageTimeRead = ((double)totalTimeReadTrans) / numRepsReadTrans;
		double averageAbortsRead = ((double)numAbortsReadTrans) / numRepsReadTrans;
		String readTransResultString = "Action: READ transaction\tNum reps: " + numRepsReadTrans
				+ "\tAverage latency: " + averageTimeRead
				+ "\tAverage num aborts: " + averageAbortsRead;
		
		// Transactional writes
		long totalTimeWriteTrans = 0;
		int numRepsWriteTrans = 0;
		long numAbortsWriteTrans = 0;
		
		for (int i = 0; i < NUM_ACTIONS; i++) {
			if (i >= 200 && i <= 800) {
				long[] ret = writeMessageTransaction(i, MESSAGE);
				totalTimeWriteTrans += ret[0];
				numAbortsWriteTrans += ret[1];
				numRepsWriteTrans++;
			}
		}
		double averageTimeWrite = ((double)totalTimeWriteTrans) / numRepsWriteTrans;
		double averageAbortsWrite = ((double)numAbortsWriteTrans) / numRepsWriteTrans;
		String writeTransResultString = "Action: WRITE transaction\tNum reps: " + numRepsWriteTrans
				+ "\tAverage latency: " + averageTimeWrite
				+ "\tAverage num aborts: " + averageAbortsWrite;
		
		// Atomic reads
		long totalTimeReadAtomic = 0;
		int numRepsReadAtomic = 0;
		
		for (int i = 0; i < NUM_ACTIONS; i++) {
			if (i >= 200 && i <= 800) {
				totalTimeReadAtomic += readMessagesAtomic(i);
				numRepsReadAtomic++;
			}
		}
		double averageTimeReadAtomic = ((double)totalTimeReadAtomic) / numRepsReadAtomic;
		String readAtomicResultString = "Action: READ atomic\tNum reps: " + numRepsReadAtomic + "\tAverage latency: " + averageTimeReadAtomic;		

		// Atomic writes
		long totalTimeWriteAtomic = 0;
		int numRepsWriteAtomic = 0;
		
		for (int i = 0; i < NUM_ACTIONS; i++) {
			if (i >= 200 && i <= 800) {
				totalTimeWriteAtomic += writeMessageAtomic(i, MESSAGE);
				numRepsWriteAtomic++;
			}
		}
		double averageTimeWriteAtomic = ((double)totalTimeWriteAtomic) / numRepsWriteAtomic;
		String writeAtomicResultString = "Action: WRITE atomic\tNum reps: " + numRepsWriteAtomic 
				+ "\tAverage latency: " + averageTimeWriteAtomic;
		
		textBox.setText(readTransResultString + "\n"
				+ writeTransResultString + "\n"
				+ readAtomicResultString + "\n"
				+ writeAtomicResultString + "\n");
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
