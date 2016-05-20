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

import android.app.Activity;
import android.app.DialogFragment;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.SearchRecentSuggestions;
import android.widget.Button;
import android.widget.LinearLayout;
import ch.ethz.twimight.R;
import ch.ethz.twimight.data.DBOpenHelper;
import ch.ethz.twimight.fragments.LoginDialogFragment;
import ch.ethz.twimight.net.twitter.TwitterAlarm;
import ch.ethz.twimight.net.twitter.TwitterService;
import ch.ethz.twimight.util.Constants;
import ch.ethz.twimight.util.TwimightSuggestionProvider;

/**
 * Logging the user in and out.
 * Different things can happen, depending on whether we have (i) tokens and/or (ii) connectivity:
 * Tokens, Connectivity: Start the Timeline. In the background verify the tokens and report only on error.
 * Tokens, no Connectivity: Start the Timeline. Display Toast about lack of connectivity.
 * No tokens: whether or not we have connectivity, we show the login button.
 * TODO: Dump the state in a file upon logout and read it again when logging in.
 * @author thossmann
 *
 */
public class LoginActivity extends Activity implements LoginDialogFragment.LoginDialogListener {

	private static final String TAG = "LoginActivity"; /** For logging */
	
	// shared preferences
		public static final String TWITTER_ID = "twitter_id"; /** Name of Twitter ID in shared preferences */

		//TODO: changed/added by Niel
		public static final String TWITTER_SCREENNAME = "twitter_screenname"; /** Name of Twitter screenname in shared preferences */
		public static final String TWITTER_USERNAME = "twitter_username"; /** Name of Twitter username in shared preferences */
		public static final String TWITTER_URL = "twitter_url"; /** Name of Twitter username in shared preferences */
		//TODO: end changes
		
		private static final String TWITTER_ACCESS_TOKEN = "twitter_access_token"; /** Name of access token in preference */
		private static final String TWITTER_ACCESS_TOKEN_SECRET = "twitter_access_token_secret"; /** Name of secret in preferences */

		private static final String TWITTER_REQUEST_TOKEN = "twitter_request_token"; /** Name of the request token in preferences */
		private static final String TWITTER_REQUEST_TOKEN_SECRET = "twitter_request_token_secret"; /** Name of the request token secret in preferences */
		
		// twitter urls
		private static final String TWITTER_REQUEST_TOKEN_URL = "https://api.twitter.com/oauth/request_token"; 
		private static final String TWITTER_ACCESS_TOKEN_URL = "https://api.twitter.com/oauth/access_token";
		private static final String TWITTER_AUTHORIZE_URL = "https://api.twitter.com/oauth/authorize";
		private static final Uri CALLBACK_URI = Uri.parse("my-app://bluetest");
		
		public static final String LOGIN_RESULT_ACTION = "twitter_login_result_action";
		public static final String LOGIN_RESULT = "twitter_login_result";
		public static final int LOGIN_SUCCESS = 1;
		public static final int LOGIN_FAILURE = 2;
		
		// views
		Button buttonLogin;
		LinearLayout showLoginLayout;

		private static PendingIntent restartIntent;
		private static LoginActivity instance = null; /** The single instance of this class */
		
		
		
		/** 
		 * Called when the activity is first created. 
		 */
		@Override
		public void onCreate(Bundle savedInstanceState) {
			super.onCreate(savedInstanceState);		
			setContentView(R.layout.login);
			
			
			setRestartIntent(PendingIntent.getActivity(this.getBaseContext(), 0, 
					new Intent(getIntent()), getIntent().getFlags()));
			instance = this;

			if (hasTwitterId(this.getBaseContext())) {
				hackStartTimeline();
			}
			else {
				DialogFragment dialog = new LoginDialogFragment();
				dialog.show(getFragmentManager(), "LoginDialogFragment");
			}
		}
		
		//TODO: added by Niel
		private void hackStartTimeline() {
			Intent i = new Intent(TwitterService.SYNCH_ACTION);
			i.putExtra("synch_request", TwitterService.SYNCH_LOGIN);
			startService(i);
			startTimeline(getApplicationContext());
		}
		
		//TODO: added by Niel
		public void onDialogPositiveClick(DialogFragment dialog) {
			hackStartTimeline();
		}
		
		
		/**
		 * onDestroy
		 */
		@Override
		public void onDestroy(){
			super.onDestroy();	
						
			// null the onclicklistener of the button
			if(buttonLogin != null){
				buttonLogin.setOnClickListener(null);
			}
			TwimightBaseActivity.unbindDrawables(findViewById(R.id.showLoginRoot));
			
		}

		private void startTimeline(Context context) {		
			Intent i = new Intent(context, ShowTweetListActivity.class);
			
			i.putExtra("login", true);		
			startActivity(i);		
			startAlarms(context);
			finish();
		}

		/**
		 * Start all the enabled alarms and services.
		 */
		public static void startAlarms(Context context) {	
			
			//start the twitter update alarm
			if(PreferenceManager.getDefaultSharedPreferences(context).getBoolean(context.getString(R.string.prefRunAtBoot), 
					Constants.TWEET_DEFAULT_RUN_AT_BOOT)==true){
				
				new TwitterAlarm(context,true);
			}
							
		}
		
		/**
		 * Stop all the alarms and services
		 */
		private static void stopServices(Context context) {
						
			context.stopService(new Intent(context, TwitterService.class));	
			
			TwitterAlarm.stopTwitterAlarm(context);
		}
		
		/**
		 * Upon pressing the login button, we first get Request tokens from Twitter.
		 * @param context
		 */
		
		  static class PerformLogoutTask extends AsyncTask<Context,Void,Void> {

			@Override
			protected Void doInBackground(Context... params) {
				// Stop all services and pending alarms
				Context context = params[0];
				stopServices(context);
				
				// Delete persistent Twitter update information
				TwitterService.setFavoritesSinceId(null, context);
				TwitterService.setLastFavoritesUpdate(null, context);
				TwitterService.setTimelineSinceId(null, context);
				TwitterService.setLastTimelineUpdate(0, context);
				TwitterService.setLastFriendsUpdate(null, context);
				TwitterService.setLastFollowerUpdate(null, context);
				TwitterService.setLastDMsInUpdate(null, context);
				TwitterService.setLastDMsOutUpdate(null, context);
				TwitterService.setDMsOutSinceId(null, context);
				TwitterService.setDMsInSinceId(null, context);
				
				// Delete our Twitter ID and screenname
				setTwitterId(null, context);
				setTwitterScreenname(null, context);
				
				// Flush DB
				DBOpenHelper dbHelper = DBOpenHelper.getInstance(context);
				dbHelper.flushDB();
				
				SearchRecentSuggestions suggestions = new SearchRecentSuggestions(context,
		                TwimightSuggestionProvider.AUTHORITY, TwimightSuggestionProvider.MODE);
				suggestions.clearHistory();
				
				// Start login activity
				Intent intent = new Intent(context, LoginActivity.class);
				intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
				context.startActivity(intent);
				return null;
			}
			
			
			
		}
		
		/**
		 * Deleting all the state
		 */
		public static void logout(Context context){
			new PerformLogoutTask().execute(context);
			
			
		}
		
		/**
		 * Stores the local Twitter ID in the shared preferences
		 * @param id
		 * @param context
		 */
		public static void setTwitterId(String id, Context context) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
			SharedPreferences.Editor prefEditor = prefs.edit();
			prefEditor.putString(TWITTER_ID, id);
			prefEditor.commit();
		}
		
		/**
		 * Gets the Twitter ID from shared preferences
		 * @param context
		 * @return
		 */
		public static String getTwitterId(Context context) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
			return prefs.getString(TWITTER_ID, null);
		}
		
		/**
		 * Do we have a Twitter ID in shared preferences?
		 * @param context
		 * @return
		 */
		public static boolean hasTwitterId(Context context) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
			return prefs.getString(TWITTER_ID, null)!=null;
		}
		
		/**
		 * Stores the local Twitter screenname in the shared preferences
		 * @param id
		 * @param context
		 */
		public static void setTwitterScreenname(String screenname, Context context) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
			SharedPreferences.Editor prefEditor = prefs.edit();
			prefEditor.putString(TWITTER_SCREENNAME, screenname);
			prefEditor.commit();
		}
		
		/**
		 * Gets the Twitter screenname from shared preferences
		 * @param context
		 * @return
		 */
		public static String getTwitterScreenname(Context context) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
			return prefs.getString(TWITTER_SCREENNAME, null);
		}
		
		/**
		 * Stores the local Twitter username in the shared preferences
		 * @param username
		 * @param context
		 */
		public static void setTwitterUsername(String username, Context context) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
			SharedPreferences.Editor prefEditor = prefs.edit();
			prefEditor.putString(TWITTER_USERNAME, username);
			prefEditor.commit();
		}
		
		/**
		 * Gets the Twitter username from shared preferences
		 * @param context
		 * @return
		 */
		public static String getTwitterUsername(Context context) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
			return prefs.getString(TWITTER_USERNAME, null);
		}
		
		public static void setTwitterUrl(String username, Context context) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
			SharedPreferences.Editor prefEditor = prefs.edit();
			prefEditor.putString(TWITTER_URL, username);
			prefEditor.commit();
		}
		
		public static String getTwitterUrl(Context context) {
			SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
			return prefs.getString(TWITTER_URL, null);
		}
		
		
		
		/**
		 * returns the one instance of this activity
		 */
		public static LoginActivity getInstance() {
			return instance;
		}
		
		/**
		 * @param restartIntent the restartIntent to set
		 */
		public static void setRestartIntent(PendingIntent restartIntent) {
			LoginActivity.restartIntent = restartIntent;
		}

		/**
		 * @return the restartIntent
		 */
		public static PendingIntent getRestartIntent() {
			return restartIntent;
		}	
}
