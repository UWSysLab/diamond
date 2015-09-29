package ch.ethz.twimight.net.twitter;

import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.Diamond.DString;
import edu.washington.cs.diamond.Diamond.DLong;


public class DiamondTweet {
	public DString text;
	public DString screenname;
	public DLong createdAt;
	public DLong userid;
	
	public DiamondTweet() {
		text = new DString();
		screenname = new DString();
		createdAt = new DLong();
		userid = new DLong();
	}
	
	public String getScreenname() {
		return screenname.Value();
	}
	
	public String getText() {
		return text.Value();
	}
	
	public long getCreatedAt() {
		return createdAt.Value();
	}
	
	public long getUserId() {
		return userid.Value();
	}
	
	
	
	public String getRetweetedBy() {
		return null;
	}
	
	public boolean getToFavorite() {
		return false;
	}
	
	public long getMentions() {
		return 0;
	}
}
