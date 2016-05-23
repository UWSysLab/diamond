package com.example.nl35.diamondparse;

/**
 * Created by nl35 on 18/05/16.
 */
public class ParseException {
    String message;

    public ParseException(String msg) {
        message = msg;
    }
    public String getMessage() {
        return message;
    }
}
