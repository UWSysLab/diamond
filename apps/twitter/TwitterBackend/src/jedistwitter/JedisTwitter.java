package jedistwitter;

import java.util.List;
import java.util.Map;
import java.util.Set;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonPrimitive;

import redis.clients.jedis.Jedis;

public class JedisTwitter {
	private Jedis jedis;
	
	public JedisTwitter(Jedis j) {
		jedis = j;
	}
	
	public JsonElement updateStatus(String screenName, String status, String replyIdString, long time) {
		String uidString = jedis.get("user:" + screenName + ":uid");
		String timeString = String.valueOf(time);
		
		if (uidString == null) {
			System.out.println("updateStatus error: no user with screenname " + screenName);
			return new JsonObject();
		}
		
		//create post hash
		long pid = jedis.incr("global:pid");
		String postKey = "pid:" + pid;
		jedis.hset(postKey, "content", status);
		jedis.hset(postKey, "uid", uidString);
		jedis.hset(postKey, "time", timeString);
		if (replyIdString != null) {
			jedis.hset(postKey, "reply", replyIdString);
		}

		//add to user timeline of poster
		jedis.rpush("uid:" + uidString + ":posts", String.valueOf(pid));

		//add to home timeline of poster and all of poster's followers
		jedis.rpush("uid:" + uidString + ":timeline", String.valueOf(pid));
		Set<String> followerUids = jedis.smembers("uid:" + uidString + ":followers");
		for (String followerUidString : followerUids) {
			jedis.rpush("uid:" + followerUidString + ":timeline", String.valueOf(pid));
		}
		
		return getTweet(pid, screenName);
	}

	public JsonElement addUser(String screenName, String name) {
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
		
		return getUser(uid);
	}

	public JsonElement getHomeTimeline(String screenName) {
		String uidString = jedis.get("user:" + screenName + ":uid");
		JsonArray result = new JsonArray();
		List<String> timelinePids = jedis.lrange("uid:" + uidString + ":timeline", 0, -1);
		for (int i = 0; i < timelinePids.size(); i++) {
			result.add(getTweet(Long.parseLong(timelinePids.get(i)), screenName));
		}
		return result;
	}

	public JsonElement getUserTimeline(long uid) {
		List<String> pids = jedis.lrange("uid:" + uid + ":posts", 0, -1);
		
		JsonArray result = new JsonArray();
		for (int i = 0; i < pids.size(); i++) {
			result.add(getTweet(Long.parseLong(pids.get(i)), null));
		}
		
		return result;
	}

	public JsonElement destroyFriendship(String screenName, long toUnfollowUid) {
		String unfollowerUidString = jedis.get("user:" + screenName + ":uid");
		
		jedis.srem("uid:" + unfollowerUidString + ":following", String.valueOf(toUnfollowUid));
		jedis.srem("uid:" + toUnfollowUid + ":followers", unfollowerUidString);
		
		return getUser(toUnfollowUid);
	}

	public JsonElement createFriendship(String username, long toFollowUid) {
		
		String followerUidString = jedis.get("user:" + username + ":uid");
		
		jedis.sadd("uid:" + followerUidString + ":following", String.valueOf(toFollowUid));
		jedis.sadd("uid:" + toFollowUid + ":followers", followerUidString);
		
		return getUser(toFollowUid);
	}

	public JsonElement getUser(long uid) {
		String userKey = "uid:" + uid;
		String name = jedis.hget(userKey, "name");
		String screenName = jedis.hget(userKey, "screen_name");
		
		JsonObject user = new JsonObject();
		user.add("id", new JsonPrimitive(uid));
		user.add("id_str", new JsonPrimitive(String.valueOf(uid)));
		user.add("screen_name", new JsonPrimitive(screenName));
		user.add("name", new JsonPrimitive(name));
		
		user.add("friends_count", new JsonPrimitive(jedis.scard("uid:" + uid + ":following")));
		user.add("followers_count", new JsonPrimitive(jedis.scard("uid:" + uid + ":followers")));
		return user;
	}
	
	public JsonElement getTweet(long pid, String authScreenName) {
		String postKey = "pid:" + pid;
		String content = jedis.hget(postKey, "content");
		String time = jedis.hget(postKey, "time");
		int uid = Integer.parseInt(jedis.hget(postKey, "uid"));
		JsonElement user = getUser(uid);

		JsonObject tweet = new JsonObject();
		tweet.add("text", new JsonPrimitive(content));
		tweet.add("id", new JsonPrimitive(pid));
		tweet.add("id_str", new JsonPrimitive(String.valueOf(pid)));
		tweet.add("created_at", new JsonPrimitive(time));
		tweet.add("user", user);
		
		String replyIdString = jedis.hget(postKey, "reply");
		long replyPid = -1;
		if (replyIdString != null) {
			replyPid = Long.parseLong(replyIdString);
		}
		if (replyPid >= 1) {
			String replyUidString = jedis.hget("pid:" + replyIdString, "uid");
			long replyUid = Long.parseLong(replyUidString);
			String replyScreenName = jedis.hget("uid:" + replyUidString, "screen_name");
			
			tweet.add("in_reply_to_screen_name", new JsonPrimitive(replyScreenName));
			tweet.add("in_reply_to_status_id", new JsonPrimitive(replyPid));
			tweet.add("in_reply_to_status_id_str", new JsonPrimitive(replyIdString));
			tweet.add("in_reply_to_user_id", new JsonPrimitive(replyUid));
			tweet.add("in_reply_to_user_id_str", new JsonPrimitive(replyUidString));
		}
		
		String retweetIdString = jedis.hget(postKey, "retweet");
		if (retweetIdString != null) {
			tweet.add("retweeted_status", getTweet(Long.parseLong(retweetIdString), authScreenName));
		}
		
		String retweeterSetKey = "pid:" + pid + ":retweeters";	
		long numRetweets = jedis.scard(retweeterSetKey);
		tweet.add("retweet_count", new JsonPrimitive(numRetweets));
		
		String favoriterSetKey = "pid:" + pid + ":favoriters";
		long numFavorites = jedis.scard(favoriterSetKey);
		tweet.add("favorite_count", new JsonPrimitive(numFavorites));
		
		String authUidString = jedis.get("user:" + authScreenName + ":uid");
		boolean retweeted = false;
		boolean favorited = false;
		if (authUidString != null) {
			retweeted = jedis.sismember(retweeterSetKey, authUidString);
			favorited = jedis.sismember(favoriterSetKey, authUidString);
		}
		tweet.add("retweeted", new JsonPrimitive(retweeted));
		tweet.add("favorited", new JsonPrimitive(favorited));

		return tweet;
	}

	public long getUid(String screenName) {
		String uidString = jedis.get("user:" + screenName + ":uid");
		return Long.parseLong(uidString);
	}
	
	public JsonElement getAllTweets() {
		JsonArray result = new JsonArray();
		for (int i = 1; i <= Long.parseLong(jedis.get("global:pid")); i++) {
			result.add(getTweet(i, null));
		}
		return result;
	}
	
	public JsonElement getAllUsers() {
		JsonArray result = new JsonArray();
		for (int i = 1; i <= Long.parseLong(jedis.get("global:uid")); i++) {
			result.add(getUser(i));
		}
		return result;
	}
	
	/**
	 * If either user_id or screen_name is provided in the parameter map,
	 * return the corresponding uid. If neither is provided, return -1.
	 */
	public long getUid(Map<String, String> params) {
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
	
	public JsonElement getFavorites(long uid) {
		Set<String> favorites = jedis.zrange("uid:" + uid + ":favorites", -1, -20);
		
		JsonArray result = new JsonArray();
		for (String s : favorites) {
			result.add(getTweet(Long.parseLong(s), null));
		}
		
		return result;
	}
	
	public JsonElement createFavorite(String screenName, long pid) {
		String uidString = jedis.get("user:" + screenName + ":uid");
		String ssKey = "uid:" + uidString + ":favorites";
		long score = jedis.zcard(ssKey);
		jedis.zadd(ssKey, score, String.valueOf(pid));
		
		jedis.sadd("pid:" + pid + ":favoriters", uidString);
		
		return getTweet(pid, screenName);
	}
	
	public JsonElement destroyFavorite(String screenName, long pid) {
		String uidString = jedis.get("user:" + screenName + ":uid");
		jedis.zrem("uid:" + uidString + ":favorites", String.valueOf(pid));
		return getTweet(pid, screenName);
	}

	//TODO: currently has duplicated code from updateStatus(): more elegant way to handle this?
	public JsonElement createRetweet(String screenName, long origPid, long time) {
		String retweeterUidString = jedis.get("user:" + screenName + ":uid");

		//create new Tweet
		String timeString = String.valueOf(time);
		
		if (retweeterUidString == null) {
			System.out.println("updateStatus error: no user with screenname " + screenName);
			return new JsonObject();
		}
		
		//create post hash
		long pid = jedis.incr("global:pid");
		String pidString = String.valueOf(pid);
		String postKey = "pid:" + pid;
		String origPostKey = "pid:" + origPid;
		String origUid = jedis.hget(origPostKey, "uid");
		String origScreenName = jedis.hget("uid:" + origUid, "screen_name");
		jedis.hset(postKey, "content", "RT @" + origScreenName + ": " + jedis.hget(origPostKey, "content"));
		jedis.hset(postKey, "uid", retweeterUidString);
		jedis.hset(postKey, "time", timeString);
		jedis.hset(postKey, "retweet", String.valueOf(origPid));

		//add to user timeline of poster
		jedis.rpush("uid:" + retweeterUidString + ":posts", pidString);

		//add to home timeline of poster and all of poster's followers
		jedis.rpush("uid:" + retweeterUidString + ":timeline", pidString);
		Set<String> followerUids = jedis.smembers("uid:" + retweeterUidString + ":followers");
		for (String followerUidString : followerUids) {
			jedis.rpush("uid:" + followerUidString + ":timeline", pidString);
		}
		
		//update retweet details of original Tweet
		jedis.sadd("pid:" + origPid + ":retweeters", retweeterUidString);
		jedis.sadd("pid:" + origPid + ":retweets", pidString);
		
		return getTweet(pid, screenName);
	}
	
	public JsonElement getRetweets(long pid) {
		Set<String> retweetPidStrings = jedis.smembers("pid:" + pid + ":retweets");
		JsonArray result = new JsonArray();
		for (String pidString : retweetPidStrings) {
			result.add(getTweet(Long.parseLong(pidString), null));
		}
		return result;
	}
}
