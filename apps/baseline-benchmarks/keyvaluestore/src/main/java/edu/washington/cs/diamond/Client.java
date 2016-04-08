package edu.washington.cs.diamond;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

public class Client {
	
	private String server;
	private Random random;
	
	public void put(String key, String value) {
		int responseCode = -1;
		try {
			URL serverURL = new URL("http://" + server + "/put");
			HttpURLConnection connection = (HttpURLConnection)serverURL.openConnection();
			connection.setRequestMethod("POST");
			connection.setDoOutput(true);
			OutputStreamWriter out = new OutputStreamWriter(connection.getOutputStream());
			out.write("key=" + key + "&value=" + value);
			out.close();
			responseCode = connection.getResponseCode();
		} catch (MalformedURLException e) {
			System.out.println("Error: malformed URL");
			System.exit(1);
		} catch (IOException e) {
			System.out.println("Error connecting to server: " + e);
			System.exit(1);
		}
		if (responseCode != 200) {
			System.out.println("HTTP error: " + responseCode);
			System.exit(1);
		}
	}
	
	public String get(String key) {
		int responseCode = -1;
		String value = null;
		try {
			URL serverURL = new URL("http://" + server + "/get?key=" + key);
			HttpURLConnection connection = (HttpURLConnection)serverURL.openConnection();
			BufferedReader in = new BufferedReader(new InputStreamReader(connection.getInputStream()));
			String jsonStr = in.readLine();
			JsonParser parser = new JsonParser();
			JsonObject jsonObject = parser.parse(jsonStr).getAsJsonObject();
			value = jsonObject.get("value").getAsString();
			in.close();
			responseCode = connection.getResponseCode();
		} catch (MalformedURLException e) {
			System.out.println("Error: malformed URL");
			System.exit(1);
		} catch (IOException e) {
			System.out.println("Error connecting to server: " + e);
			System.exit(1);
		}
		if (responseCode != 200) {
			System.out.println("HTTP error: " + responseCode);
			System.exit(1);
		}
		return value;
	}
	
	public String getRandomKey() {
		int range = 10 + 26 + 26;
		char[] keyChars = new char[65];
		for (int i = 0; i < 64; i++) {
			int index = random.nextInt(range - 1);
			int finalVal = -1;
			if (index < 10) {
				finalVal = index + 48;
			}
			else if (index >= 10 && index < 36) {
				finalVal = index - 10 + 65;
			}
			else { // index >= 36
				finalVal = index - 36 + 97;
			}
			keyChars[i] = (char)finalVal;
		}
		keyChars[64] = '\0';
		return new String(keyChars);
	}
	
	public void start(String url, int seconds) {
		server = url;
		random = new Random();
		
		List<String> keys = new ArrayList<String>();
		keys.add("test");
		
		long globalStartTime = System.currentTimeMillis();
		boolean done = false;

		String writeKey = getRandomKey();
		
		while (!done) {
			long startTime = System.currentTimeMillis();
			String readKey = keys.get(random.nextInt(keys.size()));
			get(readKey);
			String val = Long.toString(random.nextLong());
			put(writeKey, val);
			long endTime = System.currentTimeMillis();
			
			System.out.println(startTime + "\t" + endTime + "\t" + 1 + "\t" + readKey + "\t" + writeKey);
			try { Thread.sleep(10); } catch(Exception e) {}
			
			if ((endTime - globalStartTime) / 1000 >= seconds) {
				done = true;
			}
		}
	}
	
	public static void main(String[] args) {
		if (args.length < 2) {
			System.err.println("usage: java Client hostname:port numSeconds");
		}
		new Client().start(args[0], Integer.parseInt(args[1]));
	}
}
