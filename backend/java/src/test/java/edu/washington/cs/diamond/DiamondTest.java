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

       System.out.println(s1.Value());
       System.out.println(s2.Value());
       
       s2.Set("13");
       
       assert(s2.Value().equals("13"));
       assert(s1.Value().equals("13"));
       
    }
}
