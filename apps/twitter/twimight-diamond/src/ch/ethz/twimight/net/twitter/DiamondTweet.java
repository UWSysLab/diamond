package ch.ethz.twimight.net.twitter;

import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.Diamond.DString;
import edu.washington.cs.diamond.Diamond.DLong;


public class DiamondTweet {
	public DString text;
	public DString screenname;
	public DString inReplyToStatusId;
	public DLong createdAt;
	public DLong userid;
	
	public DiamondTweet() {
		text = new DString();
		screenname = new DString();
		inReplyToStatusId = new DString();
		createdAt = new DLong();
		userid = new DLong();
	}
	
	public String getScreenname() {
		return screenname.Value();
	}
	public void setScreenname(String s) {
		screenname.Set(s);
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
