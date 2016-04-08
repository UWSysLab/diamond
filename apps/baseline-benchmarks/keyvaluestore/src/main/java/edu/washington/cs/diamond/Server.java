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
import org.apache.commons.codec.binary.Base64;
import org.apache.commons.codec.binary.StringUtils;
import org.apache.http.NameValuePair;
import org.apache.http.client.utils.URLEncodedUtils;
import redis.clients.jedis.*;

class Utils {
    public static Map<String, String> getQueryParams(URI uri) {
        List<NameValuePair> queryParams = URLEncodedUtils.parse(uri, "UTF-8");
        Map<String, String> result = new HashMap<String, String>();
        for (NameValuePair pair : queryParams) {
            result.put(pair.getName(), pair.getValue());
        }
        return result;
    }

    public static Map<String, String> getBodyParams(InputStream requestBody) {
        try {
            byte[] bodyArray = new byte[requestBody.available()];
            requestBody.read(bodyArray);
            String bodyString = new String(bodyArray, "UTF-8");
            List<NameValuePair> queryParams = URLEncodedUtils.parse(bodyString, Charset.forName("UTF-8"));
            Map<String, String> result = new HashMap<String, String>();
            for (NameValuePair pair : queryParams) {
                result.put(pair.getName(), pair.getValue());
            }
            return result;
        }
        catch (IOException e) {
            return null;
        }
    }
}

public class Server
{
    Jedis jedis;

    class PutHandler implements HttpHandler {

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                Map<String, String> bodyParams = Utils.getBodyParams(exchange.getRequestBody());
                String key = bodyParams.get("key");
                String value = bodyParams.get("value");

                JsonObject responseJson = new JsonObject();
                int responseCode = -1;
                if (key == null || value == null) {
                    responseJson.add("key", new JsonPrimitive("(nil)"));
                    responseJson.add("value", new JsonPrimitive("(nil)"));
                    responseCode = 400;
                }
                else {
                    jedis.set(key, value);
                    responseJson.add("key", new JsonPrimitive(key));
                    responseJson.add("value", new JsonPrimitive(value));
                    responseCode = 200;
                }

                exchange.getResponseHeaders().add("Connection", "close");
                exchange.sendResponseHeaders(responseCode, 0);
                OutputStream os = exchange.getResponseBody();
                os.write(responseJson.toString().getBytes());
                os.close();
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    class GetHandler implements HttpHandler {

        @Override
        public void handle(HttpExchange exchange) {
            try {
                Map<String, String> queryParams = Utils.getQueryParams(exchange.getRequestURI());
                String key = queryParams.get("key");
                String value = null;

                JsonObject responseJson = new JsonObject();
                int responseCode = -1;
                if (key == null) {
                    responseJson.add("key", new JsonPrimitive("(nil)"));
                    responseJson.add("value", new JsonPrimitive("(nil)"));
                    responseCode = 400;
                }
                else {
                    value = jedis.get(key);
                    if (value == null) {
                        value = "(nil)";
                    }
                    responseJson.add("key", new JsonPrimitive(key));
                    responseJson.add("value", new JsonPrimitive(value));
                    responseCode = 200;
                }

                exchange.getResponseHeaders().add("Connection", "close");
                exchange.sendResponseHeaders(responseCode, 0);
                OutputStream os = exchange.getResponseBody();
                os.write(responseJson.toString().getBytes());
                os.close();
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void start(int port, String redisHostname, int redisPort) {
        JedisPool pool = new JedisPool(new JedisPoolConfig(), redisHostname, redisPort);
        jedis = null;
        HttpServer server = null;

        try {
            jedis = pool.getResource();
            server = HttpServer.create(new InetSocketAddress(port), 0);
            server.createContext("/put", new PutHandler());
            server.createContext("/get", new GetHandler());
            server.setExecutor(null);
            server.start();
        }
        catch(IOException e) {
            System.out.println(e);
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
        new Server().start(Integer.parseInt(args[0]), args[1], Integer.parseInt(args[2]));
    }
}
