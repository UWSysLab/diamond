package com.example.androidchatbaseline;

import android.support.v7.app.ActionBarActivity;
import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;

import android.os.Bundle;
import android.os.StrictMode;
import android.view.Menu;
import android.view.MenuItem;

public class MainActivity extends ActionBarActivity {

	static final String MESSAGE = "Help, I'm trapped in a Diamond benchmark";
	static final int PORT = 9000;
	
	static final int NUM_ACTIONS = 1000;
	static final int INITIAL_CAPACITY = NUM_ACTIONS;
	
	static String serverName = "coldwater.cs.washington.edu";
	static String userName = "android";
	static String serverURLString = "http://" + serverName + ":" + PORT + "/chat";
	
	public static double writeMessage(String msg) {
		String fullMsg = userName + ": " + msg;
		int responseCode = -1;
		long startTime = System.nanoTime();
		try {
			URL serverURL = new URL(serverURLString);
			HttpURLConnection connection = (HttpURLConnection)serverURL.openConnection();
			connection.setRequestMethod("POST");
			connection.setDoOutput(true);
			OutputStreamWriter out = new OutputStreamWriter(connection.getOutputStream());
			out.write(fullMsg);
			out.close();
			responseCode = connection.getResponseCode();
		} catch (MalformedURLException e) {
			Log.e("BENCHMARK", "Error: malformed URL");
		} catch (IOException e) {
			Log.e("BENCHMARK", "Error: could not connect to server: " + e);
		}
		if (responseCode != 200) {
			Log.e("BENCHMARK", "Error: response code not 200: " + responseCode);
		}
		long endTime = System.nanoTime();
		double time = ((double)(endTime - startTime)) / (1000 * 1000);
		return time;
	}
	
	public static double readMessages() {
		int responseCode = -1;
		long startTime = System.nanoTime();
		try {
			URL serverURL = new URL(serverURLString);			
			HttpURLConnection connection = (HttpURLConnection)serverURL.openConnection();
			BufferedReader in = new BufferedReader(new InputStreamReader(connection.getInputStream()));
			String jsonStr = in.readLine();
			JsonParser parser = new JsonParser();
			JsonArray jsonArray = parser.parse(jsonStr).getAsJsonArray();
			List<String> result = new ArrayList<String>();
			for (int i = 0; i < jsonArray.size(); i++) {
				JsonElement item = jsonArray.get(i);
				result.add(item.getAsString());
			}
			in.close();
			responseCode = connection.getResponseCode();
		} catch (MalformedURLException e) {
			Log.e("BENCHMARK", "Error: malformed URL");
		} catch (IOException e) {
			Log.e("BENCHMARK", "Error: could not connect to server: " + e);
		}
		if (responseCode != 200) {
			Log.e("BENCHMARK", "Error: response code not 200: " + responseCode);
		}
		long endTime = System.nanoTime();
		double time = ((double)(endTime - startTime)) / (1000 * 1000);
		return time;
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
		StrictMode.setThreadPolicy(policy);
		
		//Warm up JVM and fill chat log
		Log.i("BENCHMARK", "Progress: warming up JVM");
		for (int i = 0; i < NUM_ACTIONS; i++) {
			writeMessage(MESSAGE);
			readMessages();
		}
		
		//Reads
		Log.i("BENCHMARK", "Progress: starting reads");
		List<Double> timesRead = new ArrayList<Double>(INITIAL_CAPACITY);
		for (int i = 0; i < NUM_ACTIONS; i++) {
			double time = readMessages();
			timesRead.add(time);
		}
		
		//Writes
		Log.i("BENCHMARK", "Progress: starting writes");
		List<Double> timesWrite = new ArrayList<Double>(INITIAL_CAPACITY);
		for (int i = 0; i < NUM_ACTIONS; i++) {
			double time = writeMessage(MESSAGE);
			timesWrite.add(time);
		}
		
		//Output
		for (int i = 0; i < timesRead.size(); i++) {
			Log.i("BENCHMARK", "data:\tread\t" + timesRead.get(i));
			try {Thread.sleep(1);} catch (InterruptedException e) {}
		}
		for (int i = 0; i < timesWrite.size(); i++) {
			Log.i("BENCHMARK", "data:\twrite\t" + timesWrite.get(i));
			try {Thread.sleep(1);} catch (InterruptedException e) {}
		}
		
		Log.i("BENCHMARK", "Done with Diamond experiment");
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
