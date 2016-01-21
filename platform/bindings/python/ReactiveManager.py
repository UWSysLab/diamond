import threading
import time
import sys
sys.path.append("../../build/bindings/python")
from libpydiamond import *

txSet = set()

def start():
    thread = threading.Thread(target=run, args=())
    thread.daemon = True
    thread.start()

def run():
    while True:
        time.sleep(1)
        for func in txSet:
            DObject.TransactionBegin()
            func()
            DObject.TransactionCommit()

def add(func):
    txSet.add(func)

def remove(func):
    txSet.remove(func)

start()

#class TestObj:
#    def testFunc(self):
#        print "Hello world 2!"
#obj = TestObj()
#add(obj.testFunc)
#time.sleep(10)
