import threading
import time
import sys
sys.path.append("../../build/bindings/python")
from libpydiamond import *
from twisted.internet import reactor

txSet = set()

def runInBackground(target, *args, **kwargs):
    #threading.Thread(target=target, args=args, kwargs=kwargs).start()
    reactor.callInThread(target, *args, **kwargs)

def start():
    #thread = threading.Thread(target=run, args=())
    #thread.daemon = True
    #thread.start()
    reactor.callInThread(run)

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
