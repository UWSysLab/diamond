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
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;
import ch.ethz.twimight.R;

/**
 * Show information about Twimight
 * @author thossmann
 *
 */
public class AboutActivity extends Activity{

	public static final String TAG = "AboutActivity";
	Button revokeButton;
	Button updateButton;
	TextView lastUpdate;
	TextView keyOk;
	TextView versionName;
	
	/** 
	 * Called when the activity is first created. 
	 */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.showabout);
		
		versionName = (TextView) findViewById(R.id.showAboutVersion);
		try
		{
		    String appVer = this.getPackageManager().getPackageInfo(this.getPackageName(), 0).versionName;
		    versionName.setText(appVer);
		}
		catch (NameNotFoundException e)
		{
		    Log.v(TAG, e.getMessage());
		}
	}
	
	/**
	 * on Resume
	 */
	@Override
	public void onResume(){
		super.onResume();

	}

	/**
	 * Called at the end of the Activity lifecycle
	 */
	@Override
	public void onDestroy(){
		super.onDestroy();
		
		TwimightBaseActivity.unbindDrawables(findViewById(R.id.showAboutRoot));


	}
}
