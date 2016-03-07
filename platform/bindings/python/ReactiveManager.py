import threading
import time
import sys
sys.path.append("../../build/bindings/python")
from libpydiamond import *
from twisted.internet import reactor

funcArgMap = dict() # function <-> arguments map
idFuncMap = dict() # reactive_id <-> function map
funcIdMap = dict() # function <-> reactive_id map
nextId = 0

notificationList = []
cv = threading.Condition()

def runInBackground(target, *args, **kwargs):
    threading.Thread(target=target, args=args, kwargs=kwargs).start()
    #reactor.callInThread(target, *args, **kwargs)

def start():
    NotificationInit(callback)
    thread = threading.Thread(target=run, args=())
    thread.daemon = True
    thread.start()

def callback(reactive_id):
    cv.acquire()
    notificationList.append(reactive_id)
    cv.notify()
    cv.release()

def run():
    while True:
        cv.acquire()
        while len(notificationList) == 0:
            cv.wait()
        reactive_id = notificationList.pop(0)
        cv.release()
        func = idFuncMap[reactive_id]
        DObject.BeginReactive(reactive_id)
        func(*funcArgMap[func])
        DObject.TransactionCommit()

def add(func, *args):
    reactive_id = generateId()
    idFuncMap[reactive_id] = func
    funcIdMap[func] = reactive_id
    funcArgMap[func] = args
    DObject.BeginReactive(reactive_id)
    func(*args)
    DObject.TransactionCommit()

def remove(func):
    reactive_id = funcIdMap[func]
    del idFuncMap[reactive_id]
    del funcIdMap[func]
    del funcArgMap[func]

def txn_execute(func, *args):
    runInBackground(txn_execute_helper, func, *args)

def txn_execute_helper(func, *args):
    DObject.TransactionBegin()
    func(*args)
    DObject.TransactionCommit()

def generateId():
    global nextId
    ret = nextId
    nextId = nextId + 1
    return ret
