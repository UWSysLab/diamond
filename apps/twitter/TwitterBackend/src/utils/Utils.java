package utils;

import java.net.URI;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.http.NameValuePair;
import org.apache.http.client.utils.URLEncodedUtils;

public class Utils {
	public static Map<String, String> getQueryParams(URI uri) {
		List<NameValuePair> queryParams = URLEncodedUtils.parse(uri, "UTF-8");
		Map<String, String> result = new HashMap<String, String>();
		for (NameValuePair pair : queryParams) {
			result.put(pair.getName(), pair.getValue());
		}
		return result;
	}
}
