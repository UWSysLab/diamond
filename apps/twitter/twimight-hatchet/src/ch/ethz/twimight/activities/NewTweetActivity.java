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
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.location.Location;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.provider.MediaStore;
import android.text.Editable;
import android.text.Html;
import android.text.TextWatcher;
import android.util.Base64;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import ch.ethz.twimight.R;
import ch.ethz.twimight.data.StatisticsDBHelper;
import ch.ethz.twimight.net.twitter.Tweets;
import ch.ethz.twimight.net.twitter.TwitterService;
import ch.ethz.twimight.util.Constants;

/**
 * The activity to write a new tweet.
 * @author thossmann
 * @author pcarta
 */
public class NewTweetActivity extends Activity{

	private static final String TAG = "TweetActivity";
	
	private EditText text;
	private TextView characters;
	private Button cancelButton;
	private Button sendButton;
	
	private long isReplyTo;
	
	private TextWatcher textWatcher;

	//LOGS
	long timestamp;		
	ConnectivityManager cm;		
	/** 
	 * Called when the activity is first created. 
	 */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.tweet);				

		cm = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);			

		setupBasicButtons();

		characters = (TextView) findViewById(R.id.tweet_characters);
		characters.setText(Integer.toString(Constants.TWEET_LENGTH));

		text = (EditText) findViewById(R.id.tweetText);		

		// Did we get some extras in the intent?
		Intent i = getIntent();
		if(i.hasExtra("text")){
			text.setText(Html.fromHtml("<i>"+i.getStringExtra("text")+"</i>"));
		}
		if(text.getText().length()==0){
			sendButton.setEnabled(false);
		}

		if(text.getText().length()>Constants.TWEET_LENGTH){
			text.setText(text.getText().subSequence(0, Constants.TWEET_LENGTH));
			text.setSelection(text.getText().length());
			characters.setTextColor(Color.RED);
		}

		characters.setText(Integer.toString(Constants.TWEET_LENGTH-text.getText().length()));

		if(i.hasExtra("isReplyTo")){
			isReplyTo = i.getLongExtra("isReplyTo", 0);
		}

		// This makes sure we do not enter more than 140 characters	
		textWatcher = new TextWatcher(){
			public void afterTextChanged(Editable s){
				int nrCharacters = Constants.TWEET_LENGTH-text.getText().length();

				if(nrCharacters < 0){
					text.setText(text.getText().subSequence(0, Constants.TWEET_LENGTH));
					text.setSelection(text.getText().length());
					nrCharacters = Constants.TWEET_LENGTH-text.getText().length();
				}

				if(nrCharacters <= 0){
					characters.setTextColor(Color.RED);
				} else {
					characters.setTextColor(Color.BLACK);
				}

				if(nrCharacters == Constants.TWEET_LENGTH){
					sendButton.setEnabled(false);
				} else {
					sendButton.setEnabled(true);
				}

				characters.setText(Integer.toString(nrCharacters));

			}
			public void  beforeTextChanged(CharSequence s, int start, int count, int after){}
			public void  onTextChanged (CharSequence s, int start, int before,int count) {} 
		};
		text.addTextChangedListener(textWatcher);
		text.setSelection(text.getText().length());	

	}

	private void setupBasicButtons() {

		cancelButton = (Button) findViewById(R.id.tweet_cancel);
		cancelButton.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {				
				finish();		
			}

		});

		sendButton = (Button) findViewById(R.id.tweet_send);
		sendButton.setOnClickListener(new OnClickListener(){

			@Override
			public void onClick(View v) {
				new SendTweetTask().execute();				
			}

		});
	}
	
	/**
	 * On Destroy
	 */
	@Override
	public void onDestroy(){
		super.onDestroy();
		Log.d(TAG, "onDestroy");
		
		cancelButton.setOnClickListener(null);
		cancelButton = null;
		
		sendButton.setOnClickListener(null);
		sendButton = null;
		
		text.removeTextChangedListener(textWatcher);		
		textWatcher = null;
		
		TwimightBaseActivity.unbindDrawables(findViewById(R.id.showNewTweetRoot));
	}
	
	/**	
	 * Checks whether we are in disaster mode and inserts the content values into the content provider.
	 *
	 * @author pcarta
	 *
	 */
private class SendTweetTask extends AsyncTask<Void, Void, Boolean>{
		
		Uri insertUri = null;
		StatisticsDBHelper statsDBHelper;	
		
		@Override
		protected Boolean doInBackground(Void... params) {
			boolean result=false;
			
			//Statistics
			statsDBHelper = new StatisticsDBHelper(getApplicationContext());
			statsDBHelper.open();
			timestamp = System.currentTimeMillis();

			
			// if no connectivity, notify user that the tweet will be send later		
				
				ContentValues cv = createContentValues(); 
				
					// our own tweets go into the timeline buffer
					cv.put(Tweets.COL_BUFFER, Tweets.BUFFER_TIMELINE);
					//we publish on twitter directly only normal tweets
					cv.put(Tweets.COL_FLAGS, Tweets.FLAG_TO_INSERT);
					
					insertUri = getContentResolver().insert(Uri.parse("content://" + Tweets.TWEET_AUTHORITY + "/" + Tweets.TWEETS + "/" + 
																Tweets.TWEETS_TABLE_TIMELINE + "/" + Tweets.TWEETS_SOURCE_NORMAL), cv);
					getContentResolver().notifyChange(Tweets.TABLE_TIMELINE_URI, null);
					//getContentResolver().notifyChange(insertUri, null);
					ConnectivityManager cm = (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);
					if(cm.getActiveNetworkInfo()==null || !cm.getActiveNetworkInfo().isConnected()){
						result=true;
					}

				return result;
			
		}

		@Override
		protected void onPostExecute(Boolean result){
			if (result)
				Toast.makeText(NewTweetActivity.this, getString(R.string.no_connection4), Toast.LENGTH_SHORT).show();
			
			if(insertUri != null){
				// schedule the tweet for uploading to twitter
				Intent i = new Intent(NewTweetActivity.this, TwitterService.class);
				i.putExtra("synch_request", TwitterService.SYNCH_TWEET);
				i.putExtra("rowId", new Long(insertUri.getLastPathSegment()));
				startService(i);
			}
			finish();
		}
	}
	
	
	
	
	
	/**
	 * Prepares the content values of the tweet for insertion into the DB.
	 * @return
	 */
	private ContentValues createContentValues() {
		ContentValues tweetContentValues = new ContentValues();
		
		tweetContentValues.put(Tweets.COL_TEXT_PLAIN, text.getText().toString());
		tweetContentValues.put(Tweets.COL_TWITTERUSER, LoginActivity.getTwitterId(this));
		tweetContentValues.put(Tweets.COL_SCREENNAME, LoginActivity.getTwitterScreenname(this));
		if (isReplyTo > 0) {
			tweetContentValues.put(Tweets.COL_REPLYTO, isReplyTo);
		}	
		// set the current timestamp
		tweetContentValues.put(Tweets.COL_CREATED, System.currentTimeMillis());
		
		return tweetContentValues;
	}
	
}
