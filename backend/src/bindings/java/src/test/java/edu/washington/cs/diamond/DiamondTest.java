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
        
        assert(list2.MembersList().size() == 0);

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
        assert(list2.MembersList().equals(membersList));

        //System.out.println("DStringList test ok!");
    }

    /*
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
    */

}
