package main;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.util.List;
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
import utils.JedisUtils;
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
		InputStream requestBody = exchange.getRequestBody();
		JsonElement responseJson = getResponseJson(requestMethod, requestHeaders, requestURI, requestBody);
		exchange.sendResponseHeaders(200, 0);
		OutputStream os = exchange.getResponseBody();
		os.write(responseJson.toString().getBytes());
		os.close();
	}
	
	abstract JsonElement getResponseJson(String requestMethod, Headers requestHeaders,
			URI requestURI, InputStream requestBody);
	
}

class AddUserHandler extends BaseJsonHandler {
	public AddUserHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		
		Map<String, String> bodyParams = Utils.getBodyParams(requestBody);
		String screenName = bodyParams.get("screen_name");
		String name = bodyParams.get("name");
		String userBackrefKey = "user:" + screenName + ":uid";
		long uid;
		
		if (jedis.get(userBackrefKey) == null) {
			uid = jedis.incr("global:uid");
			String userKey = "uid:" + uid;
			jedis.set(userBackrefKey, String.valueOf(uid));
			jedis.hset(userKey, "screen_name", screenName);
			jedis.hset(userKey, "name", name);
		}
		else {
			uid = Long.parseLong(jedis.get(userBackrefKey));
		}
		
		return JedisUtils.getUserJson(jedis, uid);
	}
}

class UserTimelineHandler extends BaseJsonHandler {
	public UserTimelineHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		
		long uid = JedisUtils.getUidFromQuery(jedis, requestURI);
		
		if (uid == -1) {
			System.out.println("UserTimelineHandler error: must specify either screen name or user id");
			return new JsonObject();
		}
		
		List<String> pids = jedis.lrange("uid:" + uid + ":posts", 0, -1);
		
		JsonArray result = new JsonArray();
		for (int i = 0; i < pids.size(); i++) {
			result.add(JedisUtils.getTweetJson(jedis, Long.parseLong(pids.get(i))));
		}
		
		return result;
	}
}

class HomeTimelineHandler extends BaseJsonHandler {
	public HomeTimelineHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		JsonArray result = new JsonArray();
		int numPosts = Integer.parseInt(jedis.get("global:pid"));
		for (int i = 1; i <= numPosts; i++) {
			result.add(JedisUtils.getTweetJson(jedis, i));
		}
		return result;
	}
}

class UpdateHandler extends BaseJsonHandler {
	public UpdateHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		
		Map<String, String> bodyParams =  Utils.getBodyParams(requestBody);
		
		String username = Utils.getUsername(requestHeaders);
		String uidString = jedis.get("user:" + username + ":uid");

		String status = bodyParams.get("status");
		String timeString = String.valueOf(System.currentTimeMillis());

		
		if (uidString == null) {
			System.out.println("UpdateHandler error: no user with username " + username);
			return new JsonObject();
		}
		if (status == null) {
			System.out.println("UpdateHandler error: update request with no status parameter");
			return new JsonObject();
		}
		
		long pid = jedis.incr("global:pid");
		String postKey = "pid:" + pid;
		jedis.hset(postKey, "content", status);
		jedis.hset(postKey, "uid", uidString);
		jedis.hset(postKey, "time", timeString);
		
		jedis.rpush("uid:" + uidString + ":posts", String.valueOf(pid));
		
		return JedisUtils.getTweetJson(jedis, pid);
	}
}

class TestHomeTimelineHandler extends BaseJsonHandler {
	public TestHomeTimelineHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
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
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
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
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
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
			server.createContext("/statuses/user_timeline.json", new UserTimelineHandler(jedis));
			server.createContext("/statuses/update.json", new UpdateHandler(jedis));
			server.createContext("/hack/adduser.json", new AddUserHandler(jedis));
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
