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

    class TestObject {
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
        TestObject testObjA1 = new TestObject();
        TestObject testObjB1 = new TestObject();
        List<Object> objList1 = new ArrayList<Object>();
        objList1.add(testObjA1);
        objList1.add(testObjB1);

        List<String> keysList = new ArrayList<String>();
        keysList.add("javatest:testobjectrangeA");
        keysList.add("javatest:testobjectrangeB");
        Diamond.MapObjectRange(objList1, keysList, new TestObjectFunction());

        testObjA1.dstr.Set("testA");
        testObjA1.dl.Set(16);
        testObjA1.dstr.Set("testB");
        testObjA1.dl.Set(17);

        assert(testObjA1.dstr.Value().equals("testA"));
        assert(testObjA1.dl.Value() == 16);
        assert(testObjB1.dstr.Value().equals("testB"));
        assert(testObjB1.dl.Value() == 17);

        TestObject testObjA2 = new TestObject();
        TestObject testObjB2 = new TestObject();
        List<Object> objList2 = new ArrayList<Object>();
        objList2.add(testObjA2);
        objList2.add(testObjB2);

        Diamond.MapObjectRange(objList1, keysList, new TestObjectFunction());
        assert(testObjA2.dstr.Value().equals("testA"));
        assert(testObjA2.dl.Value() == 16);
        assert(testObjB2.dstr.Value().equals("testB"));
        assert(testObjB2.dl.Value() == 17);
    }

}
