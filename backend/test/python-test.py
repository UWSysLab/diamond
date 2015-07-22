from libdiamond import *

s1 = DString()
s2 = DString()

DString.Map(s1, "a")
DString.Map(s2, "a")

s1.Set("10")

print s2.Value()

c1 = DCounter()
c2 = DCounter()

DCounter.Map(c1, "b")
DCounter.Map(c2, "b")

c1.Set(12)

print c2.Value()


