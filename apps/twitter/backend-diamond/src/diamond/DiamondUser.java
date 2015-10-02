package diamond;

import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.Diamond.DString;
import edu.washington.cs.diamond.Diamond.DStringList;
import edu.washington.cs.diamond.Diamond.DList;
import edu.washington.cs.diamond.Diamond.DLong;
import edu.washington.cs.diamond.Diamond.DSet;

public class DiamondUser {
	public DString name;
	public DString screenname;
	public DLong id;
	public DSet following;
	public DSet followers;
	public DStringList posts;
	public DStringList timeline;
	public DList favorites;
	
	public DiamondUser() {
		name = new DString();
		screenname = new DString();
		id = new DLong();
		following = new DSet();
		followers = new DSet();
		posts = new DStringList();
		timeline = new DStringList();
		favorites = new DList();
	}
	
	public void setScreenname(String s) {
		screenname.Set(s);
	}
	public String getScreenname() {
		return screenname.Value();
	}
	
	public void setName(String s) {
		name.Set(s);
	}
	public String getName() {
		return name.Value();
	}
	
	public long getId() {
		return id.Value();
	}
	public void setId(long l) {
		id.Set(l);
	}
	public boolean isFollowing(DiamondUser u) {
		if (following.InSet(u.getId())) {
			return true;
		}
		return false;
	}
	
	public long getNumTweets() {
		return posts.Size();
	}
	public long getNumFavorites() {
		return favorites.Size();
	}
	public long getNumFollowing() {
		return following.Size();
	}
	public long getNumFollowers() {
		return followers.Size();
	}
}
