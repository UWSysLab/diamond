package edu.washington.cs.diamond;

import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;
import java.net.URL;
import java.util.List;
import java.util.ArrayList;

@Platform(include={"data_types.h"})

@Namespace("diamond")

public class Diamond {

    public static class DObject extends Pointer {
        static { Loader.load(); }
        protected DObject() { }
        protected DObject(String key) { }

        public native void Lock();
        public native void ContinueLock();
        public native void Unlock();
        public native void Signal();
        public native void Broadcast();
        public native void Wait();
    }
   
   public static class DString extends DObject {
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

   public static class DLong extends DObject {
      static { Loader.load(); }
      public DLong() { allocate(); }
      public DLong(long l, String key) { allocate(l, key); }
      private native void allocate();
      private native void allocate(long l, @StdString String key);

      public native long Value();
      public native void Set(long l);
   }

   public static class DCounter extends DObject {
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

   public static class DSet extends DObject {
      static { Loader.load(); }
      DSet() { allocate(); }
      private native void allocate();

      public native @Cast("bool") boolean InSet(long val);
      public native void Add(long val);
      public native void Remove(long val);
   }

    public static class DStringList extends DObject {
        static { Loader.load(); }
        public DStringList() { allocate(); }
        public DStringList(String key) { allocate(key); }
        private native void allocate();
        private native void allocate(@ByRef @StdString String key);

        //TODO: Figure out if there's a way to name the method that binds
        //to the native Members() method something else, so that this method
        //can be named Members()
        public List<String> MembersList() {
            List<String> result = new ArrayList<String>();
            DiamondUtil.StringVector members = Members();
            for (int i = 0; i < members.size(); i++) {
                result.add(members.get(i).getString());
            }
            return result;
        }
        public native @ByVal DiamondUtil.StringVector Members();

        public native @StdString String Value(int index);
        public native int Index(@StdString String val);
        public native void Append(@StdString String val);
        public native void Erase(int index);
        public native void Remove(@StdString String val);
        public native void Clear();
        public native int Size();
    }

}
