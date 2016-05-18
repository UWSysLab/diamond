package com.example.nl35.diamondparse;

/**
 * Created by nl35 on 18/05/16.
 */
public class ParseQuery<T extends ParseObject> {
    public static ParseQuery<ParseObject> getQuery(String className) {
        return new ParseQuery(className);
    }

    public ParseQuery(String className) {

    }

    public void whereEqualTo(String key, String value) {

    }

    public void findInBackground(FindCallback<T> callback) {

    }
}
