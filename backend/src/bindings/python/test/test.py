from libpydiamond import *
import unittest
import sys
    
class TestDiamond(unittest.TestCase):

    def test_DString(self):
        s1 = DString()
        s2 = DString()

        DString.Map(s1, "a")
        DString.Map(s2, "a")

        s1.Set("10")
        
        self.assertEqual(s2.Value(), "10")

    def test_DCounter(self):
        c1 = DCounter()
        c2 = DCounter()

        DCounter.Map(c1, "b")
        DCounter.Map(c2, "b")

        c1.Set(12)

        print c2.Value()

        self.assertEqual(c2.Value(), 12)

if __name__ == '__main__':
    unittest.main()
                                                                                



