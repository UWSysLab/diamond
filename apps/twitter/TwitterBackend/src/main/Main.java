package main;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.util.List;
import java.util.Map;

import javax.json.Json;
import javax.json.JsonObject;
import javax.json.JsonStructure;
import javax.json.JsonWriter;

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
		JsonStructure responseJson = getResponseJson(requestMethod, requestURI.getQuery());
		exchange.sendResponseHeaders(200, 0);
		OutputStream os = exchange.getResponseBody();
		JsonWriter writer = Json.createWriter(os);
		writer.write(responseJson);
		os.close();
	}
	
	abstract JsonStructure getResponseJson(String requestMethod, String requestQuery);
	
}

class TestJedisHandler extends BaseJsonHandler {
	public TestJedisHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonStructure getResponseJson(String requestMethod, String requestQuery) {
		return Json.createObjectBuilder().add("global:uid", jedis.get("global:uid")).build();
	}
}

class TestJsonHandler extends BaseJsonHandler {
	public TestJsonHandler(Jedis j) {
		super(j);
	}

	@Override
	JsonStructure getResponseJson(String requestMethod, String requestQuery) {
		return Json.createObjectBuilder().add("testval", 1).build();
	}
}

class TestHandler implements HttpHandler {

	@Override
	public void handle(HttpExchange exchange) throws IOException {
		String requestMethod = exchange.getRequestMethod();
		URI requestURI = exchange.getRequestURI();
		String response = "Test response";
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
			server.createContext("/testjedis", new TestJedisHandler(jedis));
			server.createContext("/testjson", new TestJsonHandler(jedis));
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
