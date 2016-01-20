#!/usr/bin/env python
from twisted.internet import reactor
import warnings
warnings.filterwarnings("ignore")

from nevow import appserver
from pyscrabble import constants
from pyscrabble import dist
from pyscrabble import manager
import logging
import os
import ConfigParser

import sys
sys.path.append("../../../platform/build/bindings/python/")
sys.path.append("../../../platform/bindings/python/")
from libpydiamond import *

class ServerConsole(object):
    '''
    Console Server
    '''
    
    def __init__(self):
        '''
        Constructor
        '''
        
        self.configure()
    
    def startConsole(self, installSignals=True):
        '''
        Start the console
        
        @param installSignals: True to install custom signal handlers
        '''
        
        from pyscrabble.net.server import ScrabbleServerFactory
        self.gameFactory = ScrabbleServerFactory()
        
        from pyscrabble.net.site import ScrabbleSite
        self.site = appserver.NevowSite( ScrabbleSite(self.gameFactory) )
        
        reactor.listenTCP(self.g_port, self.gameFactory)
        reactor.listenTCP(self.w_port, self.site)
        
        if installSignals:
            import signal
            signal.signal(signal.SIGTERM, self.handleSignal)
            signal.signal(signal.SIGINT, self.handleSignal)
            if hasattr(signal, "SIGBREAK"):
                signal.signal(signal.SIGBREAK, self.handleSignal)

        logger = logging.getLogger("pyscrabble.server_console")
        logger.info('Server running')
        
        reactor.run(installSignalHandlers=False)
    
    def handleSignal(self, signum, frame):
        '''
        Handle signal
        
        @param signum:
        @param frame:
        '''
        logger = logging.getLogger("pyscrabble.server_console")
        logger.info("Server caught signal: %s. Shutting down..." % str(signum))
        self.stopConsole()
    
    def stopConsole(self):
        '''
        Stop the console
        '''
        logger = logging.getLogger("pyscrabble.server_console")
        logger.info("Console stopped")
        
        self.gameFactory.stopFactory()
        reactor.stop()
    
    def configure(self):
        '''
        Configure the server
        '''
        dist.ensure_config_dir(dist.CONFIG_DIR)
        resources = manager.ResourceManager()
        logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(name)s %(levelname)s %(message)s',
                    filename=resources["config"][constants.LOG_FILE],
                    filemode='w')
        
        
        config = resources["config"][constants.SERVER_CONSOLE_CONFIG]
        
        if not os.path.exists(config):
            raise IOError, "%s must exist in %s" % (constants.SERVER_CONSOLE_CONFIG, resources["config"].path)
            
        parser = ConfigParser.ConfigParser()
        parser.read( config )
        self.w_port = int(parser.get("pyscrabble","web_port"))
        self.g_port = int(parser.get("pyscrabble","game_port"))
        
if __name__ == '__main__':
    DiamondInit("../../../platform/test/local", 1, 0)
    x = ServerConsole()
    x.startConsole()
    
