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

public class KeyValueServer
{
    Jedis jedis;

    class PutHandler implements HttpHandler {

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            Map<String, String> bodyParams = Utils.getBodyParams(exchange.getRequestBody());
            String key = bodyParams.get("key");
            String value = bodyParams.get("key");
            jedis.set(key, value);

            JsonObject responseJson = new JsonObject();
            responseJson.add("key", new JsonPrimitive(key));
            responseJson.add("value", new JsonPrimitive(value));

            exchange.getResponseHeaders().add("Connection", "close");
            exchange.sendResponseHeaders(200, 0);
            OutputStream os = exchange.getResponseBody();
            os.write(responseJson.toString().getBytes());
            os.close();
        }
    }

    class GetHandler implements HttpHandler {

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            Map<String, String> queryParams = Utils.getQueryParams(exchange.getRequestURI());
            String key = queryParams.get("key");
            String value = jedis.get(key);

            JsonObject responseJson = new JsonObject();
            responseJson.add("key", new JsonPrimitive(key));
            responseJson.add("value", new JsonPrimitive(value));

            exchange.getResponseHeaders().add("Connection", "close");
            exchange.sendResponseHeaders(200, 0);
            OutputStream os = exchange.getResponseBody();
            os.write(responseJson.toString().getBytes());
            os.close();
        }
    }

    public void start() {
        JedisPool pool = new JedisPool(new JedisPoolConfig(), "localhost", 6380);
        jedis = null;
        HttpServer server = null;

        try {
            jedis = pool.getResource();
            jedis.flushDB();
            server = HttpServer.create(new InetSocketAddress(8000), 0);
            server.createContext("/put", new PutHandler());
            server.createContext("/get", new GetHandler());
            server.setExecutor(null);
            server.start();
        }
        catch(IOException e) {
            System.out.println(e);
        }
        finally {
            jedis.close();
        }
        pool.destroy();
    }

    public static void main( String[] args )
    {
        new KeyValueServer().start();
    }
}
