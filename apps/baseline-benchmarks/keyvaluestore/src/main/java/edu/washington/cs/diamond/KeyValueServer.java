package edu.washington.cs.diamond;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import com.sun.net.httpserver.*;
import java.io.*;
import java.net.*;
import java.nio.charset.Charset;
import java.util.*;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.codec.binary.StringUtils;
import org.apache.http.NameValuePair;
import org.apache.http.client.utils.URLEncodedUtils;
import org.eclipse.jetty.server.Handler;
import org.eclipse.jetty.server.Request;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.server.handler.AbstractHandler;
import org.eclipse.jetty.server.handler.ContextHandler;
import org.eclipse.jetty.server.handler.ContextHandlerCollection;
import org.eclipse.jetty.util.MultiMap;

import redis.clients.jedis.*;

class Utils {
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
}

public class KeyValueServer
{
    Jedis jedis;

    class PutGetHandler extends AbstractHandler {

		@Override
		public void handle(String target, Request baseRequest, HttpServletRequest request, HttpServletResponse response)
				throws IOException, ServletException {

			JsonObject responseJson = new JsonObject();
            int responseCode = HttpServletResponse.SC_BAD_REQUEST;

			if (target.equals("/put")) {
				Map<String,String> bodyParams = Utils.getBodyParams(request.getInputStream());
				String key = bodyParams.get("key");
				String value = bodyParams.get("value");

                if (key != null && value != null) {
                    jedis.set(key, value);
                    responseJson.add("key", new JsonPrimitive(key));
                    responseJson.add("value", new JsonPrimitive(value));
                    responseCode = HttpServletResponse.SC_OK;
                }
			}
			else if (target.equals("/get")) {
				String queryString = request.getQueryString();
				if (queryString == null) {
					queryString = "";
				}
				Map<String,String> queryParams = Utils.getQueryParams(queryString);
				String key = queryParams.get("key");
				String value = null;

                if (key != null) {
                    value = jedis.get(key);
                    if (value == null) {
                        value = "";
                    }
                    responseJson.add("key", new JsonPrimitive(key));
                    responseJson.add("value", new JsonPrimitive(value));
                    responseCode = HttpServletResponse.SC_OK;
                }
			}

			response.setStatus(responseCode);
			PrintWriter out = response.getWriter();
			out.print(responseJson.toString());

			baseRequest.setHandled(true);
		}
    }

    public void start(int port, String redisHostname, int redisPort) {
        JedisPool pool = new JedisPool(new JedisPoolConfig(), redisHostname, redisPort);
        jedis = null;
        Server server = null;

        try {
            jedis = pool.getResource();
            server = new Server(port);

            server.setHandler(new PutGetHandler());

            server.start();
            server.join();
        }
        catch(Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
        finally {
            if (jedis != null) {
                jedis.close();
            }
        }
        pool.destroy();
    }

    public static void main( String[] args )
    {
    	if (args.length < 3) {
    		System.err.println("usage: java Server port redis-hostname redis-port");
    		System.exit(1);
    	}
        new KeyValueServer().start(Integer.parseInt(args[0]), args[1], Integer.parseInt(args[2]));
    }
}
