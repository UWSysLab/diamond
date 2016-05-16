import subprocess

logFile = ""

def readClients(clientsFileName):
    clients = []
    clientsFile = open(clientsFileName, "r")
    for line in clientsFile:
        clients.append(line.rstrip())
    return clients

def setLog(log):
    global logFile
    logFile = log

def logPrint(msg):
    subprocess.call("echo %s | tee -a %s" % (msg, logFile), shell=True)
