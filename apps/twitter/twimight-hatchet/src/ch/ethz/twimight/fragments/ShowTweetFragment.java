/*******************************************************************************
 * Copyright (c) 2011 ETH Zurich.
 * All rights reserved. activity program and the accompanying materials
 * are made available under the terms of the GNU Public License v2.0
 * which accompanies activity distribution, and is available at
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * 
 * Contributors:
 *     Paolo Carta - Implementation
 *     Theus Hossmann - Implementation
 *     Dominik Schatzmann - Message specification
 ******************************************************************************/
package ch.ethz.twimight.fragments;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Fragment;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.ContentObserver;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.text.Html;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import ch.ethz.twimight.R;
import ch.ethz.twimight.activities.LoginActivity;
import ch.ethz.twimight.activities.NewTweetActivity;
import ch.ethz.twimight.activities.ShowUserActivity;
import ch.ethz.twimight.activities.TwimightBaseActivity;
import ch.ethz.twimight.data.StatisticsDBHelper;
import ch.ethz.twimight.location.LocationHelper;
import ch.ethz.twimight.net.twitter.Tweets;
import ch.ethz.twimight.net.twitter.TwitterService;
import ch.ethz.twimight.net.twitter.TwitterUsers;
import ch.ethz.twimight.util.Constants;
import ch.ethz.twimight.util.SDCardHelper;
import ch.ethz.twimight.util.TweetTagHandler;

/**
 * Display a tweet
 * 
 * @author thossmann
 * @author pcarta
 */
@SuppressLint("ValidFragment")
public class ShowTweetFragment extends Fragment {

	private static final String ARG_KEY_ROWID = "rowId";
	
	Cursor c;

	// Views
	private TextView screenNameView;
	private TextView realNameView;
	private TextView tweetTextView;
	private TextView createdTextView;
	private TextView createdWithView;

	private LinearLayout userInfoView;
	ImageButton retweetButton;
	ImageButton deleteButton;
	ImageButton replyButton;
	ImageButton favoriteButton;
	ImageButton offlineButton;

	Uri uri;
	ContentObserver observer;
	Handler handler;

	private boolean favorited;
	int flags;
	int buffer;
	int userRowId;
	long rowId;
	String text;
	String screenName;

	protected String TAG = "ShowTweetFragment";

	// LOGS
	LocationHelper locHelper;
	Intent intent;
	ConnectivityManager cm;
	StatisticsDBHelper statsDBHelper;

	Activity activity;
	ContentResolver resolver;
	View view;

	// photo
	private String photoPath;

	// SDcard helper
	private SDCardHelper sdCardHelper;

	private String userID = null;

	// Container Activity must implement this interface
	public interface OnTweetDeletedListener {
		public void onDelete();
	}

	OnTweetDeletedListener listener;

	@Override
	public void onAttach(Activity activity) {
		super.onAttach(activity);
		try {
			listener = (OnTweetDeletedListener) activity;
		} catch (ClassCastException e) {
		}
	}

	public static ShowTweetFragment newInstance(long rowId) {
		ShowTweetFragment instance = new ShowTweetFragment();
		Bundle args = new Bundle();
		args.putLong(ARG_KEY_ROWID, rowId);
		instance.setArguments(args);
		return instance;
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
			Bundle savedInstanceState) {

		// TODO Auto-generated method stub
		super.onCreateView(inflater, container, savedInstanceState);
		// Inflate the layout for activity fragment
		view = inflater.inflate(R.layout.showtweet, container, false);
		screenNameView = (TextView) view.findViewById(R.id.showTweetScreenName);
		realNameView = (TextView) view.findViewById(R.id.showTweetRealName);

		tweetTextView = (TextView) view.findViewById(R.id.showTweetText);
		createdTextView = (TextView) view.findViewById(R.id.showTweetCreatedAt);
		createdWithView = (TextView) view
				.findViewById(R.id.showTweetCreatedWith);

		// if we are creating a new instance, get row id from arguments, otherwise from saved instance state
		if (savedInstanceState == null) {
			rowId = getArguments().getLong(ARG_KEY_ROWID);
		} else {
			rowId = savedInstanceState.getLong(ARG_KEY_ROWID);
		}

    	// If we don't know which tweet to show, we stop the activity
    	if(rowId != 0) {
    		
    		queryContentProvider();

    		if(c.getCount() == 0) {    			
    			activity.getFragmentManager().beginTransaction().remove(this).commit();
    		}

    		else {	    			
    			// register content observer to refresh when user was updated	    			
    			handler = new Handler();											
    			
    			userID = String.valueOf(c.getLong(c.getColumnIndex(TwitterUsers.COL_TWITTERUSER_ID)));
    			//locate the directory where the photos are stored
    			photoPath = Tweets.PHOTO_PATH + "/" + userID;
    			
    			setTweetInfo();
    			setUserInfo();			
    			setProfilePicture();	

    			setPhotoAttached();    		
    			// disaster info		
    			if( (buffer & Tweets.BUFFER_DISASTER) != 0 ){

    				if(c.getInt(c.getColumnIndex(Tweets.COL_ISVERIFIED))==0){
    					LinearLayout unverifiedInfo = (LinearLayout) view.findViewById(R.id.showTweetUnverified);
    					unverifiedInfo.setVisibility(LinearLayout.VISIBLE);
    				}
    			}

    			
    			

    			handleTweetFlags();					
    			setupButtons();		

    			// If there are any flags, schedule the Tweet for synch
    			if(c.getInt(c.getColumnIndex(Tweets.COL_FLAGS)) >0){
    				Log.i(TAG,"requesting tweet update to twitter");
    				Intent i = new Intent(TwitterService.SYNCH_ACTION);
    				i.putExtra("synch_request", TwitterService.SYNCH_TWEET);
    				i.putExtra("rowId", new Long(uri.getLastPathSegment()));
    				activity.startService(i);
    			}
    		}		
    	}
    	else 
    		activity.getFragmentManager().beginTransaction().remove(this).commit();
    		

    	return view;
    }

	/** 
	 * Called when the activity is first created. 
	 */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		activity = getActivity();			
		resolver = activity.getContentResolver();

		statsDBHelper = new StatisticsDBHelper(activity.getApplicationContext());
		statsDBHelper.open();

		sdCardHelper = new SDCardHelper();

		cm = (ConnectivityManager) activity.getSystemService(Context.CONNECTIVITY_SERVICE);		
		locHelper = LocationHelper.getInstance(activity);		

	}

	/**
	 *  method to set the photo attached with thi tweet
	 *  
	 */
	
	private void setPhotoAttached() {
		// Profile image
		String[] filePath = {photoPath};
		if(sdCardHelper.checkSDState(filePath)){
			if(!c.isNull(c.getColumnIndex(Tweets.COL_MEDIA))){
				ImageView photoView = (ImageView) view.findViewById(R.id.showPhotoAttached);
				
				String photoFileName =  c.getString(c.getColumnIndex(Tweets.COL_MEDIA));
				Uri photoUri = Uri.fromFile(sdCardHelper.getFileFromSDCard(photoPath, photoFileName));//photoFileParent, photoFilename));
				Bitmap photo = sdCardHelper.decodeBitmapFile(photoUri.getPath());
				photoView.setImageBitmap(photo);
			}
		}

		
	}
	
	
	private void queryContentProvider() {
		// get data from local DB and mark for update
		if (c != null && !c.isClosed())
			c.close();	
		
		uri = Uri.parse("content://" + Tweets.TWEET_AUTHORITY + "/" + Tweets.TWEETS + "/" + rowId);		
		c = resolver.query(uri, null, null, null, null);
		
		if(c != null && c.getCount() > 0) {
			Log.i(TAG,"cursor ok");
			c.moveToFirst();
			buffer = c.getInt(c.getColumnIndex(Tweets.COL_BUFFER));
			flags = c.getInt(c.getColumnIndex(Tweets.COL_FLAGS));
		} else
			Log.i(TAG,"cursor not ok");
			
		
	}


	
	/**
	 *  method to handle tweet's flags
	 *  
	 */
	private void handleTweetFlags() {
		LinearLayout toSendNotification = (LinearLayout) view.findViewById(R.id.showTweetTosend);
		LinearLayout toDeleteNotification = (LinearLayout) view.findViewById(R.id.showTweetTodelete);
		LinearLayout toFavoriteNotification = (LinearLayout) view.findViewById(R.id.showTweetTofavorite);
		LinearLayout toUnfavoriteNotification = (LinearLayout) view.findViewById(R.id.showTweetTounfavorite);
		LinearLayout toRetweetNotification = (LinearLayout) view.findViewById(R.id.showTweetToretweet);					
		if ( toSendNotification != null) {
			if((flags & Tweets.FLAG_TO_INSERT) ==0 ){						
				toSendNotification.setVisibility(LinearLayout.GONE);						
			} else
				toSendNotification.setVisibility(LinearLayout.VISIBLE);	
		} else 
			Log.i(TAG,"toSendNotification");
		
		if (toDeleteNotification != null) {
			if((flags & Tweets.FLAG_TO_DELETE) ==0){						
				toDeleteNotification.setVisibility(LinearLayout.GONE);						

			} else{
				toDeleteNotification.setVisibility(LinearLayout.VISIBLE);
				TextView toDeleteText = (TextView) view.findViewById(R.id.showTweetInfoText2);
				if (toDeleteText != null) {
					toDeleteText.setBackgroundResource(android.R.drawable.list_selector_background);						
					toDeleteText.setOnClickListener(new OnClickListener() {

						@Override
						public void onClick(View v) {
							LinearLayout toDeleteNotification = (LinearLayout) view.findViewById(R.id.showTweetTodelete);
							if (toDeleteNotification != null) {
								
								int num = resolver.update(uri, removeDeleteFlag(flags), null, null);
								toDeleteNotification.setVisibility(LinearLayout.GONE);
								if (num > 0) {
									
									queryContentProvider();
									if (c != null) {													
										flags = c.getInt(c.getColumnIndex(Tweets.COL_FLAGS));
										setupButtons();
									}												
								}
							} else 
								Log.i(TAG,"toDeleteNotification");
															 
						}							
					});
				} else
					Log.i(TAG,"toSendNotification");						
			}
		} else 
			Log.i(TAG,"toDeleteNotification");
		
		if ( toFavoriteNotification != null) {
			if((flags & Tweets.FLAG_TO_FAVORITE) ==0){						
				toFavoriteNotification.setVisibility(LinearLayout.GONE);

			} else
				toFavoriteNotification.setVisibility(LinearLayout.VISIBLE);
		} else 
			Log.i(TAG,"toFavoriteNotification");

		if (toUnfavoriteNotification != null) {
			if((flags & Tweets.FLAG_TO_UNFAVORITE) ==0){						
				toUnfavoriteNotification.setVisibility(LinearLayout.GONE);

			} else
				toUnfavoriteNotification.setVisibility(LinearLayout.VISIBLE);
		} else 
			Log.i(TAG,"toUnFavoriteNotification");

		if (toRetweetNotification != null) {
			if((flags & Tweets.FLAG_TO_RETWEET) ==0){						
				toRetweetNotification.setVisibility(LinearLayout.GONE);

			} else
				toRetweetNotification.setVisibility(LinearLayout.VISIBLE);
		}
	}

	/**
	 *  Buttons
	 */
	private void setupButtons() {		
		
		String userString = Long.toString(c.getLong(c.getColumnIndex(TwitterUsers.COL_TWITTERUSER_ID)));
		String localUserString = LoginActivity.getTwitterId(activity);		

		
		// Retweet Button
		retweetButton = (ImageButton) view.findViewById(R.id.showTweetRetweet);
		// we do not show the retweet button for (1) tweets from the local user, (2) tweets which have been flagged to retweeted and (3) tweets which have been marked as retweeted 
		if(userString.equals(localUserString) || ((flags & Tweets.FLAG_TO_RETWEET) > 0) || (c.getInt(c.getColumnIndex(Tweets.COL_RETWEETED))>0)){
			retweetButton.setVisibility(Button.GONE);
		} else {
			retweetButton.setOnClickListener(new OnClickListener(){

				@Override
				public void onClick(View v) {
					showRetweetDialog();
					retweetButton.setVisibility(Button.GONE);
				}
				
			});
		}
		
		// Delete Button
		deleteButton = (ImageButton) view.findViewById(R.id.showTweetDelete);
		if(userString.equals(localUserString)){			
			
			deleteButton.setVisibility(ImageButton.VISIBLE);			
			if((flags & Tweets.FLAG_TO_DELETE) == 0){
				deleteButton.setOnClickListener(new OnClickListener(){
					@Override
					public void onClick(View v) {
						showDeleteDialog();						
					}
				});				
				
			} else {
				deleteButton.setVisibility(ImageButton.GONE);
			}
		} else {
			deleteButton.setVisibility(ImageButton.GONE);
		}
	
		// Reply button: we show it only if we have a Tweet ID!
		replyButton = (ImageButton) view.findViewById(R.id.showTweetReply);
		if(!c.isNull(c.getColumnIndex(Tweets.COL_TID)) || 
				PreferenceManager.getDefaultSharedPreferences(activity).getBoolean("prefDisasterMode", Constants.DISASTER_DEFAULT_ON)==true){
			replyButton.setOnClickListener(new OnClickListener() {
				@Override
				public void onClick(View v) {
					Intent i = new Intent(activity, NewTweetActivity.class);
					if(!c.isNull(c.getColumnIndex(Tweets.COL_TID)))
						i.putExtra("isReplyTo", c.getLong(c.getColumnIndex(Tweets.COL_TID)));
					else
						i.putExtra("isReplyTo", -1);
					i.putExtra("text", "@"+c.getString(c.getColumnIndex(TwitterUsers.COL_SCREENNAME))+ " ");
					startActivity(i);
				}
			});
		} else {
			replyButton.setVisibility(Button.GONE);
		}
		
		// Favorite button
		favorited = ((c.getInt(c.getColumnIndex(Tweets.COL_BUFFER)) & Tweets.BUFFER_FAVORITES) != 0 ) || ((flags & Tweets.FLAG_TO_FAVORITE)>0);
		favoriteButton = (ImageButton) view.findViewById(R.id.showTweetFavorite);
		if( favorited && !((flags & Tweets.FLAG_TO_UNFAVORITE)>0)){
			favoriteButton.setImageResource(R.drawable.btn_twimight_favorite_on);
		}
		favoriteButton.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				
				if(favorited){
					// unfavorite
					resolver.update(uri, clearFavoriteFlag(flags), null, null);					
					((ImageButton) v).setImageResource(R.drawable.btn_twimight_favorite);
					favorited=false;
					
				} else {
					// favorite
					ContentValues cv = setFavoriteFlag(flags);
					if (cv != null) {
						resolver.update(uri, cv , null, null);
						((ImageButton) v).setImageResource(R.drawable.btn_twimight_favorite_on);
						favorited=true;
					} 
				}
				
			}
			
		});
		
	}


	/**
	 *  method to set the profile picture
	 *  
	 */
	private void setProfilePicture() {
		// Profile image
		if(!c.isNull(c.getColumnIndex(TwitterUsers.COL_PROFILEIMAGE_PATH))){
			
			ImageView picture = (ImageView) view.findViewById(R.id.showTweetProfileImage);			
			int userId = c.getInt(c.getColumnIndex("userRowId"));
			Uri imageUri = Uri.parse("content://" +TwitterUsers.TWITTERUSERS_AUTHORITY + "/" + TwitterUsers.TWITTERUSERS + "/" + userId);
			InputStream is;
			
			try {
				is = resolver.openInputStream(imageUri);
				if (is != null) {						
					Bitmap bm = BitmapFactory.decodeStream(is);
					picture.setImageBitmap(bm);	
				} else
					picture.setImageResource(R.drawable.default_profile);
			} catch (FileNotFoundException e) {
				Log.e(TAG,"error opening input stream",e);
				picture.setImageResource(R.drawable.default_profile);
			}	
		}
		
	}

	/**
	 *  The user info
	 *  
	 */
	private void setUserInfo() {
		
		userRowId = c.getInt(c.getColumnIndex("userRowId")); 
		userInfoView = (LinearLayout) view.findViewById(R.id.showTweetUserInfo);
		
		userInfoView.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				Intent i = new Intent(activity, ShowUserActivity.class);
				i.putExtra("rowId", userRowId);
				i.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP); 
				startActivity(i);
			}
			
		});
		
	}


	/**
	 *  The tweet info
	 *  
	 */
	private void setTweetInfo() {

		screenName = c.getString(c.getColumnIndex(TwitterUsers.COL_SCREENNAME));
		screenNameView.setText("@"+screenName);
		realNameView.setText(c.getString(c.getColumnIndex(TwitterUsers.COL_NAME)));
		text = c.getString(c.getColumnIndex(Tweets.COL_TEXT));

		SpannableString str = new SpannableString(Html.fromHtml(text, null, new TweetTagHandler(activity)));

		Log.i(TAG,"setting tweet text: " + str.toString());
		tweetTextView.setText(str);
		tweetTextView.setMovementMethod(LinkMovementMethod.getInstance());

		createdTextView.setText(				
				DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT).format(
						new Date(c.getLong(c.getColumnIndex(Tweets.COL_CREATED)))
						).toString()
						);
		
		if(c.getString(c.getColumnIndex(Tweets.COL_SOURCE))!=null){
			createdWithView.setText(Html.fromHtml(c.getString(c.getColumnIndex(Tweets.COL_SOURCE))));
		} else {
			createdWithView.setVisibility(TextView.GONE);
		}


		String retweeted_by = c.getString(c.getColumnIndex(Tweets.COL_RETWEETED_BY));
		TextView textRetweeted_by = (TextView) view.findViewById(R.id.showTweetRetweeted_by);
		if (retweeted_by != null) {
			textRetweeted_by.append(retweeted_by);		
			textRetweeted_by.setVisibility(View.VISIBLE);					
		}					

	}

	/**
	 * On resume
	 */
	@Override
	public void onResume(){
		super.onResume();
		locHelper.registerLocationListener();
		observer = new TweetContentObserver(handler);
		c.registerContentObserver(observer);

	}


	/**
	 * On Pause
	 */
	@Override
	public void onPause(){
		
		super.onPause();
		if (locHelper != null) 			
			locHelper.unRegisterLocationListener();    			
		if(c!=null){
			if(observer != null) 
				try {
					c.unregisterContentObserver(observer);
				} catch (IllegalStateException ex) {
					//Log.e(TAG,"error unregistering observer",ex);
				}
		}

	}

	/**
	 * Called at the end of the Activity lifecycle
	 */
	@Override
	public void onDestroy(){		
		super.onDestroy();	

		if (userInfoView != null)
			userInfoView.setOnClickListener(null);
		if (retweetButton != null)
			retweetButton.setOnClickListener(null);
		if (deleteButton != null)
			deleteButton.setOnClickListener(null);
		if (replyButton != null)
			replyButton.setOnClickListener(null);
		if (favoriteButton != null)
			favoriteButton.setOnClickListener(null);
		observer = null;
		if (c != null)
			c.close();
		TwimightBaseActivity.unbindDrawables(getActivity().findViewById(R.id.showTweetRoot));

	}

	/**
	 * Asks the user if she wants to delete a tweet.
	 */
	private void showDeleteDialog(){
		AlertDialog.Builder builder = new AlertDialog.Builder(activity);
		builder.setMessage(R.string.delete_tweet)
		       .setCancelable(false)
		       .setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
		           public void onClick(DialogInterface dialog, int id) {		        	   
		        	       	   
		        	   queryContentProvider();	        	   
		        	   
		        	   String delPhotoName = c.getString(c.getColumnIndex(Tweets.COL_MEDIA));
		        	   
		        	   if(delPhotoName != null){
		        		   photoPath = Tweets.PHOTO_PATH + "/" + userID;
		        		   String[] filePath = {photoPath};
		       			   if(sdCardHelper.checkSDState(filePath)){
		       				   File photoFile = sdCardHelper.getFileFromSDCard(photoPath, delPhotoName);//photoFileParent, photoFilename));
				        	   photoFile.delete();
		       			   }
		       			   
		        	   }
		  
		        	   if (!c.isNull((c.getColumnIndex(Tweets.COL_TID))))
		        		   resolver.update(uri, setDeleteFlag(flags), null, null);
		        	   else {
		        		   resolver.delete(uri,null,null );		        	       
		        	   }
		        	   listener.onDelete();
		           }
		       })
		       .setNegativeButton(R.string.no, new DialogInterface.OnClickListener() {
		           public void onClick(DialogInterface dialog, int id) {
		                dialog.cancel();
		           }
		       });
		AlertDialog alert = builder.create();
		alert.show();
	}
	
	/**
	 * Asks the user how to retweet a tweet (old or new style)
	 */
	private void showRetweetDialog(){
		AlertDialog.Builder builder = new AlertDialog.Builder(activity);
		builder.setMessage(R.string.modify_tweet)
		       .setCancelable(false)
		       .setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
		           public void onClick(DialogInterface dialog, int id) {
		        	   Intent i = new Intent(activity, NewTweetActivity.class);
		        	   i.putExtra("text", "RT @"+screenName+" " +text);
		        	   i.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
		        	   startActivity(i);
		           }
		       })
		       .setNegativeButton(R.string.no, new DialogInterface.OnClickListener() {
		           public void onClick(DialogInterface dialog, int id) {
		        	   resolver.update(uri, setRetweetFlag(flags), null, null);
		        	   c.requery();
		               dialog.cancel();
		           }
		       });
		AlertDialog alert = builder.create();
		alert.show();
	}
	
	/**
	 * Removes the delete flag and returns the flags in a content value structure 
	 * to send to the content provider 
	 * @param flags
	 * @return
	 */
	private ContentValues removeDeleteFlag(int flags) {
		ContentValues cv = new ContentValues();
		cv.put(Tweets.COL_FLAGS, flags & (~ Tweets.FLAG_TO_DELETE) );
		cv.put(Tweets.COL_BUFFER, buffer);
		return cv;
	}
	
	/**
	 * Adds the delete flag and returns the flags in a content value structure 
	 * to send to the content provider 
	 * @param flags
	 * @return
	 */
	private ContentValues setDeleteFlag(int flags) {
		ContentValues cv = new ContentValues();
		cv.put(Tweets.COL_FLAGS, flags | Tweets.FLAG_TO_DELETE);
		cv.put(Tweets.COL_BUFFER, buffer);
		return cv;
	}
	
	/**
	 * Adds the to retweet flag and returns the flags in a content value structure 
	 * to send to the content provider 
	 * @param flags
	 * @return
	 */
	private ContentValues setRetweetFlag(int flags) {
		ContentValues cv = new ContentValues();
		cv.put(Tweets.COL_FLAGS, flags | Tweets.FLAG_TO_RETWEET);
		
		if (PreferenceManager.getDefaultSharedPreferences(activity).getBoolean("prefDisasterMode", Constants.DISASTER_DEFAULT_ON)==true) {
			
			cv.put(Tweets.COL_BUFFER, buffer | Tweets.BUFFER_DISASTER);
		} else
			cv.put(Tweets.COL_BUFFER, buffer);
		return cv;
	}
	
	/**
	 * Adds the favorite flag and returns the flags in a content value structure 
	 * to send to the content provider 
	 * @param flags
	 * @return
	 */
	private ContentValues setFavoriteFlag(int flags) {
		ContentValues cv = new ContentValues();
		
		queryContentProvider();

		try {
			// set favorite flag und clear unfavorite flag
			if ((c.getInt(c.getColumnIndex(Tweets.COL_BUFFER)) & Tweets.BUFFER_FAVORITES) != 0 )
				cv.put(Tweets.COL_FLAGS, (flags  & ~Tweets.FLAG_TO_UNFAVORITE));
			else			
				cv.put(Tweets.COL_FLAGS, (flags | Tweets.FLAG_TO_FAVORITE) & (~Tweets.FLAG_TO_UNFAVORITE));
			// put in favorites bufer
			cv.put(Tweets.COL_BUFFER, buffer | Tweets.BUFFER_FAVORITES);
			return cv;
		} catch (Exception ex){
			Log.e(TAG,"error: ",ex);
			return null;
		}
		
		
		
	}
	
	/**
	 * Clears the favorite flag and returns the flags in a content value structure 
	 * to send to the content provider 
	 * @param flags
	 * @return
	 */
	private ContentValues clearFavoriteFlag(int flags) {
		ContentValues cv = new ContentValues();
		
		// clear favorite flag and set unfavorite flag
		if ((c.getInt(c.getColumnIndex(Tweets.COL_BUFFER)) & Tweets.BUFFER_FAVORITES) != 0 )
			cv.put(Tweets.COL_FLAGS, (flags & (~Tweets.FLAG_TO_FAVORITE)) | Tweets.FLAG_TO_UNFAVORITE);	
		else
			cv.put(Tweets.COL_FLAGS, (flags & (~Tweets.FLAG_TO_FAVORITE)));	
		
		if ( !c.isNull(c.getColumnIndex(Tweets.COL_TID)) ) {
			cv.put(Tweets.COL_BUFFER, buffer);
		}
		else {
			cv.put(Tweets.COL_BUFFER, buffer & (~Tweets.BUFFER_FAVORITES));
		}
		return cv;
	}
	
	/**
	 * Calls methods if tweet data has been updated
	 * @author pcarta
	 *
	 */
	class TweetContentObserver extends ContentObserver {
		public TweetContentObserver(Handler h) {
			super(h);
		}

		@Override
		public boolean deliverSelfNotifications() {
			
			return true;
		}

		@Override
		public void onChange(boolean selfChange) {
			
			super.onChange(selfChange);

			/* close the old cursor
			if(c!=null) {				
				c.close();
			}*/
			
			// and get a new one
			uri = Uri.parse("content://" + Tweets.TWEET_AUTHORITY + "/" + Tweets.TWEETS + "/" + rowId);		
			c = resolver.query(uri, null, null, null, null);		
			if(c.getCount() == 1) {
				
				
				c.moveToFirst();
				if (c.getColumnIndex(Tweets.COL_FLAGS) > -1)
					flags = c.getInt(c.getColumnIndex(Tweets.COL_FLAGS));
				// update the views
				handleTweetFlags();				
			}
			
			

		}
	}
	
	
	
	
}
