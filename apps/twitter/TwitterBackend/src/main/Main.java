package main;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.util.List;
import java.util.Map;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import redis.clients.jedis.Jedis;
import redis.clients.jedis.JedisPool;
import redis.clients.jedis.JedisPoolConfig;

abstract class BaseHandler implements HttpHandler {
	protected Jedis jedis;
	
	public BaseHandler(Jedis j) {
		jedis = j;
	}
	
	@Override
	public void handle(HttpExchange exchange) throws IOException {
		String requestMethod = exchange.getRequestMethod();
		URI requestURI = exchange.getRequestURI();
		
		String response = getResponse(requestMethod, requestURI.getQuery());
		exchange.sendResponseHeaders(200, response.getBytes().length);
		OutputStream os = exchange.getResponseBody();
		os.write(response.getBytes());
		os.close();
	}
	
	abstract String getResponse(String requestMethod, String requestQuery);
	
}

class TestHandler extends BaseHandler {
	public TestHandler(Jedis j) {
		super(j);
	}

	@Override
	String getResponse(String requestMethod, String requestQuery) {
		System.out.println(requestQuery);

		return "Default response";
	}
}

class TestJedisHandler extends BaseHandler {
	public TestJedisHandler(Jedis j) {
		super(j);
	}

	@Override
	String getResponse(String requestMethod, String requestQuery) {
		return jedis.get("global:uid");
	}
}

/*class TestJsonHandler extends BaseHandler {
	public TestJsonHandler(Jedis j) {
		super(j);
	}

	@Override
	String getResponse(String requestMethod,
			Map<String, List<String>> requestHeaders, InputStream requestBody) {
		
	}
}*/

public class Main {
	public static void main(String[] args) {
		JedisPool pool = new JedisPool(new JedisPoolConfig(), "localhost");
		Jedis jedis = null;
		HttpServer server = null;
		
		try {
			jedis = pool.getResource();
			server = HttpServer.create(new InetSocketAddress(8000), 0);
			server.createContext("/test", new TestHandler(jedis));
			server.createContext("/testjedis", new TestJedisHandler(jedis));
			//server.createContext("/testjson", new TestJsonHandler(jedis));
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
