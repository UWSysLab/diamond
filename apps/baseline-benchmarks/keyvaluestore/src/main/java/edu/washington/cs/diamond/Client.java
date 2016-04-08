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

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

public class Client {
	
	private String server;
	
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
	
	public void start(String url) {
		server = url;
		
		System.out.println(get("test"));
		put("test", "client time: " + System.currentTimeMillis());
		System.out.println(get("test"));
	}
	
	public static void main(String[] args) {
		if (args.length < 1) {
			System.err.println("usage: java Client hostname:port");
		}
		new Client().start(args[0]);
	}
}
