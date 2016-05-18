package com.example.nl35.diamondparse;

import java.util.List;

/**
 * Created by nl35 on 18/05/16.
 */
public interface FindCallback<T extends ParseObject> {
    public void done(List<T> objects, ParseException e);
}
