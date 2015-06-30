package main;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import redis.clients.jedis.Jedis;
import redis.clients.jedis.JedisPool;
import redis.clients.jedis.JedisPoolConfig;

abstract class BaseJsonHandler implements HttpHandler {
	protected Jedis jedis;
	
	public BaseJsonHandler(Jedis j) {
		jedis = j;
	}
	
	@Override
	public void handle(HttpExchange exchange) throws IOException {
		String requestMethod = exchange.getRequestMethod();
		URI requestURI = exchange.getRequestURI();
		JsonElement responseJson = getResponseJson(requestMethod, requestURI.getQuery());
		exchange.sendResponseHeaders(200, 0);
		OutputStream os = exchange.getResponseBody();
		os.write(responseJson.toString().getBytes());
		os.close();
	}
	
	abstract JsonElement getResponseJson(String requestMethod, String requestQuery);
	
}

class TestHomeTimelineHandler extends BaseJsonHandler {
	public TestHomeTimelineHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, String requestQuery) {
		JsonArray result = new JsonArray();
		JsonObject testUser = new JsonObject();
		testUser.add("id", new JsonPrimitive(1));
		testUser.add("id_str", new JsonPrimitive("1"));
		testUser.add("screen_name", new JsonPrimitive("testuser"));
		testUser.add("name", new JsonPrimitive("Test Name"));
		JsonObject testTweet = new JsonObject();
		testTweet.add("text", new JsonPrimitive("Test tweet"));
		testTweet.add("id", new JsonPrimitive(1));
		testTweet.add("id_str", new JsonPrimitive("1"));
		testTweet.add("created_at", new JsonPrimitive("Wed Aug 27 13:08:45 +0000 2008"));
		testTweet.add("user", testUser);
		result.add(testTweet);
		return result;
	}
}

class TestJedisHandler extends BaseJsonHandler {
	public TestJedisHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, String requestQuery) {
		JsonObject result = new JsonObject();
		result.add("global:uid", new JsonPrimitive(jedis.get("global:uid")));
		return result;
	}
}

class TestJsonHandler extends BaseJsonHandler {
	public TestJsonHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, String requestQuery) {
		JsonObject result = new JsonObject();
		result.add("testval", new JsonPrimitive(1));
		return result;
	}
}

class TestHandler implements HttpHandler {

	@Override
	public void handle(HttpExchange exchange) throws IOException {
		String response = "Test response\n";
		exchange.sendResponseHeaders(200, 0);
		OutputStream os = exchange.getResponseBody();
		os.write(response.getBytes());
		os.close();
	}
}

public class Main {
	public static void main(String[] args) {
		JedisPool pool = new JedisPool(new JedisPoolConfig(), "localhost");
		Jedis jedis = null;
		HttpServer server = null;
		
		try {
			jedis = pool.getResource();
			server = HttpServer.create(new InetSocketAddress(8000), 0);
			server.createContext("/test", new TestHandler());
			server.createContext("/testjedis.json", new TestJedisHandler(jedis));
			server.createContext("/testjson.json", new TestJsonHandler(jedis));
			server.createContext("/statuses/home_timeline.json", new TestHomeTimelineHandler(jedis));
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
}
