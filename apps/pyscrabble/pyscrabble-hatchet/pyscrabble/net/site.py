from formless import webform
from nevow import rend, loaders, static, tags as T, flat, inevow
from pyscrabble.net import interfaces
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
        flat.registerFlattener(self.flattenTime, util.Time)
        
        r = manager.ResourceManager()
        ScrabbleSite.child_styles = static.File( r["resources"]["web"].path )
        ScrabbleSite.docFactory = loaders.xmlfile( r["resources"]["web"]["index.html"] )
    
    def flattenTime(self, original, ctx):
        '''
        Flatten a Time object
        
        @param original:
        @param ctx:
        '''
        return str(original)
        
    
    def renderHTTP(self, ctx):
        '''
        Trap all http requests to make sure the user is logged in
        
        @param ctx:
        '''
        
        request = inevow.IRequest(ctx)
        username, password = request.getUser(), request.getPassword()
        
        # First make sure we have users defined
        if (self.factory.hasUsers()):
            # Make sure that the user/password match and the user is an admin
            if not (self.factory.authenticate(username, util.hashPassword(password)) and self.factory.isUserAdmin(username)):
                request.setHeader('WWW-Authenticate', 'Basic realm="PyScrabble Server"')
                request.setResponseCode(http.UNAUTHORIZED)    
                return "Authentication Required."
        
        return rend.Page.renderHTTP(self, ctx)
            
    def locateChild(self, ctx, segments):
        '''
        Locate child segments
        
        @param ctx:
        @param segments:
        '''
        
        if (segments[0] == 'user_admin'):
            return UserAdmin(original=segments[1], ctx=ctx, factory=self.factory), segments[2:]
        if (segments[0] == 'xmlrpc'):
            return ScrabbleRPC(self.factory), segments[1:]
        
        return super(ScrabbleSite, self).locateChild(ctx, segments)
    
    def configurable_bulletinForm(self, ctx):
        f = BulletinForm()
        f.factory = self.factory
        return f
    
    def configurable_newGameForm(self, ctx):
        f = NewGameForm()
        f.factory = self.factory
        return f
    
    def configurable_deleteUserForm(self, ctx):
        f = DeleteUserForm()
        f.factory = self.factory
        return f
    
    def configurable_kickUserForm(self, ctx):
        f = KickUserForm()
        f.factory = self.factory
        return f
    
    def configurable_stopServerForm(self, ctx):
        f = StopServerForm()
        f.factory = self.factory
        return f
    
    def configurable_resetRankForm(self, ctx):
        f = ResetRankForm()
        f.factory = self.factory
        return f
    
    def data_allUsers(self, context, data):
        return [(user.getUsername(), user.isAdmin(), user.getCreatedDate(), user.getLastLoginDate()) for user in self.factory.getUsers()]
    
    def data_allGames(self, context, data):
        return [(game.getName(), game.getNumberOfPlayers(), str(game.getPlayers()), repr(game.getStatus())) for game in self.factory.getGameListing()]
    
    def data_allBulletins(self, ctx, data):
        return [ (message.id, repr(message.data), message.date) for message in self.factory.getServerBulletins()]
    
    def render_totalUsers(self, context, data):
        '''
        Render the total # of users
        
        @param context:
        @param data:
        '''
        return len( self.factory.getUsers() )
    
    def render_loggedInUsers(self, context, data):
        return self.factory.getLoggedInPlayers()
        
    def render_bulletinRow(self, context, (id, data, date)):
        context.fillSlots('message',data)
        context.fillSlots('date', date)
        return context.tag
    
    def render_userRow(self, context, data):
        context.fillSlots('isAdmin', data[1])
        context.fillSlots('createdDate', data[2])
        context.fillSlots('lastLogin', data[3])
        return context.tag
    
    def render_gameRow(self, context, data):
        context.fillSlots('gameId', data[0])
        context.fillSlots('numberOfPlayers', data[1])
        context.fillSlots('players', data[2])
        context.fillSlots('status', data[3])
        return context.tag
    
    def render_userAdmin(self, context, data):
        return T.a(href='user_admin/%s'%data[0])[data[0]]
    
    def render_deleteGameForm(self, context, (gameId, num, players, stat)):
        ret = T.form(action="./freeform_post!newGameForm!deleteGame",
                     enctype="multipart/form-data", method="POST")[
               T.input(type="hidden", name="gameId", value=gameId),
               T.input(type="submit", value="Delete")]
        return ret
    
    def render_deleteBulletinForm(self, context, (bulletinId, data, date)):
        ret = T.form(action="./freeform_post!bulletinForm!deleteBulletin",
                     enctype="multipart/form-data", method="POST")[
               T.input(type="hidden", name="bulletinId", value=bulletinId),
               T.input(type="submit", value="Delete")]
        return ret
    
    def render_deleteUserForm(self, context, (username, isAdmin, createdDate, lastLoginDate)):
        ret = T.form(action="./freeform_post!deleteUserForm!deleteUser",
                     enctype="multipart/form-data", method="POST")[
               T.input(type="hidden", name="username", value=username),
               T.input(type="submit", value="Delete")]
        return ret
    
    def render_kickUserForm(self, context, (username, isAdmin, createdDate, lastLoginDate)):
        ret = T.form(action="./freeform_post!kickUserForm!kickUser",
                     enctype="multipart/form-data", method="POST")[
               T.input(type="hidden", name="username", value=username),
               T.input(type="submit", value="Boot")]
        return ret
    
    def render_stopServerForm(self, context):
        ret = T.form(action="./freeform_post!stopServerForm!stopServer",
                     enctype="multipart/form-data", method="POST")[
               T.input(type="submit", value="Stop Server")]
        return ret
    
    def render_resetRankForm(self, context):
        ret = T.form(action="./freeform_post!resetRankForm!resetRank",
                     enctype="multipart/form-data", method="POST")[
               T.input(type="submit", value="Reset Ranks")]
        return ret
    
    def render_bulletinForm(self, context, data):
        return webform.renderForms('bulletinForm')
    
    
class UserAdmin(rend.Page):
    addSlash = True
    
    def __init__(self, original, ctx, factory):
        self.factory = factory  
        self.user = self.factory.getUser(original)
        rend.Page.__init__(self, original)
        
        r = manager.ResourceManager()
        UserAdmin.docFactory = loaders.xmlfile( r["resources"]["web"]["user_admin.html"] )
    
    def renderHTTP(self, ctx):
        request = inevow.IRequest(ctx)
        username, password = request.getUser(), request.getPassword()
        
        # First make sure we have users defined
        if (self.factory.hasUsers()):
            # Make sure that the user/password match and the user is an admin
            if not (self.factory.authenticate(username, util.hashPassword(password)) and self.factory.isUserAdmin(username)):
                request.setHeader('WWW-Authenticate', 'Basic realm="PyScrabble Server"')
                request.setResponseCode(http.UNAUTHORIZED)    
                return "Authentication Required."
                
        return rend.Page.renderHTTP(self, ctx)
    
    def configurable_modifyUserForm(self, context):
        f = EditUserForm(self.original)
        f.factory = self.factory
        return f
    
    def render_userName(self, context, data):
       return self.original
    
    def render_userForm(self, context, form):
        isAdmin = "No"
        if (self.user.isAdmin()):
            isAdmin = "Yes"
        return webform.renderForms('modifyUserForm', bindingDefaults = {'modifyUser': {'isAdministrator' : isAdmin}})    
            

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

class BulletinForm(object):
    implements(interfaces.IBulletinForm)
    
    def addNewBulletin(self,ctx, message):
        self.factory.addServerBulletin(message)
    
    def deleteBulletin(self,ctx, bulletinId):
        self.factory.deleteServerBulletin(bulletinId)

class EditUserForm(object):
    implements(interfaces.IEditUserForm)
    
    def __init__(self, username):
        self.username = username
    
    def modifyUser(self, ctx, oldPassword, password, isAdministrator):
        
        user = self.factory.getUser(self.username)
        
        if password is not None:
            pw = util.hashPassword(password)
            self.factory.doChangePassword(self.username, pw)
            user.setPassword( pw )
        
        if isAdministrator == "Yes":
            user.setIsAdmin(True)
        else:
            user.setIsAdmin(False)
        
        self.factory.updateUser(user)

class NewGameForm(object):
    implements(interfaces.INewGameForm)
    
    def deleteGame(self, ctx, gameId):
        self.factory.deleteGame(gameId)

class DeleteUserForm(object):
    implements(interfaces.IDeleteUserForm)
    
    def deleteUser(self, ctx, username):
        self.factory.removeUser(username)

class KickUserForm(object):
    implements(interfaces.IKickUserForm)
    
    def kickUser(self, ctx, username):
        self.factory.bootUser(username)

class StopServerForm(object):
    implements(interfaces.IStopServerForm)
    
    def stopServer(self, ctx):
        import logging
        logger = logging.getLogger("pyscrabble.net.site")
        logger.info('Server shutdown via web console')
        self.factory.stopFactory()
        reactor.stop()

class ResetRankForm(object):
    implements(interfaces.IResetRankForm)
    
    def resetRank(self, ctx):
        self.factory.resetRanks()
        