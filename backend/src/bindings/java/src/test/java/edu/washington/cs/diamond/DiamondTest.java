package edu.washington.cs.diamond;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import java.util.List;
import java.util.ArrayList;

/**
 * Unit test for Diamond data structures
 */
public class DiamondTest 
    extends TestCase
{
    /**
     * Create the test case
     *
     * @param testName name of the test case
     */
    public DiamondTest( String testName )
    {
        super( testName );
    }

    /**
     * @return the suite of tests being tested
     */
    public static Test suite()
    {
        return new TestSuite( DiamondTest.class );
    }

    /**
     * Initialize Diamond
     */
    public void setUp() {
        Diamond.DiamondInit();
    }

    /**
     * Diamond string test
     */
    public void testDString()
    {
       Diamond.DString s1 = new Diamond.DString("", "a");
       Diamond.DString s2 = new Diamond.DString("", "a");

       s2.Assign("13");
       
       assert(s2.Value().equals("13"));
       assert(s1.Value().equals("13"));
       //System.out.println("DString test ok!");
    }

   /**
     * Diamond long test
     */
    public void testDLong()
    {
       Diamond.DLong s1 = new Diamond.DLong(0, "b");
       Diamond.DLong s2 = new Diamond.DLong(0, "b");

       s2.Set(13);
       
       assert(s2.Value() == 13);
       assert(s1.Value() == 13);
       //System.out.println("DLong test ok!");
    }

    /**
     * Diamond string list test
     */
    public void testDStringList()
    {
        Diamond.DStringList list1 = new Diamond.DStringList("c");
        Diamond.DStringList list2 = new Diamond.DStringList("c");

        list1.Clear();
        
        assert(list2.Members().size() == 0);

        list1.Append("hello");
        list1.Append("test");
        list1.Append("world");

        assert(list2.Index("world") == 2);
        assert(list2.Value(0).equals("hello"));

        list1.Remove("test");

        assert(list2.Index("world") == 1);

        List<String> membersList = new ArrayList<String>();
        membersList.add("hello");
        membersList.add("world");
        assert(list2.Members().equals(membersList));

        //System.out.println("DStringList test ok!");
    }

    public void testDList()
    {
        Diamond.DList list1 = new Diamond.DList("f");
        Diamond.DList list2 = new Diamond.DList("f");

        list1.Clear();
        
        assert(list2.Members().size() == 0);

        list1.Append(4);
        list1.Append(9);
        list1.Append(16);

        assert(list2.Index(16) == 2);
        assert(list2.Value(0) == 4);

        list1.Remove(9);

        assert(list2.Index(16) == 1);

        List<Long> membersList = new ArrayList<Long>();
        membersList.add((long)4);
        membersList.add((long)16);
        assert(list2.Members().equals(membersList));
    }

    public void testDStringSet() {
        Diamond.DStringSet set1 = new Diamond.DStringSet();
        Diamond.DStringSet set2 = new Diamond.DStringSet();

        Diamond.DObject.Map(set1, "d");
        Diamond.DObject.Map(set2, "d");

        set1.Clear();

        assert(set2.Members().size() == 0);

        set1.Add("Hello");
        set1.Add("World");
        set1.Add("Hello");

        assert(set2.InSet("Hello"));
        assert(set2.InSet("World"));
        assert(set2.Members().size() == 2);

        set1.Remove("Hello");

        assert(!set2.InSet("Hello"));
        assert(set2.InSet("World"));
        assert(set2.Members().size() == 1);
    }

    public void testDSet() {
        Diamond.DSet set1 = new Diamond.DSet();
        Diamond.DSet set2 = new Diamond.DSet();

        Diamond.DObject.Map(set1, "3");
        Diamond.DObject.Map(set2, "3");

        set1.Clear();

        assert(set2.Members().size() == 0);

        set1.Add(1);
        set1.Add(2);
        set1.Add(1);

        assert(set2.InSet(1));
        assert(set2.InSet(2));
        assert(set2.Members().size() == 2);
        assert(set2.Size() == 2);

        set1.Remove(1);

        assert(!set2.InSet(1));
        assert(set2.InSet(2));
        assert(set2.Members().size() == 1);
    }

    public void testMultiMap() {
        Diamond.DString string1 = new Diamond.DString();
        Diamond.DString string2 = new Diamond.DString();
        Diamond.DLong long1 = new Diamond.DLong();
        Diamond.DLong long2 = new Diamond.DLong();
        Diamond.DStringList list1 = new Diamond.DStringList();
        Diamond.DStringList list2 = new Diamond.DStringList();

        List<String> keyList = new ArrayList<String>();
        keyList.add("string");
        keyList.add("long");
        keyList.add("list");

        List<Diamond.DObject> objectList = new ArrayList<Diamond.DObject>();
        objectList.add(string1);
        objectList.add(long1);
        objectList.add(list1);

        Diamond.DObject.MultiMap(objectList, keyList);

        string1.Set("Testing");
        long1.Set(42);
        list1.Clear();
        list1.Append("Hello");
        list1.Append("World");

        assert(string1.Value().equals("Testing"));
        assert(long1.Value() == 42);
        assert(list1.Index("Hello") == 0);
        assert(list1.Value(1).equals("World"));

        List<Diamond.DObject> objectList2 = new ArrayList<Diamond.DObject>();
        objectList2.add(string2);
        objectList2.add(long2);
        objectList2.add(list2);

        Diamond.DObject.MultiMap(objectList2, keyList);

        assert(string2.Value().equals("Testing"));
        assert(long2.Value() == 42);
        assert(list2.Index("Hello") == 0);
        assert(list2.Value(1).equals("World"));
    }

    public static class TestObject {
        Diamond.DString dstr;
        Diamond.DLong dl;

        public TestObject() {
            dstr = new Diamond.DString();
            dl = new Diamond.DLong();
        }
    }

    class TestObjectFunction implements Diamond.MapObjectFunction {
        public String function(String key, String varname) {
            return key + ":" + varname;
        }
    }

    public void testMapObject() {
        TestObject testObj1 = new TestObject();
        Diamond.MapObject(testObj1, "javatest:testobj", new TestObjectFunction());

        testObj1.dstr.Set("map object test");
        testObj1.dl.Set(15);

        assert(testObj1.dstr.Value().equals("map object test"));
        assert(testObj1.dl.Value() == 15);

        TestObject testObj2 = new TestObject();
        Diamond.MapObject(testObj2, "javatest:testobj", new TestObjectFunction());

        assert(testObj2.dstr.Value().equals("map object test"));
        assert(testObj2.dl.Value() == 15);
    }

    public void testMapObjectRange() {
        String key = "javatest:objectlist";
        Diamond.DStringList keyList = new Diamond.DStringList();
        Diamond.DObject.Map(keyList, key);
        keyList.Append("testobjectA");
        keyList.Append("testobjectB");
        keyList.Append("testobjectC");

        // Set objects A and B
        Diamond.MappedObjectList<TestObject> objList1 = 
            new Diamond.MappedObjectList<TestObject>(key, new TestObjectFunction(), TestObject.class, 0, 2);
        TestObject testObjA1 = objList1.Get(0);
        TestObject testObjB1 = objList1.Get(1);

        testObjA1.dstr.Set("testA");
        testObjA1.dl.Set(16);
        testObjB1.dstr.Set("testB");
        testObjB1.dl.Set(17);

        assert(testObjA1.dstr.Value().equals("testA"));
        assert(testObjA1.dl.Value() == 16);
        assert(testObjB1.dstr.Value().equals("testB"));
        assert(testObjB1.dl.Value() == 17);
        assert(objList1.Size() == 2);


        // Read object B and set object C
        Diamond.MappedObjectList<TestObject> objList2 = 
            new Diamond.MappedObjectList<TestObject>(key, new TestObjectFunction(), TestObject.class, 1, 3);
        TestObject testObjB2 = objList2.Get(0);
        TestObject testObjC2 = objList2.Get(1);

        testObjC2.dstr.Set("testC");
        testObjC2.dl.Set(18);

        assert(testObjB2.dstr.Value().equals("testB"));
        assert(testObjB2.dl.Value() == 17);
        assert(testObjC2.dstr.Value().equals("testC"));
        assert(testObjC2.dl.Value() == 18);
        assert(objList2.Size() == 2);

        // Read objects A, B, and C
        Diamond.MappedObjectList<TestObject> objList3 = 
            new Diamond.MappedObjectList<TestObject>(key, new TestObjectFunction(), TestObject.class, 0, 3);
        TestObject testObjA3 = objList3.Get(0);
        TestObject testObjB3 = objList3.Get(1);
        TestObject testObjC3 = objList3.Get(2);

        assert(testObjA3.dstr.Value().equals("testA"));
        assert(testObjA3.dl.Value() == 16);
        assert(testObjB3.dstr.Value().equals("testB"));
        assert(testObjB3.dl.Value() == 17);
        assert(testObjC3.dstr.Value().equals("testC"));
        assert(testObjC3.dl.Value() == 18);
        assert(objList3.Size() == 3);
    }

//    class TransactionTestRunnable implements Runnable {
//        int txAttempts = 0;
//        int committed = 0;
//
//        public void run() {
//            Diamond.DLong a = new Diamond.DLong();
//            Diamond.DLong b = new Diamond.DLong();
//            Diamond.DObject.Map(a, "javatest:transactions:a");
//            Diamond.DObject.Map(b, "javatest:transactions:b");
//
//            while (committed == 0) {
//                assert(txAttempts < 10000);
//                txAttempts++;
//
//                Diamond.DObject.TransactionBegin();
//
//                a.Set(10);
//                assert(a.Value() == 10);
//
//                try {
//                    Thread.sleep(2);
//                }
//                catch(InterruptedException e) {}
//
//                b.Set(11);
//                assert(b.Value() == 11);
//
//                committed = Diamond.DObject.TransactionCommit();
//            }
//        }
//    }
//
//    public void testTransactions() {
//        new Thread(new TransactionTestRunnable()).start();
//        new Thread(new TransactionTestRunnable()).start();
//    }
}
