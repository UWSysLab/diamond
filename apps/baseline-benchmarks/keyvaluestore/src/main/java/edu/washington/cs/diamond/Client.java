package edu.washington.cs.diamond;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
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
	private List<String> keys;
	
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
	
	List<String> parseKeys(String keyFile) {
		List<String> keys = new ArrayList<String>();
		try {
			BufferedReader reader = new BufferedReader(new FileReader(keyFile));
			String line = reader.readLine();
			while (line != null) {
				keys.add(line);
				line = reader.readLine();
			}
			reader.close();
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		return keys;
	}
	
	String parseConfigFile(String configFile) {
		String server = null;
		try {
			BufferedReader reader = new BufferedReader(new FileReader(configFile));
			String line = reader.readLine();
			while (line != null) {
				String[] lineSplit = line.split("\\s+");
				if (lineSplit[0].equals("replica")) {
					server = lineSplit[1];
				}
				line = reader.readLine();
			}
			reader.close();
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		return server;
	}
	
	public void start(int seconds, boolean printKeys) {
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
			
			System.out.print(startTime + "\t" + endTime + "\t" + 1);
			if (printKeys) {
			System.out.print("\t" + readKey + "\t" + writeKey);
			}
			System.out.print("\n");
			
			if ((endTime - globalStartTime) / 1000 >= seconds) {
				done = true;
			}
		}
	}
	
	public Client(String configFile, String keyFile) {
		server = parseConfigFile(configFile);
		random = new Random();
		
		keys = parseKeys(keyFile);
	}
	
	public static void main(String[] args) {
		if (args.length < 3) {
			System.err.println("usage: java Client configFile keyFile numSeconds");
			System.exit(1);
		}
		new Client(args[0], args[1]).start(Integer.parseInt(args[2]), false);
	}
}
