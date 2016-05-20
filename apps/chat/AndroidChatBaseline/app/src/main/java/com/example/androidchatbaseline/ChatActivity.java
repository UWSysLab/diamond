package com.example.androidchatbaseline;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import com.example.androidchatbaseline.R;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.widget.EditText;
import android.widget.TextView;

public class ChatActivity extends ActionBarActivity {
	
	private static final String TAG = "ChatActivity";
	
	static final int PORT = 8000;
	static String serverName = "moranis.cs.washington.edu";
	static String serverURLString = "http://" + serverName + ":" + PORT + "/chat";
	
	final int NUM_LINES = 11;
	final int MESSAGE_LIST_SIZE = 100;
	
	private TextView chatBox;
	private String userName;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_chat);
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this.getBaseContext());
		
		chatBox = (TextView)findViewById(R.id.chatTextBox);
		userName = prefs.getString(LoginActivity.PREFS_SCREENNAME, "AnonymousUser");
		
		EditText entryTextBox = (EditText)findViewById(R.id.entryTextBox);
		entryTextBox.setOnEditorActionListener(new EditText.OnEditorActionListener() {
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
				EditText ev = (EditText)v;
				new Thread(new MessageSender(ev.getText().toString())).start();
				ev.setText("");
				return true;
			}
		});
		
		new Thread(new MessageReader()).start();
	}
	
	private class MessageSender implements Runnable {
		String message;
		public MessageSender(String msg) {
			message = msg;
		}
		public void run() {
			String fullMsg = userName + ": " + message;
			writeMessage(fullMsg);
		}
		
		public void writeMessage(String msg) {
			int responseCode = -1;
			try {
				URL serverURL = new URL(serverURLString);
				HttpURLConnection connection = (HttpURLConnection)serverURL.openConnection();
				connection.setRequestMethod("POST");
				connection.setDoOutput(true);
				OutputStreamWriter out = new OutputStreamWriter(connection.getOutputStream());
				out.write(msg);
				out.close();
				responseCode = connection.getResponseCode();
			} catch (MalformedURLException e) {
				Log.e(TAG, "Error: malformed URL");
			} catch (IOException e) {
				Log.e(TAG, "Error: could not connect to server: " + e);
			}
			if (responseCode != 200) {
				Log.e(TAG, "Error: response code " + responseCode);
			}
		}
	}
	
	private class MessageReader implements Runnable {
		public void run() {
			while(true) {
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					Log.e(TAG, "Error: MessageReader thread interrupted");
				}
				
				List<String> messageList = readMessages();
				StringBuilder sb = new StringBuilder();
				int minLine = messageList.size() - NUM_LINES;
				if (minLine < 0) {
					minLine = 0;
				}
				for (int i = minLine; i < messageList.size(); i++) {
					sb.append(messageList.get(i) + "\n");
				}
				final String text = sb.toString();
				
				//Update UI
				runOnUiThread(new Runnable() {
					public void run() {
						chatBox.setText(text);
					}
				});
			}
		}
		
		public List<String> readMessages() {
			int responseCode = -1;
			List<String> result = new ArrayList<String>();
			try {
				URL serverURL = new URL(serverURLString);			
				HttpURLConnection connection = (HttpURLConnection)serverURL.openConnection();
				BufferedReader in = new BufferedReader(new InputStreamReader(connection.getInputStream()));
				String jsonStr = in.readLine();
				JsonParser parser = new JsonParser();
				JsonArray jsonArray = parser.parse(jsonStr).getAsJsonArray();
				for (int i = 0; i < jsonArray.size(); i++) {
					JsonElement item = jsonArray.get(i);
					result.add(item.getAsString());
				}
				in.close();
				responseCode = connection.getResponseCode();
			} catch (MalformedURLException e) {
				Log.e(TAG, "Error: malformed URL");
			} catch (IOException e) {
				Log.e(TAG, "Error: could not connect to server: " + e);
			}
			if (responseCode != 200) {
				Log.e(TAG, "Error: response code " + responseCode);
			}
			return result;
		}
	}
}
