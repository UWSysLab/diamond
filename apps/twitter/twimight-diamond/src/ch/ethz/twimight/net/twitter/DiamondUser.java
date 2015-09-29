package ch.ethz.twimight.net.twitter;

import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.Diamond.DString;
import edu.washington.cs.diamond.Diamond.DLong;


public class DiamondUser {
	public DString name;
	public DString screenname;
	
	public DiamondUser() {
		name = new DString();
		screenname = new DString();
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
}
