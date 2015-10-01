package ch.ethz.twimight.fragments.adapters;


import java.util.ArrayList;

import android.app.Fragment;
import android.app.FragmentManager;
import android.support.v13.app.FragmentPagerAdapter;
import android.util.Log;
import ch.ethz.twimight.fragments.DiamondShowTweetFragment;
import ch.ethz.twimight.fragments.ShowTweetFragment;
import ch.ethz.twimight.net.twitter.DiamondTweet;
import edu.washington.cs.diamond.Diamond.MappedObjectList;

public class ShowTweetPageAdapter extends FragmentPagerAdapter {
    
	MappedObjectList<DiamondTweet> list;
	private static final String TAG = "ShowTweetPageAdapter";
	
	public ShowTweetPageAdapter(FragmentManager fm, MappedObjectList<DiamondTweet> list){
		super(fm);
		this.list = list;
		if (list == null) {
			Log.e(TAG, "tweet list is null");
		}
	}
	
	@Override
	public Fragment getItem(int pos) {
		long rowId = list.Get(pos).getId();
		Log.i(TAG, "rowId: " + rowId);
		return DiamondShowTweetFragment.newInstance(rowId);
	}

	@Override
	public int getCount() {
		// TODO Auto-generated method stub
		return list.Size();
	}



	
	

}
