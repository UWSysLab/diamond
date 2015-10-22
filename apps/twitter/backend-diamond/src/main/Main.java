package main;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.util.Map;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import diamond.DiamondTweet;
import jedistwitter.JedisTwitter;
import redis.clients.jedis.Jedis;
import redis.clients.jedis.JedisPool;
import redis.clients.jedis.JedisPoolConfig;
import utils.Utils;

import edu.washington.cs.diamond.Diamond.DStringList;
import edu.washington.cs.diamond.Diamond.DString;
import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.Diamond.DLong;
import edu.washington.cs.diamond.Diamond.DObject;

abstract class BaseJsonHandler implements HttpHandler {
	protected JedisTwitter jedisTwitter;
	
	public BaseJsonHandler(JedisTwitter jt) {
		jedisTwitter = jt;
	}
	
	@Override
	public void handle(HttpExchange exchange) throws IOException {
		String requestMethod = exchange.getRequestMethod();
		Headers requestHeaders = exchange.getRequestHeaders();
		URI requestURI = exchange.getRequestURI();
		InputStream requestBody = exchange.getRequestBody();
		JsonElement responseJson = null;
		try {
			responseJson = getResponseJson(requestMethod, requestHeaders, requestURI, requestBody);
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		exchange.sendResponseHeaders(200, 0);
		OutputStream os = exchange.getResponseBody();
		os.write(responseJson.toString().getBytes());
		os.close();
		
		System.out.println(requestURI);
	}
	
	abstract JsonElement getResponseJson(String requestMethod, Headers requestHeaders,
			URI requestURI, InputStream requestBody);
	
}

class AddUserHandler extends BaseJsonHandler {
	public AddUserHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		
		Map<String, String> bodyParams = Utils.getBodyParams(requestBody);
		String screenName = bodyParams.get("screen_name");
		String name = bodyParams.get("name");
		
		return jedisTwitter.addUser(screenName, name);
	}
}

class VerifyCredentialsHandler extends BaseJsonHandler {

	public VerifyCredentialsHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI, InputStream requestBody) {
		String username = Utils.getUsername(requestHeaders);
		long uid = jedisTwitter.getUid(username);
		return jedisTwitter.getUser(uid, username);
	}
	
}

class ShowUserHandler extends BaseJsonHandler {

	public ShowUserHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI, InputStream requestBody) {
		String authScreenName = Utils.getUsername(requestHeaders);
		Map<String, String> queryParams = Utils.getQueryParams(requestURI);
		long uid = jedisTwitter.getUid(queryParams);
		return jedisTwitter.getUser(uid, authScreenName);
	}
	
}

class CreateFriendshipHandler extends BaseJsonHandler {

	public CreateFriendshipHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI, InputStream requestBody) {
		String username = Utils.getUsername(requestHeaders);
		Map<String, String> bodyParams = Utils.getBodyParams(requestBody);
		long toFollowUid = jedisTwitter.getUid(bodyParams);
		
		if (toFollowUid == -1) {
			System.out.println("CreateFriendshipHandler error: must specify either screen name or user id to follow");
			return new JsonObject();
		}
		
		return jedisTwitter.createFriendship(username, toFollowUid);
	}
	
}

class DestroyFriendshipHandler extends BaseJsonHandler {

	public DestroyFriendshipHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI, InputStream requestBody) {
		String username = Utils.getUsername(requestHeaders);
		Map<String, String> bodyParams = Utils.getBodyParams(requestBody);
		long toUnfollowUid = jedisTwitter.getUid(bodyParams);
		
		if (toUnfollowUid == -1) {
			System.out.println("DestroyFriendshipHandler error: must specify either screen name or user id to unfollow");
			return new JsonObject();
		}
		
		return jedisTwitter.destroyFriendship(username, toUnfollowUid);

	}
	
}

class UserTimelineHandler extends BaseJsonHandler {
	public UserTimelineHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		
		Map<String, String> queryParams = Utils.getQueryParams(requestURI);
		long uid = jedisTwitter.getUid(queryParams);
		
		boolean includeRetweets = true;
		if (queryParams.containsKey("include_rts") && queryParams.get("include_rts").equals("false")) {
			includeRetweets = false;
		}
		
		if (uid == -1) {
			System.out.println("UserTimelineHandler error: must specify either screen name or user id");
			return new JsonObject();
		}
		
		return jedisTwitter.getUserTimeline(uid, includeRetweets);

	}
}

class HomeTimelineHandler extends BaseJsonHandler {
	public HomeTimelineHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		String username = Utils.getUsername(requestHeaders);
		return jedisTwitter.getHomeTimeline(username);
	}
}

class UpdateHandler extends BaseJsonHandler {
	public UpdateHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		
		Map<String, String> bodyParams =  Utils.getBodyParams(requestBody);
		
		String username = Utils.getUsername(requestHeaders);
		String status = bodyParams.get("status");
		String inReplyToIdString = bodyParams.get("in_reply_to_status_id");
		long time = System.currentTimeMillis();

		if (status == null) {
			System.out.println("UpdateHandler error: update request with no status parameter");
			return new JsonObject();
		}
				
		return jedisTwitter.updateStatus(username, status, inReplyToIdString, time);
	}
}

class ListFavoritesHandler extends BaseJsonHandler {
	public ListFavoritesHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		Map<String, String> queryParams = Utils.getQueryParams(requestURI);
		long uid = jedisTwitter.getUid(queryParams);
		
		//list favorites of authenticating user if no user is specified
		if (uid == -1) {
			String username = Utils.getUsername(requestHeaders);
			uid = jedisTwitter.getUid(username);
		}
		
		return jedisTwitter.getFavorites(uid);
	}
}

class CreateFavoritesHandler extends BaseJsonHandler {
	public CreateFavoritesHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		String username = Utils.getUsername(requestHeaders);
		
		Map<String, String> bodyParams = Utils.getBodyParams(requestBody);
		long pid = Long.parseLong(bodyParams.get("id"));
		
		return jedisTwitter.createFavorite(username, pid);
	}
}

class DestroyFavoritesHandler extends BaseJsonHandler {
	public DestroyFavoritesHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		String username = Utils.getUsername(requestHeaders);
		
		Map<String, String> bodyParams = Utils.getBodyParams(requestBody);
		long pid = Long.parseLong(bodyParams.get("id"));
		
		return jedisTwitter.destroyFavorite(username, pid);
	}
}

class RetweetHandler extends BaseJsonHandler {
	public RetweetHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {

		String[] uriSplit = requestURI.getPath().split("/");
		String origPidString = uriSplit[uriSplit.length - 1].split("\\.")[0];
		long origPid = Long.parseLong(origPidString);
		
		long time = System.currentTimeMillis();
		
		if (requestMethod.equalsIgnoreCase("POST")) {
			String screenName = Utils.getUsername(requestHeaders);
			
			return jedisTwitter.createRetweet(screenName, origPid, time);
		}
		else if (requestMethod.equalsIgnoreCase("GET")) {
			return jedisTwitter.getRetweets(origPid);
		}
		else {
			System.out.println("RetweetHandler error: unhandled request type: " + requestMethod);
			return new JsonObject();
		}

	}
}

class DestroyStatusHandler extends BaseJsonHandler {
	public DestroyStatusHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {

		String[] uriSplit = requestURI.getPath().split("/");
		String pidString = uriSplit[uriSplit.length - 1].split("\\.")[0];
		long pid = Long.parseLong(pidString);
				
		String screenName = Utils.getUsername(requestHeaders);
			
		return jedisTwitter.destroyStatus(screenName, pid);
	}
}

class HackSearchTweetsHandler extends BaseJsonHandler {
	public HackSearchTweetsHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		JsonObject result = new JsonObject();
		result.add("statuses", jedisTwitter.getAllTweets());
		return result;
	}
}

class HackSearchUsersHandler extends BaseJsonHandler {
	public HackSearchUsersHandler(JedisTwitter jt) {
		super(jt);
	}

	@Override
	JsonElement getResponseJson(String requestMethod, Headers requestHeaders, URI requestURI,
			InputStream requestBody) {
		return jedisTwitter.getAllUsers();
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

class DebugHandler implements HttpHandler {

	@Override
	public void handle(HttpExchange exchange) throws IOException {
		System.out.println(exchange.getRequestURI());
		String response = "Test response\n";
		exchange.sendResponseHeaders(200, 0);
		OutputStream os = exchange.getResponseBody();
		os.write(response.getBytes());
		os.close();
	}
}

public class Main {
	public static void writeTestData(JedisTwitter jedisTwitter) {
		jedisTwitter.addUser("sconnery", "Sean Connery");
		jedisTwitter.addUser("dcraig", "Daniel Craig");
		jedisTwitter.addUser("a", "a");
		jedisTwitter.createFriendship("a", jedisTwitter.getUid("sconnery"));
		jedisTwitter.createFriendship("a", jedisTwitter.getUid("dcraig"));
		jedisTwitter.updateStatus("sconnery", "Old James Bond movies are better", null, System.currentTimeMillis());
		jedisTwitter.updateStatus("dcraig", "@sconnery No, newer James Bond movies are best", "1", System.currentTimeMillis());
		jedisTwitter.updateStatus("sconnery", "Hello", null, System.currentTimeMillis());
		jedisTwitter.updateStatus("dcraig", "This is a tweet", null, System.currentTimeMillis());
		jedisTwitter.updateStatus("sconnery", "We are famous James Bond actors", null, System.currentTimeMillis());
		jedisTwitter.updateStatus("dcraig", "I like pretending to hate the franchise during contract negotations", null, System.currentTimeMillis());
		jedisTwitter.updateStatus("sconnery", "Diamonds are Forever", null, System.currentTimeMillis());
		jedisTwitter.updateStatus("sconnery", "Was that a Timothy Dalton movie?", null, System.currentTimeMillis());
		jedisTwitter.updateStatus("dcraig", "We need two more tweets", null, System.currentTimeMillis());
		jedisTwitter.updateStatus("dcraig", "Last", null, System.currentTimeMillis());

		
		//jedisTwitter.addUser("a", "a");
		//jedisTwitter.updateStatus("a", "Testing Diamond", null, 10000000);
		/*DStringList timelineList = new DStringList();
		DObject.Map(timelineList, "twitter:uid:1:timeline");
		timelineList.Append("twitter:testTweet1");*/
		
		/*DiamondTweet testTweet1 = new DiamondTweet();
		Diamond.MapObject(testTweet1, "twitter:testTweet1", new Diamond.DefaultMapObjectFunction());
		testTweet1.setText("Testing Diamond");
		testTweet1.setScreenname("diamonduser42");
		testTweet1.setCreatedAt(0);
		testTweet1.setUserId(1);*/
	}
	
	public static void main(String[] args) {
		JedisPool pool = new JedisPool(new JedisPoolConfig(), "moranis.cs.washington.edu");
		Jedis jedis = null;
		HttpServer server = null;
		
		Diamond.DiamondInit("moranis.cs.washington.edu");
		Diamond.DObject.SetGlobalRedisWait(true, 1, 3);

		try {
			jedis = pool.getResource();
			
			JedisTwitter jedisTwitter = new JedisTwitter(jedis);
			
			jedis.flushDB();
			writeTestData(jedisTwitter);
			
			server = HttpServer.create(new InetSocketAddress(8000), 0);
			//server.createContext("/", new DebugHandler());
			server.createContext("/test", new TestHandler());
			server.createContext("/statuses/home_timeline.json", new HomeTimelineHandler(jedisTwitter));
			server.createContext("/statuses/user_timeline.json", new UserTimelineHandler(jedisTwitter));
			server.createContext("/statuses/update.json", new UpdateHandler(jedisTwitter));
			server.createContext("/statuses/destroy", new DestroyStatusHandler(jedisTwitter));
			server.createContext("/statuses/retweet", new RetweetHandler(jedisTwitter));
			server.createContext("/friendships/create.json", new CreateFriendshipHandler(jedisTwitter));
			server.createContext("/friendships/destroy.json", new DestroyFriendshipHandler(jedisTwitter));
			server.createContext("/users/show.json", new ShowUserHandler(jedisTwitter));
			server.createContext("/favorites/list.json", new ListFavoritesHandler(jedisTwitter));
			server.createContext("/favorites/create.json", new CreateFavoritesHandler(jedisTwitter));
			server.createContext("/favorites/destroy.json", new DestroyFavoritesHandler(jedisTwitter));
			server.createContext("/account/verify_credentials.json", new VerifyCredentialsHandler(jedisTwitter));
			server.createContext("/search/tweets.json", new HackSearchTweetsHandler(jedisTwitter));
			server.createContext("/users/search.json", new HackSearchUsersHandler(jedisTwitter));
			server.createContext("/hack/adduser.json", new AddUserHandler(jedisTwitter));
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
