package com.example.nl35.diamondparse;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;
import java.util.Random;

import edu.washington.cs.diamond.Diamond;
import edu.washington.cs.diamond.ReactiveManager;

/**
 * Created by nl35 on 18/05/16.
 */
public class ParseObject {
    private static String TAG = "ParseObject";

    private String className;
    private Map<String, String> keyValuePairs;

    public ParseObject(String className) {
        this.className = className;
        keyValuePairs = new HashMap<String, String>();
        keyValuePairs.put("objectId", generateRandomId());
    }

    public ParseObject(String className, Map<String, String> keyValuePairs) {
        this.className = className;
        this.keyValuePairs = keyValuePairs;
    }

    public void put(String key, String value) {
        keyValuePairs.put(key, value);
    }

    public String getString(String key) {
        return keyValuePairs.get(key);
    }

    /*
     * TODO: add retry on abort?
     */
    public void saveInBackground(final SaveCallback cb) {
        ReactiveManager.execute_txn(new ReactiveManager.TxnFunction() {
            @Override
            public void func(Object... objects) {
                String objectId = keyValuePairs.get("objectId");

                Diamond.DStringSet objectSet = new Diamond.DStringSet();
                Diamond.DObject.Map(objectSet, getDiamondObjectSetKey());

                objectSet.Add(objectId);

                Diamond.DStringSet keySet = new Diamond.DStringSet();
                Diamond.DObject.Map(keySet, getDiamondKeySetKey());

                for (String key : keyValuePairs.keySet()) {
                    String value = keyValuePairs.get(key);
                    Diamond.DString dstring = new Diamond.DString();
                    Diamond.DObject.Map(dstring, getDiamondKey(key));
                    dstring.Set(value);
                    keySet.Add(key);
                }
            }
        }, new ReactiveManager.TxnCallback() {
            @Override
            public void callback(final int committed) {
                new Handler(Looper.getMainLooper()).post(new Runnable() {
                    public void run() {
                        if (committed == 1) {
                            cb.done(null);
                        } else {
                            cb.done(new ParseException("Commit failed"));
                        }
                    }
                });
            }
        });
    }

    private String getDiamondObjectSetKey() {
        return "objects:" + className;
    }

    private String getDiamondKeySetKey() {
        return className + ":" + keyValuePairs.get("objectId") + ":keys";
    }

    private String getDiamondKey(String key) {
        return className + ":" + keyValuePairs.get("objectId") + ":" + key;
    }

    /*
     * TODO: add retry on abort?
     *
     * NOTE: this method currently only deletes the object's ID from the class's objectList
     * and the object's keys from its keysList. The individual keys retain their values. I think
     * this should work fine, since IDs should be unique, and even if another object happens to
     * be created with the same ID and uses the same keys, the keys will be overwritten when
     * added to the new object. I would be worried about the "dead" keys taking up space in storage,
     * but Diamond has no mechanism for deleting keys right now anyways.
     */
    public void deleteInBackground() {
        ReactiveManager.execute_txn(new ReactiveManager.TxnFunction() {
            @Override
            public void func(Object... objects) {
                String objectId = keyValuePairs.get("objectId");

                Diamond.DStringSet objectSet = new Diamond.DStringSet();
                Diamond.DObject.Map(objectSet, getDiamondObjectSetKey());

                Diamond.DStringSet keySet = new Diamond.DStringSet();
                Diamond.DObject.Map(keySet, getDiamondKeySetKey());

                objectSet.Remove(objectId);
                keySet.Clear();
            }
        }, new ReactiveManager.TxnCallback() {
            @Override
            public void callback(int committed) {
                if (committed == 0) {
                    Log.d(TAG, "deleteInBackground() commit failed");
                }
            }
        });
    }

    public String generateRandomId() {
        Random random = new Random();
        char[] letters = new char[10];
        for (int i = 0; i < letters.length; i++) {
            char letter = '\0';
            int index = random.nextInt(52);
            if (index <= 25) {
                letter = (char)(index + 65);
            }
            else if (index >= 26 && index < 52) {
                letter = (char)(index - 26 + 97);
            }
            letters[i] = letter;
        }
        return new String(letters);
    }
}
