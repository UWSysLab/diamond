import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.eclipse.jetty.server.Request;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.server.handler.AbstractHandler;

import com.google.gson.JsonArray;
import com.google.gson.JsonPrimitive;

import redis.clients.jedis.Jedis;
import redis.clients.jedis.JedisPool;
import redis.clients.jedis.JedisPoolConfig;

class ChatHandler extends AbstractHandler {
	JedisPool pool;
	
	public ChatHandler(JedisPool p) {
		pool = p;
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
	public void handle(String target, Request baseRequest, HttpServletRequest request, HttpServletResponse response)
			throws IOException, ServletException {
		Jedis jedis = pool.getResource();
		String method = request.getMethod();
		
		JsonArray responseJson = new JsonArray();
		int responseCode = HttpServletResponse.SC_BAD_REQUEST;
		
		if (method.equals("GET")) {
			String serializedList = jedis.get("baselinechat:chatlog");
			List<String> chatLog = deserializeList(serializedList);
			for (int i = 0; i < chatLog.size(); i++) {
				responseJson.add(new JsonPrimitive(chatLog.get(i)));
			}
			responseCode = HttpServletResponse.SC_OK;
		}
		else if (method.equals("POST")) {
			InputStream requestBody = request.getInputStream();
			byte[] bodyArray = new byte[requestBody.available()];
			requestBody.read(bodyArray);
			String bodyString = new String(bodyArray, "UTF-8");
			
			List<String> chatLog = deserializeList(jedis.get("baselinechat:chatlog"));
			chatLog.add(bodyString);
			if (chatLog.size() >= BaselineChatServer.MAX_SIZE) {
				chatLog.remove(0);
			}
			jedis.set("baselinechat:chatlog", serializeList(chatLog));
			jedis.waitReplicas(1, 3);
			responseCode = HttpServletResponse.SC_OK;
		}

		response.setStatus(responseCode);
		PrintWriter out = response.getWriter();
		out.print(responseJson.toString());
		baseRequest.setHandled(true);
		
		pool.returnResource(jedis);
	}
}

public class BaselineChatServer {
	JedisPool pool;

	static final long MAX_SIZE = 100;
	
	public void start(int port, String redisHostname, int redisPort) {		
		pool = new JedisPool(new JedisPoolConfig(), redisHostname, redisPort);
		
		Server server = null;
		try {			
			server = new Server(port);
			server.setHandler(new ChatHandler(pool));
			server.start();
			server.join();
		}
		catch (IOException e) {
			e.printStackTrace();
			System.exit(1);
		} catch (Exception e) {
			e.printStackTrace();
			System.exit(1);
		}
		
		pool.destroy();
	}
		
	public static void main(String[] args) {
		if (args.length < 3) {
			System.err.println("usage: java Main port redis-hostname redis-port");
			System.exit(0);
		}
		
		int port = Integer.parseInt(args[0]);
		String redisHostname = args[1];
		int redisPort = Integer.parseInt(args[2]);
		
		new BaselineChatServer().start(port, redisHostname, redisPort);
	}
}
