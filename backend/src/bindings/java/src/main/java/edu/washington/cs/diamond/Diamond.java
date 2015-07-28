package edu.washington.cs.diamond;

import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;
import java.net.URL;

@Platform(include={"data_types.h"})

@Namespace("diamond")

public class Diamond {
   
   public static class DString extends Pointer {
      static { Loader.load(); }
      public DString() { allocate(); }
      public DString(String s, String key) { allocate(s, key); }
      private native void allocate();
      private native void allocate(@ByRef @StdString String s, @ByRef @StdString String key);

      // to call the getter and setter functions
      public native @StdString String Value();
      public native void Set(@ByRef @StdString String s);
      @Name("operator=")
      public native @ByRef DString Assign(@ByRef @StdString String s);
   }

   public static class DLong extends Pointer {
      static { Loader.load(); }
      public DLong() { allocate(); }
      public DLong(long l, String key) { allocate(l, key); }
      private native void allocate();
      private native void allocate(long l, @StdString String key);

      public native long Value();
      public native void Set(long l);
   }

   public static class DCounter {
      static { Loader.load(); }
      public DCounter() { allocate(); }
      public DCounter(int c, String key) { allocate(c, key); }
      private native void allocate();
      private native void allocate(int c, @ByRef @StdString String key);
      
      public native int Value();
      public native void Set(int val);

      @Name("operator=")
      public native @ByRef DCounter Assign(int val);
      @Name("operator++")
      public native @ByRef DCounter Increment();
      @Name("operator--")
      public native @ByRef DCounter Decrement();

   }

   public static class DSet {
      static { Loader.load(); }
      DSet() { allocate(); }
      private native void allocate();

      public native @Cast("bool") boolean InSet(long val);
      public native void Add(long val);
      public native void Remove(long val);
   }

}
