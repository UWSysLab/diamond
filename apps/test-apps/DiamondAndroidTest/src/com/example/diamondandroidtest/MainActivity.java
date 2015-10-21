package com.example.diamondandroidtest;

import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;
import edu.washington.cs.diamond.Diamond;

public class MainActivity extends ActionBarActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		Diamond.DiamondInit("coldwater.cs.washington.edu");
		
		/*Diamond.DString testString1 = new Diamond.DString("String 1: if this string shows up, Diamond syncing is not working", "a");
		Diamond.DString testString2 = new Diamond.DString("String 2: if this string shows up, Diamond syncing is not working", "a");

		testString2.Set(new String("Testing Diamond on Android: syncing appears to work"));
		
		TextView testTextBox = new TextView(getBaseContext());
		testTextBox.setText(testString1.Value());
		testTextBox.setTextColor(Color.BLACK);
		setContentView(testTextBox);*/
		
		Diamond.DString testString = new Diamond.DString();
		Diamond.DObject.Map(testString, "a");
		Log.e("DEBUG", "String: " + testString.Value());
		Log.e("DEBUG", "String: " + testString.Value());
		//Diamond.DString testString = new Diamond.DString("Testing", "a");
		//testString.Set("Testing");
		//TextView testTextBox = new TextView(getBaseContext());
		//testTextBox.setText(testString.Value());
		//testTextBox.setTextColor(Color.BLACK);
		//setContentView(testTextBox);
		
		//setContentView(R.layout.activity_main);
	}
	
	public void testStringList() {
		Diamond.DStringList testList = new Diamond.DStringList("diamondandroidtest:testlist");
		testList.Clear();
		testList.Append("Nexus 7: hello from the Nexus 7");
		testList.Append("Nexus 5: Nexus 5 here");
		testList.Append("Nexus 7: Now to get sync working...");
		testList.Append("Nexus 5: what happened");
		testList.Append("Nexus 7: hello");
		testList.Append("Nexus 5: test");
		testList.Append("Nexus 5: testing locks");
		testList.Append("Nexus 5: testing more");
		testList.Append("Nexus 5: broadcast was causing segfaults");
		testList.Append("Nexus 5: trying broadcast again");
		testList.Append("Test string 2");
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
