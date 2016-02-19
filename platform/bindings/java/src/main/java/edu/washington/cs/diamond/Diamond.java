package edu.washington.cs.diamond;

import org.bytedeco.javacpp.*;
import org.bytedeco.javacpp.annotation.*;
import java.lang.IndexOutOfBoundsException;
import java.lang.reflect.Field;
import java.net.URL;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;

@Platform(include={"data_types.h"})

@Namespace("diamond")

public class Diamond {

    static { Loader.load(); }
    public static native void DiamondInit();
    public static native void DiamondInit(@ByRef @StdString String server, int nshards, int closestReplica);

    public static class Callback extends FunctionPointer {
        static { Loader.load(); }
        protected Callback() { allocate(); }
        private native void allocate();
        public @Name("notifyReactive") void call() {
            System.out.println("Upcall from C++ into Java worked!\n");
        }
    }

    public static class MappedObjectList<T> {
        DStringList keyList;
        MapObjectFunction func;
        Class objClass;
        int start;
        int end;
        boolean mapWholeList;
        String key; // key of keyList

        public MappedObjectList(String key, MapObjectFunction f, Class c, boolean prefetch, int rangeStart, int rangeEnd) {
            this(key, f, c, prefetch);
            start = rangeStart;
            end = rangeEnd;
            mapWholeList = false;
        }

        public MappedObjectList(String inKey, MapObjectFunction f, Class c, boolean prefetch) {
            key = inKey;
            keyList = new DStringList();
            DObject.Map(keyList, key);
            func = f;
            objClass = c;
            start = 0;
            end = keyList.Size();
            mapWholeList = true;
            if (prefetch) {
                prefetchItems();
            }
        }

        public void prefetchItems() {
            List<String> members = keyList.Members();
            DiamondUtil.StringVector keyVector = new DiamondUtil.StringVector();
            int svIndex = 0;

            List<String> diamondFieldNames = new ArrayList<String>();
            Field[] fields = objClass.getDeclaredFields();
            for (int i = 0; i < fields.length; i++) {
                Field curField = fields[i];
                if (isDiamondType(curField)) {
                    diamondFieldNames.add(curField.getName());
                }
            }

            keyVector.resize(diamondFieldNames.size() * members.size() + 1);

            keyVector.put(svIndex, key);
            svIndex++;

            for (int i = 0; i < members.size(); i++) {
                String objKey = members.get(i);
                for (int j = 0; j < diamondFieldNames.size(); j++) {
                    String fieldName = diamondFieldNames.get(j);
                    String dObjectKey = func.function(objKey, fieldName);
                    keyVector.put(svIndex, dObjectKey);
                    svIndex++;
                }
            }

            //DObject.PrefetchGlobalAddSet(keyVector);
        }

        public int Size() {
            if (mapWholeList) {
                end = keyList.Size();
            }
            return end - start;
        }

        public T Get(int index) {
            //if (index < 0 || index >= Size()) {
            //    throw new IndexOutOfBoundsException();
            //}
            try {
                int externalIndex = start + index;
                String objKey = keyList.Value(externalIndex);
                T obj = (T)objClass.newInstance();
                MapObject(obj, objKey, func);
                return obj;
            }
            catch (Exception e) {
                throw new RuntimeException(e);
            }
        }
    }

    // UNTESTED
    public static void prefetchMappedObject(Object obj) {
        DiamondUtil.DObjectVector objVector = new DiamondUtil.DObjectVector();
        int ovIndex = 0;
        Field[] fields = obj.getClass().getDeclaredFields();
        objVector.resize(fields.length);
        try {
            for (int i = 0; i < fields.length; i++) {
                Field curField = fields[i];
                if (isDiamondType(curField)) {
                    DObject dobj = (DObject)curField.get(obj);
                    objVector.put(ovIndex, dobj);
                    ovIndex++;
                }
            }
        }
        catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        }

        //DObject.PrefetchGlobalAddSet(objVector);
    }

    public static void MapObject(Object obj, String key) {
        MapObject(obj, key, new DefaultMapObjectFunction());
    }

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
            throw new RuntimeException(e);
        }
        DObject.MultiMap(dobjects, keys);
    }

    //TODO: Implement more elegantly? Right now we need to manually add each new type
    private static boolean isDiamondType(Field f) {
        if (f.getType().equals(Diamond.DString.class)
            || f.getType().equals(Diamond.DLong.class)
            || f.getType().equals(Diamond.DCounter.class)
            || f.getType().equals(Diamond.DSet.class)
            || f.getType().equals(Diamond.DStringSet.class)
            || f.getType().equals(Diamond.DStringList.class)
            || f.getType().equals(Diamond.DList.class)) {
            return true;
        }
        return false;
    }

    public interface MapObjectFunction {
        public String function(String key, String varname);
    }

    public static class DefaultMapObjectFunction implements MapObjectFunction {
        public String function(String key, String varname) {
            return key + ":" + varname;
        }
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

        /*
        public native void Lock();
        public native void ContinueLock();
        public native void Unlock();
        public native void Signal();
        public native void Broadcast();
        public native void Wait();
        */

        public static native void TransactionBegin();
        public static native int TransactionCommit();
        //public static native void TransactionRollback();
        //public static native void TransactionRetry();

        /*
        public static native void TransactionOptionPrefetch(@ByRef DiamondUtil.DObjectVector txPrefetch);

        public static native void PrefetchGlobalAddSet(@ByRef DiamondUtil.StringVector prefetchSet);
        public static native void PrefetchGlobalAddSet(@ByRef DiamondUtil.DObjectVector prefetchSet);

        public static native void SetGlobalStaleness(boolean enable);
        public static native void SetGlobalMaxStaleness(long maxStalenessMs);

        public static native void SetGlobalRedisWait(boolean enable, int replicas, int timeout);

        public static native void DebugMultiMapIndividualSet(boolean enable);
        */
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
        public DSet() { allocate(); }
        private native void allocate();

        public native @Cast("bool") boolean InSet(long val);
        public native void Add(long val);
        public native void Remove(long val);
        public native void Clear();
        public native int Size();


        public native @ByVal DiamondUtil.LongVector MembersAsVector();
        public Set<Long> Members() {
            Set<Long> result = new HashSet<Long>();
            DiamondUtil.LongVector members = MembersAsVector();
            for (int i = 0; i < members.size(); i++) {
                result.add(members.get(i));
            }
            return result;
        }
    }

    public static class DStringSet extends DObject {
        static { Loader.load(); }
        public DStringSet() { allocate(); }
        private native void allocate();

        public native @Cast("bool") boolean InSet(@ByRef @StdString String val);
        public native void Add(@ByRef @StdString String val);
        public native void Remove(@ByRef @StdString String val);
        public native void Clear();

        public native @ByVal DiamondUtil.StringVector MembersAsVector();
        public Set<String> Members() {
            Set<String> result = new HashSet<String>();
            DiamondUtil.StringVector members = MembersAsVector();
            for (int i = 0; i < members.size(); i++) {
                result.add(members.get(i));
            }
            return result;
        }
    }

    //TODO: implement DList
    public static class DList extends DObject {
        static { Loader.load(); }
        public DList() { allocate(); }
        public DList(String key) { allocate(key); }
        private native void allocate();
        private native void allocate(@ByRef @StdString String key);

        public List<Long> Members() {
            List<Long> result = new ArrayList<Long>();
            DiamondUtil.LongVector members = NativeMembers();
            for (int i = 0; i < members.size(); i++) {
                result.add(members.get(i));
            }
            return result;
        }
        @Name("Members")
        public native @ByVal DiamondUtil.LongVector NativeMembers();

        public native long Value(int index);
        public native int Index(long val);
        public native void Append(long val);
        public native void Erase(int index);
        public native void Remove(long val);
        public native void Clear();
        public native int Size();
    }

    public static class DStringList extends DObject {
        static { Loader.load(); }
        public DStringList() { allocate(); }
        public DStringList(String key) { allocate(key); }
        private native void allocate();
        private native void allocate(@ByRef @StdString String key);

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
