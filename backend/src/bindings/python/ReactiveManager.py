import threading
import time

txSet = set()

def start():
    thread = threading.Thread(target=run, args=())
    thread.daemon = True
    thread.start()

def run():
    while True:
        time.sleep(1)
        for func in txSet:
            func()

def add(func):
    txSet.add(func)

def testFunc():
    print "Hello, world!"

# Test code
start()
add(testFunc)
time.sleep(10)
