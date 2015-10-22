/*******************************************************************************
 * Copyright (c) 2011 ETH Zurich.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Public License v2.0
 * which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * 
 * Contributors:
 *     Paolo Carta - Implementation
 *     Theus Hossmann - Implementation
 *     Dominik Schatzmann - Message specification
 ******************************************************************************/
package ch.ethz.twimight.activities;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.v4.view.ViewPager;
import android.util.Log;
import android.widget.Toast;
import ch.ethz.twimight.R;
import ch.ethz.twimight.data.StatisticsDBHelper;
import ch.ethz.twimight.fragments.TweetListFragment;
import ch.ethz.twimight.fragments.adapters.ListViewPageAdapter;
import ch.ethz.twimight.listeners.TabListener;
import ch.ethz.twimight.location.LocationHelper;
import ch.ethz.twimight.net.twitter.DiamondTweet;
import ch.ethz.twimight.util.Constants;
import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.DiamondUtil;



/**
 * The main Twimight view showing the Timeline, favorites and mentions
 * @author thossmann
 * 
 */
public class ShowTweetListActivity extends TwimightBaseActivity{

	private static final String TAG = "ShowTweetListActivity";	
	
		
	public static boolean running= false;
	// handler
	static Handler handler;


	
	//LOGS
	LocationHelper locHelper ;
	long timestamp;	
	ConnectivityManager cm;
	StatisticsDBHelper locDBHelper;	
	CheckLocation checkLocation;
	public static final String ON_PAUSE_TIMESTAMP = "onPauseTimestamp";	
	
	ActionBar actionBar;
	public static final String FILTER_REQUEST = "filter_request";

	ViewPager viewPager;
	ListViewPageAdapter pagAdapter;
	
	/** 
	 * Called when the activity is first created. 
	 */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(null);				
		setContentView(R.layout.main);
					
		//statistics
		locDBHelper = new StatisticsDBHelper(getApplicationContext());
		locDBHelper.open();
		
		cm = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);
		timestamp = System.currentTimeMillis();
		
		locHelper = LocationHelper.getInstance(this);
		locHelper.registerLocationListener();
		
		handler = new Handler();
		checkLocation = new CheckLocation();
		handler.postDelayed(checkLocation, 1*60*1000L);	
		
		Bundle bundle = new Bundle();
		bundle.putInt(ListViewPageAdapter.BUNDLE_TYPE, ListViewPageAdapter.BUNDLE_TYPE_TWEETS);
		pagAdapter = new ListViewPageAdapter(getFragmentManager(), bundle);		
        
		viewPager = (ViewPager)  findViewById(R.id.viewpager);	
		
		viewPager.setAdapter(pagAdapter);
		viewPager.setOffscreenPageLimit(2);
		viewPager.setOnPageChangeListener(
	            new ViewPager.SimpleOnPageChangeListener() {
	                @Override
	                public void onPageSelected(int position) {
	                    // When swiping between pages, select the
	                    // corresponding tab.	                	
	                    getActionBar().setSelectedNavigationItem(position);
	                }
	            });

		//action bar
		actionBar = getActionBar();	
		actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);

		Tab tab = actionBar.newTab()
				.setIcon(R.drawable.ic_twimight_speech)
				.setTabListener(new TabListener(viewPager));
		actionBar.addTab(tab);

		tab = actionBar.newTab()
				.setIcon(R.drawable.ic_twimight_favorites)
				.setTabListener(new TabListener(viewPager));
		actionBar.addTab(tab);

		tab = actionBar.newTab()
				.setIcon(R.drawable.ic_twimight_mentions)
				.setTabListener(new TabListener(viewPager ));
		actionBar.addTab(tab);		


		boolean benchmark = true;
		if (benchmark) {
			doBenchmark(getBaseContext());
			//new BenchmarkTask().execute();
		}
	}
		


	private class CheckLocation implements Runnable {

		@Override
		public void run() {

			if (locHelper != null && locHelper.getCount() > 0 && locDBHelper != null && cm.getActiveNetworkInfo() != null) {	
				Log.i(TAG,"writing log");
				locDBHelper.insertRow(locHelper.getLocation(), cm.getActiveNetworkInfo().getTypeName(), StatisticsDBHelper.APP_STARTED, null, timestamp);
				locHelper.unRegisterLocationListener();

			} else {}
			
		}
		
	}
	

	@Override
	protected void onNewIntent(Intent intent) {		
		setIntent(intent);	
	}



	/**
	 * On resume
	 */
	@Override
	public void onResume(){

		super.onResume();
		running = true;

		Intent intent = getIntent();

		if(intent.hasExtra(FILTER_REQUEST)) {
			viewPager.setCurrentItem(intent.getIntExtra(FILTER_REQUEST, TweetListFragment.TIMELINE_KEY));
			intent.removeExtra(FILTER_REQUEST);

		}

		Long pauseTimestamp =  getOnPauseTimestamp(this);
		if (pauseTimestamp != 0 &&  (System.currentTimeMillis()-pauseTimestamp) > 10 * 60 * 1000L ) {
			handler = new Handler();			
			handler.post(new CheckLocation());

		}		


	}
    


	@Override
	protected void onPause() {
		
		super.onPause();
		setOnPauseTimestamp(System.currentTimeMillis(), this);
	}


	/**
	 * 
	 * @param id
	 * @param context
	 */
	private static void setOnPauseTimestamp(long timestamp, Context context) {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		SharedPreferences.Editor prefEditor = prefs.edit();
		prefEditor.putLong(ON_PAUSE_TIMESTAMP, timestamp);
		prefEditor.commit();
	}
	
	/**
	 * Gets the Twitter ID from shared preferences
	 * @param context
	 * @return
	 */
	public static Long getOnPauseTimestamp(Context context) {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		return prefs.getLong(ON_PAUSE_TIMESTAMP, 0);
	}


	@Override
	protected void onStop() {
		running=false;
		locHelper.unRegisterLocationListener();
		super.onStop();
	
		
	}
	

	/**
	 * Called at the end of the Activity lifecycle
	 */
	@Override
	public void onDestroy(){
		super.onDestroy();
		running = false;
		
		pagAdapter = null;
		viewPager = null;
		
		Log.i(TAG,"setting dd and dn to null");
		dd.setCallback(null);
		dn.setCallback(null);
		dd = null;
		dn = null;
		actionBar = null;
		
		Log.i(TAG,"destroying main activity");
		if ((System.currentTimeMillis() - timestamp <= 1 * 60 * 1000L)&& locHelper!=null && locDBHelper != null && 
				cm.getActiveNetworkInfo() != null) {
			
			if (locHelper.getCount() > 0 && cm.getActiveNetworkInfo() != null ) {				
				handler.removeCallbacks(checkLocation);				
				locDBHelper.insertRow(locHelper.getLocation(), cm.getActiveNetworkInfo().getTypeName(), StatisticsDBHelper.APP_STARTED , null, timestamp);
			} else {}
		}
		
		if ((locHelper != null && locHelper.getCount() > 0) && locDBHelper != null && cm.getActiveNetworkInfo() != null) {				
			locDBHelper.insertRow(locHelper.getLocation(), cm.getActiveNetworkInfo().getTypeName(), StatisticsDBHelper.APP_CLOSED , null, System.currentTimeMillis());
		} else {}

		TwimightBaseActivity.unbindDrawables(findViewById(R.id.rootRelativeLayout));	
		
		if(PreferenceManager.getDefaultSharedPreferences(this).getBoolean("prefDisasterMode", Constants.DISASTER_DEFAULT_ON) == true)
			Toast.makeText(this, getString(R.string.disastermode_running), Toast.LENGTH_LONG).show();


	}
	
	private static void benchmarkHelper(String timelineKey, int numReps, boolean prefetch, String outputPrefix) {
		double totalTime = 0;
		for (int rep = 0; rep < numReps; rep++) {
			long startTime = System.nanoTime();
			Diamond.MappedObjectList<DiamondTweet> tweetList = new Diamond.MappedObjectList<DiamondTweet>(timelineKey,
					new Diamond.DefaultMapObjectFunction(), DiamondTweet.class, prefetch, 0, 9);
			for (int i = 0; i < tweetList.Size(); i++) {
				DiamondTweet tweet = tweetList.Get(i);
				int committed = 0;
				String tweetText= null;
				String screenname = null;
				long createdAt = 0;
				long userId = 0;
				String retweetedBy = null;
				long numMentions = 0;
				while (committed == 0) {
					Diamond.DObject.TransactionBegin();
					tweetText = tweet.text.Value();
					screenname = tweet.screenname.Value();
					createdAt = tweet.createdAt.Value();
					userId = tweet.userid.Value();
					retweetedBy = tweet.retweetedBy.Value();
					numMentions = tweet.numMentions.Value();
					committed = Diamond.DObject.TransactionCommit();
				}
				if (i == 0 && !tweetText.equals("Old James Bond movies are better")) {
					Log.e("BENCHMARK", "Error: sanity check failed: string is " + tweetText);
				}
			}
			long endTime = System.nanoTime();
			double time = ((double)(endTime - startTime))/(1000 * 1000);
			totalTime += time;
			if (outputPrefix != null) {
				Log.i("BENCHMARK", outputPrefix + " timeline read time: " + time);
			}
			
			try { Thread.sleep(1000); } catch(InterruptedException e) {}
		}
		double avgLatency = totalTime / numReps;
		if (outputPrefix != null) {
			Log.i("BENCHMARK", outputPrefix + " timeline average read latency: " + avgLatency + " reps: " + numReps);
		}
	}
	
	public static void doBenchmark(Context c) {
		final int TOTAL_REPS = 200;
		final int WARMUP_REPS = 20;
		//String uid = LoginActivity.getTwitterId(c);
		String uid = "3";
		String timelineKey = "twitter:uid:" + uid + ":timeline";
		
		//JVM warmup
		benchmarkHelper(timelineKey, WARMUP_REPS, false, null);
		
		//Non-prefetching reads
		benchmarkHelper(timelineKey, TOTAL_REPS, false, "Diamond");
		
		//Prefetching reads
		benchmarkHelper(timelineKey, TOTAL_REPS, true, "Prefetch");
		
		//Prefetching and stale reads
		Diamond.DObject.SetGlobalStaleness(true);
		Diamond.DObject.SetGlobalMaxStaleness(200);
		//Diamond.DObject.DebugMultiMapIndividualSet(true);
		benchmarkHelper(timelineKey, TOTAL_REPS, true, "Prefetchstale");
		
		Log.i("BENCHMARK", "Done with Diamond experiment");
	}

	class BenchmarkTask extends AsyncTask<Void, Void, Void> {

		@Override
		protected Void doInBackground(Void... params) {
			ShowTweetListActivity.doBenchmark(getBaseContext());
			return null;
		}
	}
	
	
	/**
	 * Saves the current selection
	 
	@Override
	public void onSaveInstanceState(Bundle savedInstanceState) {

	  savedInstanceState.putInt("currentFilter", currentFilter);
	  positionIndex = timelineListView.getFirstVisiblePosition();
	  View v = timelineListView.getChildAt(0);
	  positionTop = (v == null) ? 0 : v.getTop();
	  savedInstanceState.putInt("positionIndex", positionIndex);
	  savedInstanceState.putInt("positionTop", positionTop);
	  
	  Log.i(TAG, "saving" + positionIndex + " " + positionTop);
	  
	  super.onSaveInstanceState(savedInstanceState);
	}
	
	/**
	 * Loads the current user selection
	
	@Override
	public void onRestoreInstanceState(Bundle savedInstanceState) {
	  super.onRestoreInstanceState(savedInstanceState);
	  
	
	  positionIndex = savedInstanceState.getInt("positionIndex");
	  positionTop = savedInstanceState.getInt("positionTop");
	  
	  Log.i(TAG, "restoring " + positionIndex + " " + positionTop);
	}
	
	*/
	
	
	
	
	
	
	
}
