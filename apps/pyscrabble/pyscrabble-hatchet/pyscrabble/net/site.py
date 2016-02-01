from formless import webform
from nevow import rend, loaders, static, tags as T, flat, inevow
from pyscrabble import manager
from pyscrabble import util
from twisted.internet import reactor
from twisted.web import http, xmlrpc
from zope.interface import implements
import os.path

class ScrabbleSite(rend.Page):
    '''
    Main Administration page
    '''
    addSlash = True
    child_styles = None
    
    def __init__(self, factory):
        '''
        Constructor
        
        @param factory: ScrabbleServerFactory
        '''
        rend.Page.__init__(self)
        self.factory = factory
        
        r = manager.ResourceManager()
        ScrabbleSite.docFactory = loaders.xmlfile( r["resources"]["web"]["index.html"] )
        
    
    def renderHTTP(self, ctx):
        '''
        Trap all http requests to make sure the user is logged in
        
        @param ctx:
        '''
        
        return rend.Page.renderHTTP(self, ctx)
            
    def locateChild(self, ctx, segments):
        '''
        Locate child segments
        
        @param ctx:
        @param segments:
        '''
        
        if (segments[0] == 'xmlrpc'):
            return ScrabbleRPC(self.factory), segments[1:]
        
        return super(ScrabbleSite, self).locateChild(ctx, segments)

class ScrabbleRPC(xmlrpc.XMLRPC):
    '''
    XML-RPC Site that gives information about the Scrabble Server
    '''
    
    def __init__(self, factory):
        xmlrpc.XMLRPC.__init__(self)
        self.factory = factory
    
    def xmlrpc_getNumUsers(self):
        '''
        Get the number of users on the server
        
        @return: Number of users on the server
        '''
        
        return len( self.factory.getUsers() )
    
    def xmlrpc_createNewUser(self, username, password):
        '''
        Create a new user
        
        @param username: Username
        @param password: Password
        @return: Tuple (bool,msg) bool=>True if successful, msg=>Detail message
        '''
        return self.factory.addNewUser(username, password, False)