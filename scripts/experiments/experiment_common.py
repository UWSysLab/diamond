import subprocess

SRC_HOST = "moranis.cs.washington.edu"
DATA_REDIS_PORT = 6379
REDIS_DIR = "redis-3.0.7/src"
WORKING_DIR = "/scratch/nl35"

CLIENTS_FILE = "clients.txt"

logFile = ""
clients = []

def getClientMachines():
    global clients
    if len(clients) == 0:
        clientsFile = open(CLIENTS_FILE, "r")
        for line in clientsFile:
            clients.append(line.rstrip())
    return clients

def setLog(log):
    global logFile
    logFile = log

def logPrint(msg):
    subprocess.call("echo %s | tee -a %s" % (msg, logFile), shell=True)

def init():
    subprocess.call("rm -f %s" % logFile, shell=True)

def startDiamond(configPrefix, keys = None, numKeys = 0, batchSize = 1):
    keyArgs = ""
    if keys != None:
        keyArgs = "--keys ../%s --numkeys %d" % (keys, numKeys)
    startDiamondCmd = "ssh -t %s 'cd diamond-src/scripts; ./manage-servers.py start ../%s %s --batch %d' >> %s 2>&1" \
            % (SRC_HOST, configPrefix, keyArgs, batchSize, logFile)
    subprocess.call(startDiamondCmd, shell=True)

def killDiamond(configPrefix):
    killDiamondCmd = "ssh %s 'cd diamond-src/scripts; ./manage-servers.py kill ../%s' >> %s 2>&1" \
            % (SRC_HOST, configPrefix, logFile)
    subprocess.call(killDiamondCmd, shell=True)

def startDataRedis():
    startRedisCmd = "ssh -f %s 'nohup %s/redis-server --port %d &' >> %s 2>&1" % (SRC_HOST, REDIS_DIR, DATA_REDIS_PORT, logFile)
    subprocess.call(startRedisCmd, shell=True)

def killDataRedis():
    killRedisCmd = "ssh %s 'pkill -f %d'" % (SRC_HOST, DATA_REDIS_PORT)
    subprocess.call(killRedisCmd, shell=True)

def clearDataRedis():
    clearRedisCmd = "ssh %s '%s/redis-cli -p %d flushdb' >> %s 2>&1" % (SRC_HOST, REDIS_DIR, DATA_REDIS_PORT, logFile)
    subprocess.call(clearRedisCmd, shell=True)

def copyFromSrcHostToClients(filename):
    for machine in getClientMachines():
        subprocess.call("ssh %s 'rsync %s:diamond-src/%s %s'" % (machine, SRC_HOST, filename, WORKING_DIR), shell=True)

def runOnClientMachines(command, numMachines):
    machines = getClientMachines()
    processes = []
    for i in range(0, numMachines):
        p = subprocess.Popen("ssh %s 'cd %s; %s' >> %s 2>&1" % (machines[i], WORKING_DIR, command, logFile), shell=True)
        processes.append(p)
    for p in processes:
        p.wait()

def getSrcHost():
    return SRC_HOST

def getClientCountFromDataRedis():
    clients = int(subprocess.check_output("ssh %s '%s/redis-cli -p %d get clients'" % (SRC_HOST, REDIS_DIR, DATA_REDIS_PORT), shell=True))
    return clients
