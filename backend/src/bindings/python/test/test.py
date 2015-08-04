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

    def test_DSet(self):
        set1 = DSet()
        set2 = DSet()

        DSet.Map(set1, "c")
        DSet.Map(set2, "c")

        set1.Clear()

        set1.Add(4)
        set1.Add(5)
        set1.Add(5)
        set1.Add(6)

        self.assertTrue(set2.InSet(4))
        self.assertEquals(set(set2.Members()), set((4, 5, 6)))

        set1.Remove(5)
        self.assertFalse(set2.InSet(5))
        self.assertEquals(set(set2.Members()), set((4, 6)))

        set1.Clear()
        self.assertEquals(len(set2.Members()), 0)

    def test_DList(self):
        list1 = DList()
        list2 = DList()

        DList.Map(list1, "d")
        DList.Map(list2, "d")

        list1.Clear()

        list1.Append(1)
        list1.Append(2)
        list1.Append(3)
        list1.Append(2)

        self.assertEquals(list2.Index(2), 1)
        self.assertEquals(list2.Index(4), -1)
        self.assertEquals(list2.Value(2), 3)

        list1.Remove(2)
        self.assertEquals(list2.Index(2), 2)
        self.assertEquals(list2.Value(1), 3)

        list1.Clear()
        self.assertEquals(len(list2.Members()), 0)

if __name__ == '__main__':
    unittest.main()
                                                                                



