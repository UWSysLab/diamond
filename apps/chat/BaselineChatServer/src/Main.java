import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.List;

import com.google.gson.JsonArray;
import com.google.gson.JsonPrimitive;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import redis.clients.jedis.Jedis;
import redis.clients.jedis.JedisPool;
import redis.clients.jedis.JedisPoolConfig;

class ChatHandler implements HttpHandler {
	Jedis jedis;
	
	public ChatHandler(Jedis j) {
		jedis = j;
	}
	
	List<String> deserializeList(String str) {
		List<String> result = new ArrayList<String>();
		if (str != null) {
			String[] split = str.split("\n");
			for (int i = 0; i < split.length; i++) {
				result.add(split[i]);
			}
		}
		return result;
	}
	
	String serializeList(List<String> list) {
		StringBuilder sb = new StringBuilder();
		for (int i = 0; i < list.size(); i++) {
			sb.append(list.get(i));
			sb.append("\n");
		}
		return sb.toString();
	}
	
	@Override
	public void handle(HttpExchange exchange) throws IOException {
		String method = exchange.getRequestMethod();
		if (method.equals("GET")) {
			JsonArray responseJson = new JsonArray();
			
			List<String> chatLog = deserializeList(jedis.get("baselinechat:chatlog"));
			for (int i = 0; i < chatLog.size(); i++) {
				responseJson.add(new JsonPrimitive(chatLog.get(i)));
			}
			
			exchange.sendResponseHeaders(200, responseJson.toString().getBytes().length);
			OutputStream os = exchange.getResponseBody();
			os.write(responseJson.toString().getBytes());
			os.write('\n');
			os.close();
		}
		else if (method.equals("POST")) {
			InputStream requestBody = exchange.getRequestBody();
			byte[] bodyArray = new byte[requestBody.available()];
			requestBody.read(bodyArray);
			String bodyString = new String(bodyArray, "UTF-8");
			
			List<String> chatLog = deserializeList(jedis.get("baselinechat:chatlog"));
			chatLog.add(bodyString);
			if (chatLog.size() >= Main.MAX_SIZE) {
				chatLog.remove(0);
			}
			jedis.set("baselinechat:chatlog", serializeList(chatLog));
			jedis.waitReplicas(1, 3);
			
			exchange.sendResponseHeaders(200, 0);
			OutputStream os = exchange.getResponseBody();
			os.close();
		}
	}
}

public class Main {

	static final long MAX_SIZE = 100;
		
	public static void main(String[] args) {
		if (args.length < 1) {
			System.err.println("usage: java Main port");
			System.exit(0);
		}
		
		int port = Integer.parseInt(args[0]);
		
		HttpServer server = null;
		
		JedisPool pool = new JedisPool(new JedisPoolConfig(), "moranis.cs.washington.edu");
		Jedis jedis = null;
		
		try {
			jedis = pool.getResource();
			
			server = HttpServer.create(new InetSocketAddress(port), 0);
			server.createContext("/chat", new ChatHandler(jedis));
			server.setExecutor(null);
			server.start();
		}
		catch (IOException e) {
			System.err.println(e);
		}
		finally {
			jedis.close();
		}
	}
}
