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
package ch.ethz.twimight.util;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * This is where all global constants for configuration go!
 * @author thossmann
 *
 */
public final class Constants { 
    
	
	/**
	 * Do not instantiate!
	 */
	private Constants() { throw new AssertionError("Constants is uninstantiable"); }

    // Bluetooth scanning configuration
	public static final boolean DISASTER_DEFAULT_ON = false; /** are we in disaster mode by default? */
	public static final boolean OFFLINE_DEFAULT_ON = false;
	
	
	// Twitter
	public static final int CONSUMER_ID = 1;
	public static final int LOGIN_ATTEMPTS = 2; /** How many times do we attempt to log in before giving up? */
	public static final int TIMELINE_ATTEMPTS = 2; /** How many times do we attempt to update the timeline before giving up? */
	public static final int TWEET_LENGTH = 140; /** The max length of a tweet */
	public static final boolean TWEET_DEFAULT_LOCATION = true; /** Are tweets by default geo-tagged or not? */
	public static final boolean TWEET_DEFAULT_RUN_AT_BOOT = true; /** Are the updates started at boot time ? */
	public static long UPDATER_UPDATE_PERIOD = 5 * 60 * 1000L; /** frequency of background updates */
	
	public static final int NR_TWEETS = 50; /** how many tweets to request from twitter in timeline update */
	public static final int NR_FAVORITES = 20; /** how many favorites to request from twitter */
	public static final int NR_DMS = 20; /** how many direct messages to request from twitter */
	public static final int NR_SEARCH_TWEETS = 20; /** how many tweets to request from twitter in search */
	
	public static final long TIMELINE_MIN_SYNCH = 120*1000L; /** Minimum time between two timeline updates */
	public static final long FAVORITES_MIN_SYNCH = 120*1000L; /** Minimum time between two favorite updates */
	public static final long FRIENDS_MIN_SYNCH = 120*60*1000L; /** Minimum time between two updates of the friends list */
	public static final long FOLLOWERS_MIN_SYNCH = 120*60*1000L; /** Minimum time between two updates of the list of followers */
	public static final long USERS_MIN_SYNCH = 24*3600*1000L; /** Minmum time between two updates of a user profile */
	public static final long DMS_MIN_SYNCH = 20*1000L; /** Minimum time between two updates of the direct messages */
	
	public static int TIMELINE_BUFFER_SIZE = 100; /** How many "normal" tweets (not favorites, mentions, etc) to store locally */
	public static final int FAVORITES_BUFFER_SIZE = 20; /** How many favorites to store locally */
	public static final int DTWEET_BUFFER_SIZE = 100; /** How many disaster tweets of other users */
	public static final int MYDTWEET_BUFFER_SIZE = 50; /** How many of our own disaster tweets */
	public static final int MESSAGES_BUFFER_SIZE = 100; /** How many direct messages to and from the local user do we buffer? */
	public static final int DISASTERDM_BUFFER_SIZE = 100; /** How many disaster direct messages of remote users do we carry? */
	public static final int MYDISASTERDM_BUFFER_SIZE = 100; /** How many disaster messages sent by the local user do we carry? */
	public static final int USERTWEETS_BUFFER_SIZE = 100; /** How many tweets to cache for showing user profiles */
	public static final int SEARCHTWEETS_BUFFER_SIZE = 100; /** How many tweets to cache from searching Twitter */
	public static final int SEARCHUSERS_BUFFER_SIZE = 100; /** How many users to cache from searching Twitter */
	
	
	//Other
	public static final long FRIENDS_FOLLOWERS_DELAY = 60*1000L; /** delay after which friends and followers are downloaded */
	public static final String DIS_MODE_USED = "dis_mode_used";
		
	
}
