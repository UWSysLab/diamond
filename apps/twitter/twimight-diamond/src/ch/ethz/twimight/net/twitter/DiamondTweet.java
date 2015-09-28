package ch.ethz.twimight.net.twitter;


public class DiamondTweet {
	public String text;
	public String screenname;
	public String name;
	public long createdAt;
	public String retweetedBy;
	public long userid;
	public int mentions;

	public boolean toFavorite;
	
	public DiamondTweet() {
		text = "placeholdertext";
		screenname = "placeholderscreenname";
		name = null;
		createdAt = 0;
		retweetedBy = null;
		userid = 0;
		mentions = 0;
	}
}
