package com.example.nl35.diamondparse;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.ReactiveManager;

/**
 * Created by nl35 on 18/05/16.
 */
public class ParseQuery<T extends ParseObject> {
    private static String TAG = "ParseQuery";

    private String className;
    private Map<String, String> keyValuePairs;

    public static ParseQuery<ParseObject> getQuery(String className) {
        return new ParseQuery(className);
    }

    public ParseQuery(String className) {
        this.className = className;
        keyValuePairs = new HashMap<String, String>();
    }

    public void whereEqualTo(String key, String value) {
        keyValuePairs.put(key, value);
    }

    /*
     * TODO: add retry on abort?
     */
    public void findInBackground(final FindCallback<T> cb) {
        final List<T> objects = new ArrayList<T>();
        ReactiveManager.execute_txn(new ReactiveManager.TxnFunction() {
            @Override
            public void func(Object... args) {
                Diamond.DStringSet objectSet = new Diamond.DStringSet();
                Diamond.DObject.Map(objectSet, getDiamondObjectSetKey());

                for (String objectId : objectSet.Members()) {
                    Diamond.DStringSet keySet = new Diamond.DStringSet();
                    Diamond.DObject.Map(keySet, getDiamondKeySetKey(objectId));

                    Map<String, String> objKeyValuePairs = new HashMap<String, String>();
                    for (String key : keySet.Members()) {
                        Diamond.DString dstring = new Diamond.DString();
                        Diamond.DObject.Map(dstring, getDiamondKey(objectId, key));
                        objKeyValuePairs.put(key, dstring.Value());
                        Log.d(TAG, "Just read " + key + ":" + dstring.Value() + " for object " + objectId);
                    }

                    boolean matchesQuery = true;
                    for (String queryKey : keyValuePairs.keySet()) {
                        if (!keyValuePairs.get(queryKey).equals(objKeyValuePairs.get(queryKey))) {
                            matchesQuery = false;
                        }
                    }

                    if (matchesQuery) {
                        objects.add((T)(new ParseObject(className, objKeyValuePairs)));
                    }
                }
            }
        }, new ReactiveManager.TxnCallback() {
            @Override
            public void callback(final int committed) {
                new Handler(Looper.getMainLooper()).post(new Runnable() {
                    public void run() {
                        if (committed == 1) {
                            cb.done(objects, null);
                        } else {
                            cb.done(objects, new ParseException("Commit failed"));
                        }
                    }
                });
            }
        });
    }

    private String getDiamondObjectSetKey() {
        return "objects:" + className;
    }

    private String getDiamondKeySetKey(String objectId) {
        return className + ":" + objectId + ":keys";
    }

    private String getDiamondKey(String objectId, String key) {
        return className + ":" + objectId + ":" + key;
    }
}
