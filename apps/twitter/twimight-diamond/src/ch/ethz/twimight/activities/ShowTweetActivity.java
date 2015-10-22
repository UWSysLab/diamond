package ch.ethz.twimight.activities;

import java.util.ArrayList;

import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.view.ViewPager;
import ch.ethz.twimight.R;
import ch.ethz.twimight.fragments.ShowTweetFragment.OnTweetDeletedListener;
import ch.ethz.twimight.fragments.TweetListFragment;
import ch.ethz.twimight.fragments.adapters.ShowTweetPageAdapter;
import ch.ethz.twimight.net.twitter.DiamondTweet;
import ch.ethz.twimight.net.twitter.Tweets;
import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.Diamond.MappedObjectList;

public class ShowTweetActivity extends TwimightBaseActivity implements OnTweetDeletedListener {
	
	
	ContentResolver resolver;
	//String query;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);		
		
		ViewPager viewPager;
		resolver = getContentResolver();
		Intent intent = getIntent();
		
		int position = intent.getIntExtra("position", 0);		
		int type = intent.getIntExtra("type", TweetListFragment.TIMELINE_KEY);
		//if (type == TweetListFragment.SEARCH_TWEETS)
			//query = intent.getStringExtra(ListFragment.SEARCH_QUERY);
		
		MappedObjectList<DiamondTweet> tweetList = null;
		switch(type) {
		case TweetListFragment.TIMELINE_KEY:
			String uid = LoginActivity.getTwitterId(this.getBaseContext());
			String timelineKey = "twitter:uid:" + uid + ":timeline";
			tweetList = new MappedObjectList<DiamondTweet>(timelineKey,
					new Diamond.DefaultMapObjectFunction(), DiamondTweet.class, true);
			break;
		case TweetListFragment.SEARCH_TWEETS:
			String globalTimelineKey = "twitter:timeline";
			tweetList = new MappedObjectList<DiamondTweet>(globalTimelineKey,
					new Diamond.DefaultMapObjectFunction(), DiamondTweet.class, true);
			break;
		}
		if (tweetList != null) {			
			ShowTweetPageAdapter pageAdapter = new ShowTweetPageAdapter(getFragmentManager(), tweetList );
			viewPager = (ViewPager) findViewById(R.id.viewpager);			
			viewPager.setAdapter(pageAdapter);
			//viewPager.setOffscreenPageLimit(2);			
			viewPager.setCurrentItem(position);
		}
			
	}

	@Override
	public void onDelete() {
		finish();
		
	}

}
