package main;
import redis.clients.jedis.Jedis;
import redis.clients.jedis.JedisPool;
import redis.clients.jedis.JedisPoolConfig;

public class Main {
	public static void main(String[] args) {
		JedisPool pool = new JedisPool(new JedisPoolConfig(), "localhost");
		Jedis jedis = null;
		try {
			jedis = pool.getResource();
			System.out.println(jedis.get("global:uid"));
		}
		finally {
			jedis.close();
		}
		
		pool.destroy();
	}
}
