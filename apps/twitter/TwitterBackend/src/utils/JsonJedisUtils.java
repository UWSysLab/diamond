package utils;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import redis.clients.jedis.Jedis;


public class JsonJedisUtils {
	public static JsonElement getTweetJson(Jedis jedis, long pid) {
		String postKey = "pid:" + pid;
		String content = jedis.hget(postKey, "content");
		String time = jedis.hget(postKey, "time");
		int uid = Integer.parseInt(jedis.hget(postKey, "uid"));
		JsonElement user = getUserJson(jedis, uid);
		
		JsonObject tweet = new JsonObject();
		tweet.add("text", new JsonPrimitive(content));
		tweet.add("id", new JsonPrimitive(pid));
		tweet.add("id_str", new JsonPrimitive(String.valueOf(pid)));
		tweet.add("created_at", new JsonPrimitive(time));
		tweet.add("user", user);
		return tweet;
	}
	
	public static JsonElement getUserJson(Jedis jedis, long uid) {
		String userKey = "uid:" + uid;
		String name = jedis.hget(userKey, "name");
		String screenName = jedis.hget(userKey, "screen_name");
		
		JsonObject user = new JsonObject();
		user.add("id", new JsonPrimitive(uid));
		user.add("id_str", new JsonPrimitive(String.valueOf(uid)));
		user.add("screen_name", new JsonPrimitive(screenName));
		user.add("name", new JsonPrimitive(name));
		return user;
	}
}
