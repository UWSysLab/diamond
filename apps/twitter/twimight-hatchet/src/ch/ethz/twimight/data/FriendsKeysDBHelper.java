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

package ch.ethz.twimight.data;

import java.util.Iterator;
import java.util.List;

import android.content.Context;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.preference.PreferenceManager;

/**
 * Manages the FriendsKeys table in the DB.
 * @author thossmann
 *
 */
public class FriendsKeysDBHelper {

	// Database columns
	public static final String KEY_FRIENDS_KEY_ID = "_id";
	public static final String KEY_FRIENDS_KEY_TWITTER_ID = "twitter_id";
	public static final String KEY_FRIENDS_KEY = "key";
	
	// Shared preferences
	private static final String TDS_LAST_FRIENDS_KEYS_UPDATE = "tds_last_friends_keys_update";

	
	private Context context;
	private DBOpenHelper dbHelper;
	private SQLiteDatabase database;
	
	public FriendsKeysDBHelper(Context context){
		this.context = context;
	}
	
	/**
	 * Opens the DB.
	 * @return
	 * @throws SQLException
	 */
	public FriendsKeysDBHelper open() throws SQLException {
		dbHelper = DBOpenHelper.getInstance(context);
		database = dbHelper.getWritableDatabase();
		return this;
	}
	
	/**
	 * Get's the time (seconds since 1970) of the last update
	 * @return
	 */
	public long getLastUpdate(){
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		return prefs.getInt(TDS_LAST_FRIENDS_KEYS_UPDATE, 0);
	}
	
	/**
	 * Stores the number of seconds since 1970 of the last update
	 * @param version
	 */
	public void setLastUpdate(long secs){
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		SharedPreferences.Editor prefEditor = prefs.edit();
		prefEditor.putLong(TDS_LAST_FRIENDS_KEYS_UPDATE, secs);
		prefEditor.commit();

	}
	
	/**
	 * Deletes all entries from DB
	 */
	public void flushKeyList(){
		database.delete(DBOpenHelper.TABLE_FRIENDS_KEYS, null, null);
		setLastUpdate(0);
	}
	
	
	/**
	 * Do we have a public key of a given Twitter user?
	 * @param twitterID
	 * @return
	 */
	public boolean hasKey(long twitterID){
		Cursor c = database.query(DBOpenHelper.TABLE_FRIENDS_KEYS, null, KEY_FRIENDS_KEY_TWITTER_ID + "=" + twitterID, null, null, null, null);
		boolean key = false;
		if(c.getCount() > 0){
			key = true;
		} 
		c.close();
		return key;
	}
	
	/**
	 * gives the public key for the specified twitter user
	 * @param screenName
	 * @return
	 * @author pcarta
	 */
	public String getKey(long twitterID){
		

		Cursor c = database.query(DBOpenHelper.TABLE_FRIENDS_KEYS, null, KEY_FRIENDS_KEY_TWITTER_ID + "=" + twitterID, null, null, null, null);

		if(c.getCount() > 0){
			c.moveToFirst();
			String key = c.getString(c.getColumnIndex("key"));
			c.close();
			return key;
		} 


		return null;
		
	}
	
}
