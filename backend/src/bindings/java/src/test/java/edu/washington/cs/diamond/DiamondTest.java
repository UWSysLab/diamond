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

}
