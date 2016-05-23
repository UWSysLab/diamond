package com.example.androidchatbaseline;

import android.app.Activity;
import android.app.DialogFragment;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;

public class LoginActivity extends Activity implements LoginDialogFragment.LoginDialogListener {
	
	public static final String PREFS_SCREENNAME = "screenname";
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		DialogFragment dialog = new LoginDialogFragment();
		dialog.setCancelable(false);
		dialog.show(getFragmentManager(), dialog.getClass().getName());
	}
	
	@Override
	public void onDialogPositiveClick(DialogFragment dialog) {
		Intent chatIntent = new Intent(this, ChatActivity.class);
		startActivity(chatIntent);
	}
	
	public static void setScreenname(String screenname, Context context) {
		if (screenname.length() == 0) {
			screenname = "Anonymous User";
		}
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		SharedPreferences.Editor prefEditor = prefs.edit();
		prefEditor.putString(PREFS_SCREENNAME, screenname);
		prefEditor.commit();
	}
	
	public static Boolean hasScreenname(Context context) {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		return prefs.getString(PREFS_SCREENNAME, null) != null;
	}
	
	public static String getScreenname(Context context) {
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		return prefs.getString(PREFS_SCREENNAME, null);
	}
}
