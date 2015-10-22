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

package ch.ethz.twimight.net.twitter;

import java.io.InputStream;
import java.lang.ref.WeakReference;
import java.util.List;

import android.content.Context;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.text.format.DateUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SimpleCursorAdapter;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.TextView;
import ch.ethz.twimight.R;
import ch.ethz.twimight.activities.LoginActivity;
import ch.ethz.twimight.data.HtmlPagesDbHelper;
import edu.washington.cs.diamond.Diamond;



/** 
 * Cursor adapter for a cursor containing tweets.
 */
public class DiamondTweetAdapter extends BaseAdapter {
	
	static final String[] from = {TwitterUsers.COL_NAME};
	static final int[] to = {R.id.textUser};	
	private static final String TAG = "tweet adapter";
	private HtmlPagesDbHelper htmlDbHelper ;
	private final Bitmap mPlaceHolderBitmap;
	private Diamond.MappedObjectList<DiamondTweet> objects = null;
	private Context context;
	
	private static class ViewHolder {
		TextView usernameTextView;
		TextView textCreatedAt; 
		TextView tweetText ;
		TextView textRetweeted_by ;
		TextView textHtml ;
		TextView splitBar;
		ImageView picture ;
		ImageView toPostInfo;
		ImageView favoriteStar ;
		LinearLayout rowLayout;
		ImageView verifiedImage ;
		long disId = -1;
	
		 
		}

	/** Constructor */
	public DiamondTweetAdapter(Context c, int resource, Diamond.MappedObjectList<DiamondTweet> inObjects) {
		context = c;
		objects = inObjects;
		mPlaceHolderBitmap = BitmapFactory.decodeResource(context.getResources(), R.drawable.default_profile);
		
	}  

	private void createHolder(View view) {
		ViewHolder holder = new ViewHolder();
		setHolderFields(view,holder);
		view.setTag(holder);
	}
	
	private void setHolderFields(View row, ViewHolder holder) {
		
		holder.usernameTextView = (TextView) row.findViewById(R.id.textUser);
		holder.textCreatedAt = (TextView) row.findViewById(R.id.tweetCreatedAt);
		holder.tweetText = (TextView) row.findViewById(R.id.textText);
		holder.textRetweeted_by = (TextView) row.findViewById(R.id.textRetweeted_by);
		holder.textHtml = (TextView) row.findViewById(R.id.linkDownloaded);
		holder.splitBar = (TextView) row.findViewById(R.id.split_bar);
		holder.picture = (ImageView) row.findViewById(R.id.imageView1);
		holder.toPostInfo = (ImageView) row.findViewById(R.id.topost);
		holder.favoriteStar = (ImageView) row.findViewById(R.id.favorite);
		holder.rowLayout = (LinearLayout) row.findViewById(R.id.rowLayout);
		holder. verifiedImage = (ImageView) row.findViewById(R.id.showTweetVerified);
		
		
	}

	/** This is where data is mapped to its view */
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {		
		LayoutInflater inflater = (LayoutInflater) parent.getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		View row = inflater.inflate(R.layout.row, null);		
		createHolder(row);	
		
		ViewHolder holder = (ViewHolder) row.getTag();			
				
		htmlDbHelper = new HtmlPagesDbHelper(context.getApplicationContext());
		htmlDbHelper.open();
		
		DiamondTweet tweet = objects.Get(position);
		
		// if we don't have a real name, we use the screen name
		//if(tweet.name==null){			
			holder.usernameTextView.setText("@" + tweet.getScreenname());
		//}
		
		long createdAt = tweet.getCreatedAt();
		
		holder.textCreatedAt.setText(DateUtils.getRelativeTimeSpanString(createdAt));		
		// here, we don't want the entities to be clickable, so we use the standard tag handler
		
		
		holder.tweetText.setText(tweet.getText());
		//Log.i(TAG, "text: " + cursor.getString(cursor.getColumnIndex(Tweets.COL_TEXT_PLAIN)));
		
		boolean retweeted = false;
		//add the retweet message in case it is a retweet
		String retweeted_by = tweet.getRetweetedBy();
		if (!retweeted_by.equals("")) {
			holder.textRetweeted_by.setText(context.getString(R.string.retweeted_by) + " " + retweeted_by);		
			holder.textRetweeted_by.setVisibility(View.VISIBLE);	
			retweeted = true;
		}
		else {

			holder.textRetweeted_by.setVisibility(View.GONE);		
		}
		
		//long disId = cursor.getLong(cursor.getColumnIndex(Tweets.COL_DISASTERID));	
		
		/*int col_html = cursor.getColumnIndex(Tweets.COL_HTML_PAGES);
		if (col_html > -1) {
			int hasHtml = cursor.getInt(col_html);
			
			holder.splitBar.setVisibility(View.GONE);
			holder.textHtml.setVisibility(View.GONE);

			if(hasHtml == 1){				
				
				
				Cursor curHtml = htmlDbHelper.getTweetUrls(disId);

				if (curHtml != null && curHtml.getCount() > 0) {		

					//if webpages have been downloaded
					int text =0;

					if(retweeted){						
						holder.splitBar.setText("|");						
						holder.splitBar.setVisibility(View.VISIBLE);
					}

					if (!htmlDbHelper.allPagesStored(curHtml)) {							
						
						text = R.string.downloading;
						holder.textHtml.setTextColor(Color.parseColor("#9ea403"));
					} else {
						
						text = R.string.downloaded;
						holder.textHtml.setTextColor(Color.parseColor("#1d8a04"));

					}

					if (text != 0) {
						holder.textHtml.setText(context.getString(text));
						holder.textHtml.setVisibility(View.VISIBLE);
					}	
				}	


			}

		}*/
		// Profile image				
		/*if(!cursor.isNull(cursor.getColumnIndex(TwitterUsers.COL_PROFILEIMAGE_PATH))){			


			if (holder.disId == -1 || holder.disId != disId) {
				holder.picture.setImageResource(R.drawable.default_profile);
				//holder.picture.setImageResource()
				holder.disId = disId;
				int userRowId = cursor.getInt(cursor.getColumnIndex("userRowId"));
				Uri imageUri = Uri.parse("content://" +TwitterUsers.TWITTERUSERS_AUTHORITY + "/" + TwitterUsers.TWITTERUSERS + "/" + userRowId);
				loadBitmap(imageUri, holder.picture, context);	
			}

		} else {*/			
			holder.picture.setImageResource(R.drawable.default_profile);
		//}


		// any transactional flags?		
		//int flags = cursor.getInt(cursor.getColumnIndex(Tweets.COL_FLAGS));
		
		/*boolean toPost = (flags>0);
		if(toPost){
			holder.toPostInfo.setVisibility(ImageView.VISIBLE);
		} else {
			holder.toPostInfo.setVisibility(ImageView.GONE);
		}*/
		
		
		// favorited
		/*int buffer = cursor.getInt(cursor.getColumnIndex(Tweets.COL_BUFFER));
		boolean favorited = (( (buffer & Tweets.BUFFER_FAVORITES) != 0) 
								&& ((flags & Tweets.FLAG_TO_UNFAVORITE)==0))
								|| ((flags & Tweets.FLAG_TO_FAVORITE)>0);*/
		if(tweet.getToFavorite()){
			holder.favoriteStar.setVisibility(ImageView.VISIBLE);
		} else {
			holder.favoriteStar.setVisibility(ImageView.GONE);
		}
		
		
		// disaster info		
		/*if( (buffer & Tweets.BUFFER_DISASTER) != 0 ){
			//Log.i(TAG,"col is disaster > 0");
			holder.rowLayout.setBackgroundResource(R.drawable.disaster_tweet_background);
		
			holder.verifiedImage.setVisibility(ImageView.VISIBLE);
			if(cursor.getInt(cursor.getColumnIndex(Tweets.COL_ISVERIFIED))>0){
				holder.verifiedImage.setImageResource(android.R.drawable.ic_secure);
			} else {
				holder.verifiedImage.setImageResource(android.R.drawable.ic_partial_secure);
			}
		} else*/ if(Long.toString(tweet.getUserId()).equals(LoginActivity.getTwitterId(context))) {
			
			holder.rowLayout.setBackgroundResource(R.drawable.own_tweet_background);
			holder.verifiedImage.setVisibility(ImageView.GONE);
			
		} else if(tweet.getMentions() > 0){
			
			holder.rowLayout.setBackgroundResource(R.drawable.mention_tweet_background);
			holder.verifiedImage.setVisibility(ImageView.GONE);			
		} else {
			holder.rowLayout.setBackgroundResource(R.drawable.normal_tweet_background);
			holder.verifiedImage.setVisibility(ImageView.GONE);
		}
		
		
		return row;
	}	
	
	public void loadBitmap(Uri uri, ImageView imageView, Context context) {
	    if (cancelPotentialWork(uri, imageView)) {
	        final BitmapWorkerTask task = new BitmapWorkerTask(imageView, context, uri);	        
	        final AsyncDrawable asyncDrawable =
	                new AsyncDrawable(context.getResources(), mPlaceHolderBitmap, task);
	        imageView.setImageDrawable(asyncDrawable);	
	        task.execute();
	    }
	}
	
	class BitmapWorkerTask extends AsyncTask<AsyncDrawable, Void, Bitmap> {
	    private final WeakReference<ImageView> imageViewReference;
	    Context context;
	    public Uri uri;
	    AsyncDrawable asyncDrawable;

	    public BitmapWorkerTask(ImageView imageView, Context context, Uri uri) {
	        // Use a WeakReference to ensure the ImageView can be garbage collected
	        imageViewReference = new WeakReference<ImageView>(imageView);
	        this.context = context;
	        this.uri = uri;
	    }

	    // Decode image in background.
	    @Override
	    protected Bitmap doInBackground(AsyncDrawable... params) {
	        Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
	    	//this.asyncDrawable = params[0];
	    	InputStream is;
			try {
				is = context.getContentResolver().openInputStream(uri);
				if (is != null) 						
					return BitmapFactory.decodeStream(is);						
				else
					return null;					
			} catch (Exception e) {
				return null;
				
			}
	    }

	    // Once complete, see if ImageView is still around and set bitmap.
	    @Override
	    protected void onPostExecute(Bitmap bitmap) {
	    	if (isCancelled()) {
	            bitmap = null;
	        }

	    	if (imageViewReference != null ) {
	    		final ImageView imageView = imageViewReference.get();
	    		
	    		final BitmapWorkerTask bitmapWorkerTask =
	                    getBitmapWorkerTask(imageView);

	    		if (bitmap != null) {	    			
	    			if (this == bitmapWorkerTask && imageView != null) {
	    				imageView.setImageBitmap(bitmap);
	    			}
	    		} 		

	    	}
	    }
	}
	
	
	static class AsyncDrawable extends BitmapDrawable {
	    private final WeakReference<BitmapWorkerTask> bitmapWorkerTaskReference;

	    public AsyncDrawable(Resources res, Bitmap bitmap,
	            BitmapWorkerTask bitmapWorkerTask) {
	    	super(res, bitmap);
	    	bitmapWorkerTaskReference =
	    			new WeakReference<BitmapWorkerTask>(bitmapWorkerTask);
	    }

	    public BitmapWorkerTask getBitmapWorkerTask() {
	    	return bitmapWorkerTaskReference.get();
	    }
	}

	private static BitmapWorkerTask getBitmapWorkerTask(ImageView imageView) {
		if (imageView != null) {
			final Drawable drawable = imageView.getDrawable();
			if (drawable instanceof AsyncDrawable) {
				final AsyncDrawable asyncDrawable = (AsyncDrawable) drawable;
				return asyncDrawable.getBitmapWorkerTask();
			}
		}
		return null;
	}
	
	public static boolean cancelPotentialWork(Uri uri, ImageView imageView) {
	    final BitmapWorkerTask bitmapWorkerTask = getBitmapWorkerTask(imageView);

	    if (bitmapWorkerTask != null) {
	        final Uri bitmapUri = bitmapWorkerTask.uri;
	        if (bitmapUri.equals(uri)) {
	            // Cancel previous task
	            bitmapWorkerTask.cancel(true);
	        } else 
	            // The same work is already in progress
	            return false;
	        
	    }
	    // No task associated with the ImageView, or an existing task was cancelled
	    return true;
	}

	@Override
	public int getCount() {
		return this.objects.Size();
	}

	@Override
	public Object getItem(int position) {
		return this.objects.Get(position);
	}

	@Override
	public long getItemId(int position) {
		// TODO Auto-generated method stub
		return 0;
	}

}
