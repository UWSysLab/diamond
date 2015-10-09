package ch.ethz.twimight.net.twitter;

import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.Diamond.DString;
import edu.washington.cs.diamond.Diamond.DCounter;
import edu.washington.cs.diamond.Diamond.DLong;


public class DiamondTweet {
	public DString text;
	public DString screenname;
	public DString inReplyToStatusId;
	public DLong createdAt;
	public DLong userid;
	public DLong id;
	public DCounter numFavorites;
	public DString name;
	
	public DiamondTweet() {
		text = new DString();
		screenname = new DString();
		inReplyToStatusId = new DString();
		createdAt = new DLong();
		userid = new DLong();
		id = new DLong();
		numFavorites = new DCounter();
		name = new DString();
	}
	
	public String getScreenname() {
		return screenname.Value();
	}
	public void setScreenname(String s) {
		screenname.Set(s);
	}
	
	public String getName() {
		return name.Value();
	}
	public void setName(String s) {
		name.Set(s);
	}
	
	public String getText() {
		return text.Value();
	}
	public void setText(String s) {
		text.Set(s);
	}
	
	public long getCreatedAt() {
		return createdAt.Value();
	}
	public void setCreatedAt(long l) {
		createdAt.Set(l);
	}
	
	public long getId() {
		return id.Value();
	}
	public void setId(long l) {
		id.Set(l);
	}
	
	public long getUserId() {
		return userid.Value();
	}
	public void setUserId(long l) {
		userid.Set(l);
	}
	
	public String getInReplyToStatusId() {
		return inReplyToStatusId.Value();
	}
	public void setInReplyToStatusId(String s) {
		inReplyToStatusId.Set(s);
	}
	
	public int getNumFavorites() {
		return numFavorites.Value();
	}
	public void incrNumFavorites() {
		numFavorites.Increment();
	}
	public void decrNumFavorites() {
		numFavorites.Decrement();
	}
	
	public String getRetweetedBy() {
		return null;
	}
	
	public long getNumRetweets() {
		return 0;
	}
	
	public boolean getToFavorite() {
		return false;
	}
	
	public long getMentions() {
		return 0;
	}
}
