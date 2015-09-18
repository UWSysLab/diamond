package edu.washington.cs.diamond;

import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;
import java.lang.reflect.Field;
import java.net.URL;
import java.util.List;
import java.util.ArrayList;

@Platform(include={"data_types.h"})

@Namespace("diamond")

public class Diamond {

    public static void MapObject(Object obj, String key, MapObjectFunction func) {
        List<DObject> dobjects = new ArrayList<DObject>();
        List<String> keys = new ArrayList<String>();
        Field[] fields = obj.getClass().getDeclaredFields();
        try {
            for (int i = 0; i < fields.length; i++) {
                Field curField = fields[i];
                if (isDiamondType(curField)) {
                    DObject dobj = (DObject)curField.get(obj);
                    dobjects.add(dobj);
                    keys.add(func.function(key, curField.getName()));
                }
            }
        }
        catch (IllegalAccessException e) {
            System.out.println("MapObject exception: " + e);
            System.exit(1);
        }
        DObject.MultiMap(dobjects, keys);
    }

    //TODO: Implement more elegantly? Right now we need to manually add each new type
    private static boolean isDiamondType(Field f) {
        if (f.getType().equals(Diamond.DString.class)
            || f.getType().equals(Diamond.DLong.class)
            || f.getType().equals(Diamond.DCounter.class)
            || f.getType().equals(Diamond.DSet.class)
            || f.getType().equals(Diamond.DStringList.class)
            || f.getType().equals(Diamond.DList.class)) {
            return true;
        }
        return false;
    }

    public interface MapObjectFunction {
        public String function(String key, String varname);
    }

    public static class DObject extends Pointer {
        static { Loader.load(); }
        protected DObject() { }
        protected DObject(String key) { }

        public static native int Map(@ByRef DObject addr, @ByRef @StdString String key);

        public static int MultiMap(List<DObject> objects, List<String> keys) {
            DiamondUtil.DObjectVector nativeObjects = new DiamondUtil.DObjectVector(objects.size());
            DiamondUtil.StringVector nativeKeys = new DiamondUtil.StringVector(keys.size());

            for (int i = 0; i < objects.size(); i++) {
                nativeObjects.put(i, objects.get(i));
            }
            for (int i = 0; i < keys.size(); i++) {
                nativeKeys.put(i, keys.get(i));
            }
            return NativeMultiMap(nativeObjects, nativeKeys);
        }
        @Name("MultiMap")
        public static native int NativeMultiMap(@ByRef DiamondUtil.DObjectVector objects, @ByRef DiamondUtil.StringVector keys);

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

    //TODO: implement DList
    public static class DList extends DObject {
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
        public List<String> Members() {
            List<String> result = new ArrayList<String>();
            DiamondUtil.StringVector members = NativeMembers();
            for (int i = 0; i < members.size(); i++) {
                result.add(members.get(i));
            }
            return result;
        }
        @Name("Members")
        public native @ByVal DiamondUtil.StringVector NativeMembers();

        public native @StdString String Value(int index);
        public native int Index(@StdString String val);
        public native void Append(@StdString String val);
        public native void Erase(int index);
        public native void Remove(@StdString String val);
        public native void Clear();
        public native int Size();
    }

}
