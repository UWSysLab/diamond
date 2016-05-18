/*
 * Copyright (c) 2015-present, Parse, LLC.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */
package com.parse.starter;

import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import com.parse.FindCallback;
import com.parse.Parse;
import com.parse.ParseAnalytics;
import com.parse.ParseException;
import com.parse.ParseObject;
import com.parse.ParseQuery;
import com.parse.SaveCallback;

import java.util.List;


public class MainActivity extends ActionBarActivity {

    private static String TAG = "ParseTest";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ParseAnalytics.trackAppOpenedInBackground(getIntent());

        Button updateButton = (Button)findViewById(R.id.updatebutton);
        Button pullButton = (Button)findViewById(R.id.pullbutton);

        updateButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                EditText field1EditText = (EditText)findViewById(R.id.editfield1);
                EditText field2EditText = (EditText)findViewById(R.id.editfield2);

                final String field1 = field1EditText.getText().toString();
                final String field2 = field2EditText.getText().toString();

                ParseQuery<ParseObject> query = ParseQuery.getQuery("StringPair");
                query.whereEqualTo("name", "singleton");
                query.findInBackground(new FindCallback<ParseObject>() {
                    @Override
                    public void done(List<ParseObject> objects, ParseException e) {
                        if (e == null) {
                            if (objects.size() == 1) {
                                ParseObject stringPair = objects.get(0);
                                stringPair.put("field1", field1);
                                stringPair.put("field2", field2);
                                stringPair.put("name", "singleton");
                                stringPair.saveInBackground(new SaveCallback() {
                                    @Override
                                    public void done(ParseException e) {
                                        if (e == null) {
                                            Log.d(TAG, "Save completed successfully");
                                        }
                                        else {
                                            Log.d(TAG, "Error on save: " + e.getMessage());
                                        }
                                    }
                                });
                            }
                            else {
                                for (int i = 0; i < objects.size(); i++) {
                                    objects.get(i).deleteInBackground();
                                }
                                ParseObject stringPair = new ParseObject("StringPair");
                                stringPair.put("field1", field1);
                                stringPair.put("field2", field2);
                                stringPair.put("name", "singleton");
                                stringPair.saveInBackground(new SaveCallback() {
                                    @Override
                                    public void done(ParseException e) {
                                        if (e == null) {
                                            Log.d(TAG, "Save completed successfully");
                                        }
                                        else {
                                            Log.d(TAG, "Error on save: " + e.getMessage());
                                        }
                                    }
                                });
                            }
                        }
                        else {
                            Log.d(TAG, "Error on find: " + e.getMessage());
                        }
                    }
                });
            }
        });

        pullButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ParseQuery<ParseObject> query = ParseQuery.getQuery("StringPair");
                query.whereEqualTo("name", "singleton");
                query.findInBackground(new FindCallback<ParseObject>() {
                    @Override
                    public void done(List<ParseObject> objects, ParseException e) {
                        if (e == null) {
                            if (objects.size() == 1) {
                                TextView field1TextView = (TextView)findViewById(R.id.field1);
                                TextView field2TextView = (TextView)findViewById(R.id.field2);
                                ParseObject stringPair = objects.get(0);
                                field1TextView.setText(stringPair.getString("field1"));
                                field2TextView.setText(stringPair.getString("field2"));
                            }
                            else {
                                Log.d(TAG, "Error on find: received " + objects.size()
                                        + " objects in query");
                            }
                        }
                        else {
                            Log.d(TAG, "Error on find: " + e.getMessage());
                        }
                    }
                });
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
