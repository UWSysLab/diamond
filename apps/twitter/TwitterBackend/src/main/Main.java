package main;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.util.Map;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import redis.clients.jedis.Jedis;
import redis.clients.jedis.JedisPool;
import redis.clients.jedis.JedisPoolConfig;
import utils.JsonJedisUtils;
import utils.Utils;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.codec.binary.StringUtils;

abstract class BaseJsonHandler implements HttpHandler {
	protected Jedis jedis;
	
	public BaseJsonHandler(Jedis j) {
		jedis = j;
	}
	
	@Override
	public void handle(HttpExchange exchange) throws IOException {
		String requestMethod = exchange.getRequestMethod();
		Headers requestHeaders = exchange.getRequestHeaders();
		URI requestURI = exchange.getRequestURI();
		JsonElement responseJson = getResponseJson(requestMethod, requestHeaders, requestURI);
		exchange.sendResponseHeaders(200, 0);
		OutputStream os = exchange.getResponseBody();
		os.write(responseJson.toString().getBytes());
		os.close();
	}
	
	abstract JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestUR);
	
}

class HomeTimelineHandler extends BaseJsonHandler {
	public HomeTimelineHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI) {
		JsonArray result = new JsonArray();
		int numPosts = Integer.parseInt(jedis.get("global:pid"));
		for (int i = 1; i <= numPosts; i++) {
			result.add(JsonJedisUtils.getTweetJson(jedis, i));
		}
		return result;
	}
}

//TODO: Finish and debug this method
class UpdateHandler extends BaseJsonHandler {
	public UpdateHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI) {
		Map<String, String> queryParams = Utils.getQueryParams(requestURI);
		String username = Utils.getUsername(requestHeaders);
		String uidString = jedis.get("user:" + username + ":uid");
		if (uidString == null) {
			System.out.println("No user with this username");
			uidString = "1";
		}
		String status = queryParams.get("status");
		String timeString = String.valueOf(System.currentTimeMillis());
		
		long pid = jedis.incr("global:pid");
		String postKey = "pid:" + pid;
		System.out.println("Jedis start updating hash");
		jedis.hset(postKey, "content", status);
		jedis.hset(postKey, "uid", uidString);
		jedis.hset(postKey, "time", timeString);
		System.out.println("Jedis end updating hash");
		
		return JsonJedisUtils.getTweetJson(jedis, pid);
	}
}

class TestHomeTimelineHandler extends BaseJsonHandler {
	public TestHomeTimelineHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI) {
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
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI) {
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
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI) {
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
	public static void writeTestData(Jedis jedis) {
		jedis.flushDB();
		jedis.incr("global:uid");
		jedis.hset("uid:1", "name", "Sean Connery");
		jedis.hset("uid:1", "screen_name", "sconnery");
		jedis.incr("global:uid");
		jedis.hset("uid:2", "name", "Daniel Craig");
		jedis.hset("uid:2", "screen_name", "dcraig");
		jedis.incr("global:pid");
		jedis.hset("pid:1", "content", "Old James Bond movies are better");
		jedis.hset("pid:1", "uid", "1");
		jedis.hset("pid:1", "time", String.valueOf(System.currentTimeMillis()));
		jedis.incr("global:pid");
		jedis.hset("pid:2", "content", "No, newer James Bond movies are best");
		jedis.hset("pid:2", "uid", "2");
		jedis.hset("pid:2", "time", String.valueOf(System.currentTimeMillis()));
	}
	
	public static void main(String[] args) {
		JedisPool pool = new JedisPool(new JedisPoolConfig(), "localhost");
		Jedis jedis = null;
		HttpServer server = null;
		
		try {
			jedis = pool.getResource();
			
			writeTestData(jedis);
			
			server = HttpServer.create(new InetSocketAddress(8000), 0);
			server.createContext("/test", new TestHandler());
			server.createContext("/testjedis.json", new TestJedisHandler(jedis));
			server.createContext("/testjson.json", new TestJsonHandler(jedis));
			server.createContext("/statuses/home_timeline.json", new HomeTimelineHandler(jedis));
			server.createContext("/statuses/update.json", new UpdateHandler(jedis));
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
