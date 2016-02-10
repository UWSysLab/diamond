import threading
import time
import sys
sys.path.append("../../build/bindings/python")
from libpydiamond import *
from twisted.internet import reactor

txDict = dict()

def runInBackground(target, *args, **kwargs):
    threading.Thread(target=target, args=args, kwargs=kwargs).start()
    #reactor.callInThread(target, *args, **kwargs)

def start():
    thread = threading.Thread(target=run, args=())
    thread.daemon = True
    thread.start()
    #reactor.callInThread(run)

def run():
    while True:
        time.sleep(1)
        for func in txDict.keys():
            DObject.TransactionBegin()
            func(*txDict[func])
            DObject.TransactionCommit()

def add(func, *args):
    txDict[func] = args

def remove(func):
    del txDict[func]

def txn_execute(func, *args):
    runInBackground(txn_execute_helper, func, *args)

def txn_execute_helper(func, *args):
    DObject.TransactionBegin()
    func(*args)
    DObject.TransactionCommit()

start()

#class TestObj:
#    def testFunc(self):
#        print "Hello world 2!"
#obj = TestObj()
#add(obj.testFunc)
#time.sleep(10)
