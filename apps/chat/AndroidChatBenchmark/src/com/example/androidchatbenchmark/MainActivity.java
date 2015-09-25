package com.example.androidchatbenchmark;

import android.support.v7.app.ActionBarActivity;
import android.util.Log;

import java.util.List;

import android.app.AlertDialog;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
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
	private Diamond.DLong updateTime;
	private long lastReadUpdateTime;
	
	public long writeMessage(int roundNum, String msg) {
		String fullMsg = userName + ": " + msg;
		int committed = 0;
		long writeTimeStart = 0;
		long writeTimeEnd = 1000 * 1000 * 1000;
		while(committed == 0) {
			writeTimeStart = System.currentTimeMillis();
			Diamond.DObject.TransactionBegin();
			messageList.Append(fullMsg);
			if (messageList.Size() > MESSAGE_LIST_SIZE) {
				messageList.Erase(0);
			}
			updateTime.Set(System.currentTimeMillis());
			committed = Diamond.DObject.TransactionCommit();
		}
		writeTimeEnd = System.currentTimeMillis();
		return writeTimeEnd - writeTimeStart;
	}
	
	public long readMessages(int roundNum) {
		List<String> result = null;
		int committed = 0;
		long readTimeStart = 0;
		long readTimeEnd = 1000 * 1000 * 1000;
		while (committed == 0) {
			readTimeStart = System.currentTimeMillis();
			Diamond.DObject.TransactionBegin();
			if (updateTime.Value() == lastReadUpdateTime) {
				Diamond.DObject.TransactionRetry();
				continue;
			}
			result = messageList.Members();
			lastReadUpdateTime = updateTime.Value();
			committed = Diamond.DObject.TransactionCommit();
		}
		readTimeEnd = System.currentTimeMillis();
		
		return readTimeEnd - readTimeStart;
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		Diamond.DiamondInit("coldwater.cs.washington.edu");
		
		String chatLogKey = "dimessage:" + chatroomName + ":chatlog";
		String updateTimeKey = "dimessage:" + chatroomName + ":updatetime";
		//String chatLogKey = "dimessage:androidbenchmark:chatlog";
		//String updateTimeKey = "dimessage:androidbenchmark:updatetime";
		
		messageList = new Diamond.DStringList(chatLogKey);
		updateTime = new Diamond.DLong(0, updateTimeKey);
		//Diamond.DObject.Map(messageList, chatLogKey);
		//Diamond.DObject.Map(updateTime, updateTimeKey);
		
		int action = ACTION_READ;
		
		long totalTime = 0;
		int numReps = NUM_ACTIONS;
		
		for (int i = 0; i < NUM_ACTIONS; i++) {
			if (action == ACTION_READ) {
				totalTime += readMessages(i);
			}
			else {
				totalTime += writeMessage(i, MESSAGE);
			}
		}
		
		double averageTime = ((double)totalTime) / numReps;
		String actionStr = (action == ACTION_READ ? "READ" : "WRITE");
		
		AlertDialog.Builder builder = new AlertDialog.Builder(getBaseContext());
		AlertDialog dialog = builder.setMessage("Action: " + actionStr + " Average latency: " + averageTime).create();
		dialog.show();
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
