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

class ChatHandler implements HttpHandler {
	List<String> chatLog;
	
	public ChatHandler(List<String> inLog) {
		chatLog = inLog;
	}
	
	@Override
	public void handle(HttpExchange exchange) throws IOException {
		String method = exchange.getRequestMethod();
		if (method.equals("GET")) {
			JsonArray responseJson = new JsonArray();
			for (int i = 0; i < chatLog.size(); i++) {
				responseJson.add(new JsonPrimitive(chatLog.get(i)));
			}
			
			exchange.sendResponseHeaders(200, responseJson.toString().getBytes().length);
			OutputStream os = exchange.getResponseBody();
			os.write(responseJson.toString().getBytes());
			os.close();
		}
		else if (method.equals("POST")) {
			InputStream requestBody = exchange.getRequestBody();
			byte[] bodyArray = new byte[requestBody.available()];
			requestBody.read(bodyArray);
			String bodyString = new String(bodyArray, "UTF-8");
			
			chatLog.add(bodyString);
			if (chatLog.size() >= Main.MAX_SIZE) {
				chatLog.remove(0);
			}
			
			exchange.sendResponseHeaders(200, 0);
			OutputStream os = exchange.getResponseBody();
			os.close();
		}
	}
}

public class Main {

	static final long MAX_SIZE = 100;
	static final int PORT = 9000;
	
	static List<String> chatLog;
	
	public static void main(String[] args) {
		chatLog = new ArrayList<String>();
		HttpServer server = null;
		try {
			server = HttpServer.create(new InetSocketAddress(PORT), 0);
			server.createContext("/chat", new ChatHandler(chatLog));
			server.setExecutor(null);
			server.start();
		}
		catch (IOException e) {
			System.err.println(e);
		}
	}
}
