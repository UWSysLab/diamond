package edu.washington.cs.diamond;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

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

}
