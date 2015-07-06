package utils;

import java.net.URI;
import java.util.Map;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;
import redis.clients.jedis.Jedis;


public class JedisUtils {
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
		
		user.add("friends_count", new JsonPrimitive(jedis.llen("uid:" + uid + ":following")));
		user.add("followers_count", new JsonPrimitive(jedis.llen("uid:" + uid + ":followers")));
		return user;
	}
	
	/**
	 * If either user_id or screen_name is provided in the parameter map,
	 * return the corresponding uid. If neither is provided, return -1.
	 */
	public static long getUid(Jedis jedis, Map<String, String> params) {
		String uidString = params.get("user_id");
		String screenNameString = params.get("screen_name");
		
		if (screenNameString == null && uidString == null) {
			return -1;
		}
		
		long uid;
		if (uidString == null) {
			uid = Long.parseLong(jedis.get("user:" + screenNameString + ":uid"));
		}
		else {
			uid = Long.parseLong(uidString);
		}
		
		return uid;
	}
}
