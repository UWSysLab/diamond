package edu.washington.cs.diamond;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.http.NameValuePair;
import org.apache.http.client.utils.URLEncodedUtils;

public class Utils {
	public static Map<String, String> getQueryParams(String queryString) {
		Map<String, String> result = new HashMap<String, String>();
		String[] paramSplit = queryString.split("&");
		for (String param : paramSplit) {
			String[] keyValueSplit = param.split("=");
			if (keyValueSplit.length == 2) {
				String key = keyValueSplit[0];
				String value = keyValueSplit[1];
				result.put(key, value);
			}
		}
		return result;
	}

	public static Map<String, String> getBodyParams(InputStream requestBody) {
		try {
			byte[] bodyArray = new byte[requestBody.available()];
			requestBody.read(bodyArray);
			String bodyString = new String(bodyArray, "UTF-8");
			List<NameValuePair> bodyParams = URLEncodedUtils.parse(bodyString, Charset.forName("UTF-8"));
			Map<String, String> result = new HashMap<String, String>();
			for (NameValuePair pair : bodyParams) {
				result.put(pair.getName(), pair.getValue());
			}
			return result;
		}
		catch (IOException e) {
			return null;
		}
	}
	
	public static List<String> parseKeys(String keyFile, int numKeys) {
		List<String> keys = new ArrayList<String>();
		try {
			BufferedReader reader = new BufferedReader(new FileReader(keyFile));
			String line = reader.readLine();
			int keyNum = 0;
			while (line != null && keyNum < numKeys) {
				keys.add(line);
				keyNum++;
				line = reader.readLine();
			}
			reader.close();
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		return keys;
	}
}
