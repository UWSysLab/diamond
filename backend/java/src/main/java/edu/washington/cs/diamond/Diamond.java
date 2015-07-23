package edu.washington.cs.diamond;

import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;
import java.net.URL;

@Platform(include={"data_types.h"})
@Namespace("diamond")

public class Diamond {
   
   public static class DString extends Pointer {
      static { URL urls[] = {}; Loader.load(); }
      public DString() { allocate(); }
      public DString(String s, String key) { allocate(s, key); }
      private native void allocate();
      private native void allocate(@StdString String s, @StdString String key);
   
      // to call the getter and setter functions
      public native @StdString String Value();
      public native void Set(@StdString String s);
   }
}
