from twisted.internet import protocol, reactor, error
from twisted.protocols.basic import NetstringReceiver
from pyscrabble.command import helper
from pyscrabble.game.player import Player, User,PlayerInfo
from pyscrabble.game.game import ScrabbleGame, ScrabbleGameInfo
from pyscrabble.lookup import *
from pyscrabble.game import rank
from pyscrabble import audit
from pyscrabble import constants
from pyscrabble import db
from pyscrabble import exceptions
from pyscrabble import manager
from pyscrabble import serialize
from pyscrabble import util
import codecs
import datetime
import logging
import math
import os
import time
import zlib
try:
    set
except NameError:
    from sets import Set as set

logger = logging.getLogger("pyscrabble.net.server")

def upper(data):
    return data.upper()

class ScrabbleServerFactory(protocol.ServerFactory, object):
    '''
    ScrabbleServerFactory controls all the Games on the server and the Server Protocols for each
    client that is connected
    '''
    
    def __init__(self):
        '''
        Constructor.
        '''
        resources = manager.ResourceManager()
        self.clients = {}
        self.gameList = {}
        self.dicts = {}
        self.db = db.DB()
        self.maxUsersLoggedIn = 0
        self.startDate = util.Time(seconds=time.time(), dispDate=True)
        self.rankings = rank.Rankings( resources["config"][constants.RANK_CONFIG] )
        
        dir = resources["resources"][constants.DICT_DIR].path
        for lang in os.listdir( dir ):
            if not lang.islower(): continue # Avoids CVS directories
            self.dicts[lang] = set()
            for file in os.listdir( os.path.join(dir, lang) ):
                if not file.islower(): continue # Avoids CVS directories
                path = os.path.join(dir, lang, file)
                
                f = codecs.open(path, encoding='utf-8', mode='rb')
                lines = f.read().split()
                f.close()
                
                l = []
                for line in lines:
                    l.append( line.upper() )
                x = set( l )
                
                self.dicts[lang] = self.dicts[lang].union(x)
        
        for game in self.db.games.values():
            self.gameList[ game.getGameId() ] = game

    
    def removeClient(self, client):
        '''
        Remove a client
        
        @param client:
        '''
        
        if self.clients.has_key(client):
            del self.clients[client]
    
    def resetRanks(self):
        '''
        Reset users ranks
        '''
        for username in self.db.users.keys():
            self.db.users[username].setRank( 0 )
            self.db.users[username].rankName = self.rankings.getMinRank().name
    
    def auditUser(self, username, action, sync=True):
        '''
        Add an audit action for a user
        
        @param username:
        @param action:
        @param sync:
        '''
        try :
            u = util.getUnicode(username)
            self.db.users[u].addAuditAction( action )
            if sync:
                self.db.sync()
        except KeyError:
            pass
    
    def getServerBulletins(self):
        '''
        Return list of ServerBulletins
        '''
        if not self.db.messages.has_key(constants.SERVER_MESSAGE_KEY):
            return []
        else:
            return self.db.messages[constants.SERVER_MESSAGE_KEY]
        
    
    def addServerBulletin(self, message):
        '''
        Add server bulletin
        
        @param message: Bulletin message
        '''
        
        if not self.db.messages.has_key(constants.SERVER_MESSAGE_KEY):
            l = []
        else:
            l = self.db.messages[constants.SERVER_MESSAGE_KEY]
        
        b = self.createServerInfoMessage(message)
        l.append( util.ServerBulletin(data=b, id=util.getRandomId(), seconds=time.time()) )
        
        self.db.messages[constants.SERVER_MESSAGE_KEY] = l
        self.db.sync()
        
        for c in self.clients:
            c.postChatMessage( (b, True) )
    
    def deleteServerBulletin(self, id):
        '''
        Delete Server bulletin
        
        @param id: Bulletin ID
        '''
        l = self.db.messages[constants.SERVER_MESSAGE_KEY]
        key = int(id)
        l = [ m for m in l if m.id != key ]
        self.db.messages[constants.SERVER_MESSAGE_KEY] = l
        self.db.sync()
    
    def isLoggedIn(self, player):
        '''
        Check if a user was logged in
        
        @param player: Player
        '''
        
        for client,_player in self.clients.iteritems():
            if player.getUsername() ==  _player.getUsername():
                return True
        
        return False
    
    def getLoggedInPlayers(self):
        '''
        Retrieve listing of players logged in
        
        @return: Formatted string of players logged in
        '''
        str = ''
        for player in self.clients.values():
            str = str + player.getUsername() + ' '
        
        return str
        
    
    def bootUser(self, username):
        '''
        Boot a user from the server
        
        @param username: Username
        '''
        p = Player( username )
        c = self.getPlayerClient(p)
        
        if c is not None:
            for game in self.gameList.itervalues():
                if game.hasPlayer(p):
                    self.leaveGame(game.getGameId(), c, penalize=True, booted=True)
                    c.gameBoot(game.getGameId())
                if game.hasSpectator( player ):
                    cmd = helper.GameCommand(constants.GAME_LEAVE, game.getGameId(), '')
                    self.spectatorLeaveGame(cmd, client)
            c.logout()
            self.removeClient(c)
        
    
    def removeUser(self, username):
        '''
        Remove a user from the server.
        
        If the user is logged in, send a booted command
        
        @param username: Username
        '''
        self.bootUser(username)
        
        del self.db.users[util.getUnicode(username)]
        self.db.sync()
        
    
    def updateUser(self, user):
        '''
        Update a user
        
        @param user: User
        '''
        
        self.db.users[ user.getUsername() ] = user
    
    def doChangePassword(self, username, newPassword):
        '''
        Change a users password
         
        @param username:
        @param newPassword:
        '''
        
        user = self.db.users[util.getUnicode(username)]
        user.setPassword(newPassword)
                

    def getUsers(self):
        '''
        Return user objects
        
        @return: Dictionary of users
        '''
        x = self.db.users.values()[:]
        x.sort( lambda x,y : cmp(x.lastLogin, y.lastLogin) )
        return x
    
    def getUser(self, username):
        '''
        Get a user
        
        @param username: Username
        @return: User obect or not
        '''
        u = util.getUnicode(username)
        if self.db.users.has_key(u):
            return self.db.users[ u ]
        else:
            return None

    def stopFactory(self):
        '''
        Callback when the factory is stopping. Close the users file
        '''
        self.db.close()

    def buildProtocol(self, addr):
        '''
        Build a Server Protocol instance for each connection
        
        @param addr: Incoming address
        '''
        
        p = ScrabbleServer()
        p.factory = self
        p.addr = addr
        
        return p
    
    def hasUsers(self):
        '''
        Check to see if the server has users
        
        @return: True if there are users defined on the server
        '''
        return len(self.db.users) != 0
        
    
    def isUserAdmin(self, username):
        '''
        Check to see if a user is administrator
        
        @param username: Username
        @return: True if the user is an Administrator
        '''
        u = util.getUnicode(username)
        if not self.db.users.has_key(u):
            return False
        
        user = self.db.users[u]
        return user.isAdmin()
    
    def addNewUser(self, username, password, isAdmin):
        '''
        Add a new user
        
        @param username:
        @param password:
        @param isAdmin: True if the user is an administrator
        '''
        
        if username.upper() in map(upper, self.db.users.keys()):
            return False, USER_ALREADY_EXISTS
        
        if username in constants.RESERVED_NAMES:
            return False, USERNAME_NOT_ALLOWED
        
        if (len(username) > constants.MAX_NAME_LENGTH):
            return False, USERNAME_MUST_BE_LESS_THAN
        
        # Ensure that the first user created is an Admin
        if not self.hasUsers():
            isAdmin = True
        
        user = User( username, password, isAdmin )
        user.rankName = self.rankings.getMinRank().name
        self.db.users[ user.getUsername() ] = user
        self.db.sync()
        
        return True, SUCCESS
        
    
    def getGameListing(self):
        '''
        Return a Listing of ScrabbleGameInfo objects about each Game on the server
        
        @return: List
        '''
        return [ScrabbleGameInfo(game) for game in self.gameList.values()]
    
    def createGame(self, gameId, client, options):
        '''
        Create a new game
        
        @param gameId: Game ID
        @param client: Client
        @param options: Options
        '''
        
        if len(gameId) > constants.MAX_NAME_LENGTH:
            
            client.showError( ServerMessage( [GAME_NAME_MUST_BE_LESS_THAN, str(constants.MAX_NAME_LENGTH), CHARACTERS] ) )
            return
        
        if not self.gameList.has_key(gameId):
            game = ScrabbleGame( gameId, options )
            game.creator = self.clients[client].getUsername()
            self.gameList[ gameId ] = game
            self.refreshGameList()
        else:
            client.showError( ServerMessage([GAME_ALREADY_EXISTS]) )


    def deleteGame(self, gameId):
        '''
        Boot all the players and spectators from the game and then call removeGame
        
        @param gameId: Game ID
        '''
        
        if (not self.gameList.has_key(gameId)):
            return
        
        game = self.gameList[ gameId ]
        game.setComplete()
        
        self.sendGameInfoMessage(gameId, [SERVER_DELETE_GAME], client=None, level=constants.GAME_LEVEL)
        
        for player in game.getPlayers():
            c = self.getPlayerClient(player)
            if c:
                c.gameLeave(gameId, True)
        
        for s in game.getSpectators():
            c = self.getPlayerClient(s)
            if c:
                c.gameLeave(gameId, True)
    
        if not game.isPaused():
            if len(game.getPlayers()) == 0:
                self.removeGame(gameId)
        else:
            if len(game.getPending()) == len(game.getPlayers()):
                self.removeGame(gameId)
        
        self.refreshGameList()
    
    
    def removeGame(self, gameId):
        '''
        Remove a game from the system and database
        
        @param gameId:
        '''
        if self.gameList.has_key(gameId):
            del self.gameList[ gameId ] 
        if self.db.games.has_key(gameId):
            del self.db.games[ gameId ]
            self.db.sync()

    # Get a players client
    def getPlayerClient(self, player):
        '''
        Get the client protocol belonging to C{player}
        
        @param player: Player
        '''
        
        for client,_player in self.clients.iteritems():
            if player.getUsername() == _player.getUsername():
                return client

    # Authenticate a user
    def authenticate(self, username, password):
        '''
        Authenticate a user
        
        @param username:
        @param password:
        '''
        u = util.getUnicode(username)
        if self.db.users.has_key(u):
            user = self.db.users[u]
            if user.getPassword() == password:
                return True

        return False
    
    def sendGameStats(self, gameId):
        '''
        Send game statistics to each person in the game
        
        @param gameId: Game ID
        '''
        game = self.gameList[ gameId ]
        
        for p in game.getPlayers():
            c = self.getPlayerClient(p)
            if c:
                c.sendGameStats(gameId, game.getStats())
        
        for s in game.getSpectators():
            c = self.getPlayerClient(s)
            if c:
                c.sendGameStats(gameId, game.getStats())
    
    def sendSpectatorList(self, gameId):
        '''
        Send list of spectators
        
        @param gameId: Game ID
        '''
        game = self.gameList[ gameId ]
        
        l = game.getSpectators()

        for p in game.getPlayers():
            c = self.getPlayerClient(p)
            if c:
                c.gameSendSpectators(gameId, l)
        
        for s in game.getSpectators():
            c = self.getPlayerClient(s)
            if c:
                c.gameSendSpectators(gameId, l)
        
        
    
    def sendGameInfoMessage(self, gameId, message, client=None, level=constants.GAME_LEVEL):
        '''
        Send an information message to the players of a Game
        
        @param gameId: Game ID
        @param message: Message
        '''
        if (client == None):
            message = self.createServerChatMessage("GAME", message)
        else:
            message = self.createChatMessage(client.getUsername(), message)
        
        game = self.gameList[ gameId ]
        for p in game.getPlayers():
            c = self.getPlayerClient(p)
            if c:
                c.gameInfo(game.getGameId(), [(level, message)])
        
        for s in game.getSpectators():
            c = self.getPlayerClient(s)
            if c:
                c.gameInfo(game.getGameId(), [(level, message)])
        
        game.appendLogMessage( (level, message) )
        
    # Change a users password
    def changePassword(self, command, client):
        '''
        Change password
        
        @param command: LoginCommand
        @param client: ScrabbleServer Protocol
        '''
        
        if command.getUsername() != None and len(command.getUsername()) != 0:
            user = self.db.users[ command.getUsername() ]
        else:
            player = self.clients[client]
            user = self.db.users[ player.getUsername() ]
        
        if (user.getPassword() == command.getData()): # Data will have the old password
            user.setPassword( command.getPassword() )
            self.db.users[ user.getUsername() ] = user
            self.db.sync()
        else:
            client.showError( ServerMessage([INVALID_OLD_PASSWORD]) )

    def loginUser(self, player, client):
        '''
        Log a user into the system. Join the main chat
        
        @param player:
        @param client:
        '''
        
        self.joinChat( player, client )
        self.clients[client] = player
        
        self.db.users[player.getUsername()].setLastLogin( time.time() )
        self.db.sync()
        
        if len(self.clients) > self.maxUsersLoggedIn:
            self.maxUsersLoggedIn = len(self.clients)
    
    def handlePrivateMessageCommand(self, command, client):
        '''
        Send a private message
        
        @param command:
        @param client:
        '''
        
        
        if not self.db.users.has_key(command.getRecipient()):
            client.showError( ServerMessage([command.getRecipient(), DOES_NOT_EXIST]) )
            return
        
        p = Player( command.getRecipient() )
        c = self.getPlayerClient(p)
        
        sender = self.clients[client].getUsername()
        recipient = command.getRecipient()
        data = command.getData()
        
        if c is None:
            if not self.db.messages.has_key(recipient):
                l = []
            else:
                l = self.db.messages[recipient]
                self.db.messages[recipient] = []
            
            num = util.getRandomId()
            msg = util.PrivateMessage(sender, data, num, time=util.Time(seconds=time.time(), dispDate=True))
            
            l.append( msg )
            self.db.messages[recipient] = l
            self.db.sync()
            
            client.sendPrivateMessage(recipient, self.createChatMessage(sender, data))
            client.showInfo( ServerMessage([MESSAGE_SENT, ': %s' % recipient]) )
            
            return
            
        msg = self.createChatMessage(sender, data)    
        c.sendPrivateMessage(sender, msg)
        client.sendPrivateMessage(recipient, msg)
        

    def handleGameCommand(self, command, client):
        '''
        Handle a game command
        
        @param command: GameCommand
        @param client: ScrabbleServer Protocol
        '''
        
        if (command.getCommand() == constants.GAME_DIAMOND_REQUEST_REFRESH):
            print "Server got a refresh request!"
            client.doDiamondRefresh(command.getGameId())
        if (command.getCommand() == constants.GAME_GET_LETTERS):
            letters = self.game.getLetters( int(command.getData()) )
            client.sendLetters( letters )

        if (command.getCommand() == constants.GAME_LIST):
            client.sendGameList( self.getGameListing() )

        if (command.getCommand() == constants.GAME_JOIN):
            self.joinGame(command, client)

        if (command.getCommand() == constants.GAME_START):
            self.startGame( command.getGameId(), client )

        if (command.getCommand() == constants.GAME_LEAVE):
            if not self.gameList.has_key(command.getGameId()):
                return
                
            game = self.gameList[ command.getGameId() ]
            if game.hasPlayer(self.clients[client]):
                self.leaveGame( command.getGameId(), client )
            if game.hasSpectator(self.clients[client]):
                self.spectatorLeaveGame(command, client)

        if (command.getCommand() == constants.GAME_SEND_MOVE):
            onboard, moves = command.getData()
            self.gameSendMove( command.getGameId(), onboard, moves, client )

        if (command.getCommand() == constants.GAME_CREATE):
            self.createGame( command.getGameId(), client, command.getData() )
        if (command.getCommand() == constants.GAME_PASS):
            self.gamePassMove( command.getGameId(), client )

        if (command.getCommand() == constants.GAME_PAUSE):
            self.saveGame(command, client)

        if (command.getCommand() == constants.GAME_UNPAUSE):
            game = self.gameList[ command.getGameId() ]
            
            if self.clients[client].getUsername() != game.creator:
                client.gameError(game.getGameId(), ServerMessage([NOT_CREATOR]))
                return
            
            # We can only unpause if all former players are present
            if (len(game.getPending()) > 0):
                client.gameError(game.getGameId(), ServerMessage([REQUIRED_NOT_MET]))
                return
            
            game.unPause()
            for player in game.getPlayers():
                c = self.getPlayerClient(player)
                c.unPauseGame( game.getGameId() )
            self.refreshGameList()
            self.sendGameInfoMessage(command.getGameId(), [GAME_RESUMED], None, level=constants.GAME_LEVEL)
            self.doGameTurn(game.getGameId(), wasUnpaused=True)
            
            del self.db.games[ game.getGameId() ]
            self.db.sync()

        if (command.getCommand() == constants.GAME_TRADE_LETTERS):
            self.tradeLetters(command, client)
        
        if (command.getCommand() == constants.GAME_CHAT_MESSAGE):
            _level = constants.GAME_LEVEL
            game = self.gameList[ command.getGameId() ]
            if ( game.hasPlayer(self.clients[client]) ):
                _level = constants.PLAYER_LEVEL
            elif ( game.hasSpectator(self.clients[client]) ):
                _level = constants.SPECTATOR_LEVEL
            
            if _level == constants.SPECTATOR_LEVEL:
                if not game.isSpectatorChatEnabled():
                    command.setCommand( constants.GAME_ERROR )
                    command.setData( ServerMessage([SPECTATOR_CHAT_DISABLED]) )
                    client.writeCommand(command)
                    return
            
            self.sendGameInfoMessage(command.getGameId(), command.getData(), self.clients[client], level=_level)
        
        if (command.getCommand() == constants.GAME_SPECTATOR_JOIN):
            game = self.gameList[ command.getGameId() ]
            if not game.isSpectatorsAllowed():
                command.setCommand( constants.GAME_JOIN_DENIED )
                command.setData( ServerMessage([SPECTATORS_BANNED]) )
                client.denyJoinGame(command)
                return
            self.spectatorJoinGame(command, client)
        
        if (command.getCommand() == constants.GAME_SPECTATOR_CHAT_SET):
            game = self.gameList[ command.getGameId() ]
            game.setSpectatorChatEnabled(command.getData())
            
            if game.isSpectatorChatEnabled():
                self.sendGameInfoMessage(command.getGameId(), [self.clients[client].getUsername(), ENABLE_SPEC_CHAT], client=None, level=constants.GAME_LEVEL)
            else:
                self.sendGameInfoMessage(command.getGameId(), [self.clients[client].getUsername(), DISABLE_SPEC_CHAT], client=None, level=constants.GAME_LEVEL)
            
            for p in game.getPlayers():
                c = self.getPlayerClient(p)
                c.writeCommand(command)
        
        if (command.getCommand() == constants.GAME_SPECTATOR_SET):
            game = self.gameList[ command.getGameId() ]
            game.setSpectatorsAllowed(command.getData())
            
            if game.isSpectatorsAllowed():
                self.sendGameInfoMessage(command.getGameId(), [self.clients[client].getUsername(), ENABLE_SPEC], client=None, level=constants.GAME_LEVEL)
            else:
                self.sendGameInfoMessage(command.getGameId(), [self.clients[client].getUsername(), DISABLE_SPEC], client=None, level=constants.GAME_LEVEL)
            
            for p in game.getPlayers():
                c = self.getPlayerClient(p)
                c.writeCommand(command)
            
            if not game.isSpectatorsAllowed():
                for s in game.getSpectators():
                    c = self.getPlayerClient(s)
                    self.spectatorLeaveGame(command, c)
                    c.gameBoot(command.getGameId())
        
        if (command.getCommand() == constants.GAME_TIME_EXPIRE):
            self.gameTimeExpired( command.getGameId(), client )
        
        if (command.getCommand() == constants.GAME_MOVE_TIME_EXPIRE):
            self.moveTimeExpired(command.getGameId(), client )
            

    def handleChatCommand(self, command, client):
        '''
        Handle a chat command
        
        @param command: ChatCommand
        @param client: ScrabbleServer Protocol
        '''
        
        if (command.getCommand() == constants.CHAT_JOIN):
            self.joinChat( self.clients[client], client )
            
        if (command.getCommand() == constants.CHAT_LEAVE):
            if self.clients.has_key(client):
                player = self.clients[client]
                
                # Remove player from game and notify other players
                for game in self.gameList.values():
                    if game.hasPlayer( player ):
                        self.leaveGame( game.getGameId(), client, command.getData() )
                    
                    if game.hasSpectator( player ):
                        cmd = helper.GameCommand(constants.GAME_LEAVE, game.getGameId(), '')
                        self.spectatorLeaveGame(cmd, client)
                
                self.removeClient(client)
                self.leaveChat( player )
                client.logout()
                

        if (command.getCommand() == constants.CHAT_USERS):
            self.sendUserList(client)
    
        if (command.getCommand() == constants.CHAT_MESSAGE):
            self.postChatMessage( self.clients[client], command.getData() )
        
        if (command.getCommand() == constants.USER_INFO):
            if not self.db.users.has_key(command.getUsername()):
                client.showError( ServerMessage([command.getUsername(), DOES_NOT_EXIST]) )
                return
            u = self.db.users[command.getUsername()].clone()
            u.status = self.getUserStatus(command.getUsername())
            client.sendUserInfo(u)
        
        if (command.getCommand() == constants.SERVER_STATS):
            client.sendServerStats(self.getStats())
        
        if (command.getCommand() == constants.CHECK_MESSAGES):
            # Print server bulletins first
            if self.db.messages.has_key(constants.SERVER_MESSAGE_KEY):
                for message in self.db.messages[constants.SERVER_MESSAGE_KEY]:
                    client.postChatMessage( (message.data, True) )
            
            key = self.clients[client].getUsername()
            if self.db.messages.has_key(key):
                if len(self.db.messages[key]) > 0:
                    new = False
                    for m in self.db.messages[key]:
                        if not m.read:
                            new = True
                    
                    if new:
                        client.postChatMessage( (self.createServerInfoMessage(MESSAGES_AVAILABLE), True) )
                    else:
                        client.postChatMessage( (self.createServerInfoMessage(OLD_MESSAGES_AVAILABLE), True) )
                
        
        if (command.getCommand() == constants.GET_MESSAGES):
            key = self.clients[client].getUsername()
            if self.db.messages.has_key(key):
                for m in self.db.messages[key]:
                    m.read = True
                client.sendOfflineMessages( self.db.messages[key] )
            else:
                client.sendOfflineMessages( [] )
            self.db.sync()
        
        if (command.getCommand() == constants.DELETE_MESSAGE):
            key = self.clients[client].getUsername()
            l = self.db.messages[key]
            data = int(command.getData())
            l = [ m for m in l if m.id != data ]
            if len(l) != 0:
                self.db.messages[key] = l
            else:
                del self.db.messages[key]
            self.db.sync()

    # Alert other users that a user has joined
    def joinChat(self, player, client):
        '''
        User joins chat
        
        @param player:
        @param client:
        '''
        
        for c in self.clients.keys():
            if (c != client):
                c.joinChat( player.getUsername() )
        
        for c in self.clients.keys():
            c.postChatMessage( (self.createLoginMessage(player.getUsername()), True) )
        
        self.auditUser( player.getUsername(), audit.LogonAction(player.getUsername()) )

    # Log a user of the system
    def leaveChat(self, player):
        '''
        User leaves chat
        
        @param player:
        '''
        
        for c in self.clients.keys():
            c.postChatMessage( (self.createLogoutMessage(player.getUsername()),True) )
                
        for c in self.clients.keys():
            self.sendUserList(c)
        
        self.auditUser( player.getUsername(), audit.LogoffAction(player.getUsername()) )

    # Post a chat message
    def postChatMessage(self, player, msg):
        '''
        User posts a chat message
        
        @param player:
        @param msg:
        '''
        
        for c in self.clients.keys():
            c.postChatMessage( (self.createChatMessage(player.getUsername(), msg), False) )

    # Send the list of users to the client
    def sendUserList(self, client):
        '''
        Send the client a list of all users on the server
        
        @param client:
        '''
        
        client.sendUserList( [ self.clients[c] for c in self.clients.keys()] )

    # Create formatted chat message
    def createServerChatMessage(self, username, msg_keys):
        '''
        Helper function to create a server chat message
        
        @param username:
        @param msg_keys:
        '''
        
        x = []
        x.append( "<%s>" % (username) )
        x.extend( msg_keys )
        x.append( "\n" )
        
        
        return ServerMessage(x, util.Time(seconds=time.time(), dispDate=False))
    
    def createServerInfoMessage(self, msg):
        '''
        Helper function to create a serverchat message
        
        @param msg:
        '''
        
        return ServerMessage([msg,"\n"], util.Time(seconds=time.time(), dispDate=True))

    # Create formatted chat message
    def createChatMessage(self, username, msg):
        '''
        Helper function to create a chat message
        
        @param username:
        @param msg:
        '''
        return ServerMessage(["<%s> %s\n" % (username, util.getUnicode(msg))],util.Time(seconds=time.time(), dispDate=False))

    # Create logout message
    def createLogoutMessage(self, username):
        '''
        Helper function to create a logout message
        
        @param username:
        '''
        
        
        return ServerMessage(["<%s>" % (username), LOGGED_OUT, "\n"], util.Time(seconds=time.time(), dispDate=False))
    
    # Create logout message
    def createLoginMessage(self, username):
        '''
        Helper function to create a login message
        
        @param username:
        '''
        return ServerMessage(["<%s>" % (username), LOGGED_IN, "\n"], util.Time(seconds=time.time(), dispDate=False))

    def refreshGameList(self):
        '''
        Send all clients the current game list
        '''
        
        for c in self.clients.keys():
            c.sendGameList( [ScrabbleGameInfo(game) for game in self.gameList.values()] )
        
    # Join a game
    def joinGame(self, command, client):
        '''
        User joins a game
        
        @param command: GameCommand
        @param client: ScrabbleServer protocol
        '''
        game = self.gameList[ command.getGameId() ]
        if (game.isStarted()):
            command.setData( ServerMessage([CANNOT_JOIN_STARTED]) )
            command.setCommand( constants.GAME_JOIN_DENIED )
            client.denyJoinGame(command)
            return
        
        if (game.getNumberOfPlayers() == constants.MAX_PLAYERS):
            command.setData( ServerMessage([GAME_FULL]) )
            command.setCommand( constants.GAME_JOIN_DENIED )
            client.denyJoinGame(command)
            return

        p = self.clients[client].clone()
        
        if (game.isPaused() and not game.hasPlayer(p)):
            command.setData( ServerMessage([CANNOT_JOIN_STARTED]) )
            command.setCommand( constants.GAME_JOIN_DENIED )
            client.denyJoinGame(command)
            return

        if not game.hasPlayer( p ):
            game.addPlayer( p )
        else:
            game.removePending( p )
        
        command.setCommand( constants.GAME_JOIN_OK )
        client.acceptJoinGame( command, game.options )
        
        time = None
        if game.options.has_key(OPTION_TIMED_GAME):
            time = int(game.options[OPTION_TIMED_GAME])
        elif game.options.has_key(OPTION_MOVE_TIME):
            time = int(game.options[OPTION_MOVE_TIME])
        
        if time is not None:
            p.setInitialTime( time )
        
        players = game.getPlayers()
        pending = game.getPending()
        
        self.sendGameScores(game.getGameId())
        
        client.sendMoves( game.getGameId(), game.getMoves() )
        client.sendGameStats( game.getGameId(), game.getStats() )
        client.gameInfo( game.getGameId(), game.getLog() )
        client.gameSendSpectators( game.getGameId(), game.getSpectators() )
        client.sendGameOptions( game.getGameId(), game.getOptions() )
        
        client.setSpectatorChatEnabled(game.getGameId(), game.isSpectatorChatEnabled())
        client.setSpectatorsAllowed(game.getGameId(), game.isSpectatorsAllowed())
        
        if (game.isPaused()):
            client.pauseGame( game.getGameId() )

        if (game.isPaused() and game.isInProgress()):
            
            player = game.getPlayer( self.clients[client] )
            
            letters = game.getLetters( player.getNumberOfLettersNeeded() )
            if (len(letters) > 0):
                player.addLetters(letters)
            client.sendLetters(game.getGameId(), player.getLetters())

            # If there is only one person (the new person), make sure that person has control of the board
            #if (len(players) == 1):
            #    self.doGameTurn( game.getGameId() )

        self.refreshGameList()
        
    # Start the game
    def startGame(self, gameId, client):
        '''
        User starts a game
        
        @param gameId: Game ID
        @param client: ScrabbleServer Protocol
        '''
        
        game = self.gameList[ gameId ]
        
        if self.clients[client].getUsername() != game.creator:
            client.gameError(gameId, ServerMessage([NOT_CREATOR]))
            return
        
        if (game.isStarted()):
            client.gameError(gameId, ServerMessage([GAME_ALREADY_STARTED]))
            return

        game.start()
        
        time = None
        if game.options.has_key(OPTION_TIMED_GAME):
            time = int(game.options[OPTION_TIMED_GAME])
        elif game.options.has_key(OPTION_MOVE_TIME):
            time = int(game.options[OPTION_MOVE_TIME])
            
        for player in game.getPlayers():
            c = self.getPlayerClient(player)
            letters = game.getLetters( player.getNumberOfLettersNeeded() )
            player.addLetters(letters)
            if time is not None:
                player.setInitialTime( time )
            c.sendLetters( game.getGameId(), letters )
        
        self.sendGameInfoMessage(gameId, [gameId, STARTED], client=None, level=constants.GAME_LEVEL)
        self.sendGameScores(game.getGameId())

        self.doGameTurn( gameId )
        self.refreshGameList()

    # Turn gameplayer over to the next player and notify other players of whose turn it is
    def doGameTurn(self, gameId, wasUnpaused=False ):
        '''
        Turn control of the board to the next player in the game
        
        @param gameId: Game ID
        '''
        
        game = self.gameList[ gameId ]
        
        player = game.getNextPlayer()
        
        if player is None:
            return
        
        player.stamp = datetime.datetime.now()
        client = self.getPlayerClient(player)
        
        if game.timer is not None and game.timer.active():
            game.timer.cancel()
        
        time = player.time
        if game.options.has_key(OPTION_MOVE_TIME):
            if not wasUnpaused:
                time = datetime.timedelta(seconds=60 * int(game.options[OPTION_MOVE_TIME]))
        
        if game.options.has_key(OPTION_MOVE_TIME):
            game.timer = reactor.callLater(time.seconds, self.moveTimeExpired, gameId, client)
        elif game.options.has_key(OPTION_TIMED_GAME):
            if game.options.has_key(OPTION_TIMED_LIMIT):
                t = 60 * int(game.options[OPTION_TIMED_LIMIT])
            else:
                t = 0
            x = datetime.timedelta(seconds=t + time.seconds)
            if game.options.has_key(OPTION_TIMED_LIMIT):
                if time.days < 0:
                    x = -time
                    x = datetime.timedelta(seconds=t - x.seconds )
            game.timer = reactor.callLater(x.seconds, self.gameTimeExpired, gameId, client)
            
        client.gameTurnCurrent(gameId, time)
        
        for _player in game.getPlayers():
            _client = self.getPlayerClient(_player)
            if (_player != player):
                _client.gameTurnOther( gameId, PlayerInfo(player.getUsername(), player.getScore(), len(player.getLetters()), player.time ))
        
        for s in game.getSpectators():
            c = self.getPlayerClient(s)
            c.gameTurnOther( gameId, PlayerInfo(player.getUsername(), player.getScore(), len(player.getLetters()), player.time ))
        
        self.sendGameInfoMessage(gameId, [player.getUsername(),TURN], client=None, level=constants.GAME_LEVEL)
        self.sendGameStats(gameId)
        if game.options[OPTION_SHOW_COUNT]:
            self.sendLetterDistribution(gameId)
    
    def sendLetterDistribution(self, gameId):
        '''
        Send the letter distribution
        
        @param gameId: Game ID
        '''
        game = self.gameList[ gameId ]
        
        for p in game.getPlayers():
            c = self.getPlayerClient(p)
            c.sendLetterDistribution( gameId, game.getDistribution() )
        
        for s in game.getSpectators():
            c = self.getPlayerClient(s)
            c.sendLetterDistribution( gameId, game.getDistribution() )
        
    
    def moveTimeExpired(self, gameId, client):
        '''
        Move time for a player has expired
        
        @param gameId:
        @param client:
        '''
        game = self.gameList[ gameId ]
        player = game.getPlayer( self.clients[client] )
        player.time = datetime.timedelta(seconds=60 * int(game.options[OPTION_MOVE_TIME]))

        self.sendGameScores(gameId)
        self.sendGameInfoMessage(gameId, [player.getUsername(),MOVE_OUT_OF_TIME], client=None, level=constants.GAME_LEVEL)
        self.doGameTurn(gameId)
    
    def gameTimeExpired(self, gameId, client):
        '''
        Game time expired
        
        @param gameId: Game ID
        @param client: Player client who has run out of time
        '''
        game = self.gameList[ gameId ]
        player = game.getPlayer( self.clients[client] )

        self.sendGameInfoMessage(gameId, [player.getUsername(),OUT_OF_TIME], client=None, level=constants.GAME_LEVEL)
        player.time = datetime.timedelta(seconds=0)
        
        self.gameOver(game, player)

    # Player leave game
    def leaveGame(self, gameId, client, penalize=True, booted=False):
        '''
        Player leaves a game
        
        @param gameId: Game ID
        @param client: ScrabbleServer Protocol
        @param penalize: Flag to penalize the player a loss.  Not penalized if the connection was not closed cleanly
        '''
        
        game = self.gameList[ gameId ]
        player = game.getPlayer( self.clients[client] )
        
        if (game.isPaused() and not booted):
            game.addPending( player )
            return
        
        if not game.isComplete() and len(game.getPlayers()) > 1:
            if game.isStarted():
                if game.options[OPTION_RANKED] and penalize:
                    self.db.users[ player.getUsername() ].addLoss(None)
        
        game.playerLeave(player)
        
        self.sendGameScores(gameId)
        
        self.sendGameInfoMessage(gameId, [player.getUsername(),LEFT_GAME], client=None, level=constants.GAME_LEVEL)

        # If there are no more players left, remove the game
        if len(game.getPlayers()) == 0 and len(game.getSpectators()) == 0 and not game.isPaused():
            #for s in game.getSpectators():
            #    c = self.getPlayerClient( s )
            #    c.gameLeave( game.getGameId() )
            if game.timer is not None and game.timer.active():
                game.timer.cancel()
            del self.gameList[ gameId ]
        elif not game.isComplete() and not game.isPaused():
            if game.isCurrentPlayer( player ):
                self.doGameTurn( gameId )

        self.refreshGameList()
        
    
    def checkServerStats(self, player, moves):
        '''
        Check if this move is a new server stat
        
        @param player: Player
        @param moves: Moves
        '''
        
        score = 0
        allDisp = ''
        for move in moves:
            score = score + move.getScore()
            newdisp = '%s (%s) by %s' % (move.getWord(), str(move.getScore()), player.getUsername())
            if self.db.stats.has_key(STAT_HIGHEST_SCORING_WORD):
                data,disp = self.db.stats[STAT_HIGHEST_SCORING_WORD]
                if (move.getScore() > data.getScore()):
                    self.db.stats[STAT_HIGHEST_SCORING_WORD] = move,newdisp
            else:
                self.db.stats[STAT_HIGHEST_SCORING_WORD] = move,newdisp
            
            newdisp = '%s (%s) by %s' % (move.getWord(), str(move.length()), player.getUsername())
            if self.db.stats.has_key(STAT_LONGEST_WORD):
                data,disp = self.db.stats[STAT_LONGEST_WORD]
                if (move.length() > data.length()):
                    self.db.stats[STAT_LONGEST_WORD] = move,newdisp
            else:
                self.db.stats[STAT_LONGEST_WORD] = move,newdisp
            allDisp = allDisp + '%s (%s) ' % (move.getWord(), str(move.getScore()))
        
        allDisp = allDisp + 'by %s' % player.getUsername()
        d = allDisp
        d = '%s, ' % str(score) + d
        if self.db.stats.has_key(STAT_HIGHEST_SCORING_MOVE):
            data, disp = self.db.stats[STAT_HIGHEST_SCORING_MOVE]
            if score > data:
                self.db.stats[STAT_HIGHEST_SCORING_MOVE] = score, d
        else:
            self.db.stats[STAT_HIGHEST_SCORING_MOVE] = score, d
        
        d = allDisp
        d = '%s, ' % str(len(moves)) + d
        if self.db.stats.has_key(STAT_MOST_WORDS_IN_MOVE):
            data, disp = self.db.stats[STAT_MOST_WORDS_IN_MOVE]
            if len(moves) > data:
                self.db.stats[STAT_MOST_WORDS_IN_MOVE] = len(moves), d
        else:
            self.db.stats[STAT_MOST_WORDS_IN_MOVE] = len(moves), d
        
        self.db.sync()
        

    # Player send move to game
    def gameSendMove(self, gameId, onboard, moves, client):
        '''
        User sends moves to the game
        
        @param gameId: Game ID
        @param onboard: Move containing letters put on the board
        @param moves: List of Moves formed
        @param client: ScrabbleServer Protocol
        '''
        
        game = self.gameList[ gameId ]
        player = game.getPlayer( self.clients[client] )
        
        if not player == game.getCurrentPlayer():
            return
        
        if (game.isPaused()):
            client.gameError(gameId, ServerMessage([MOVE_GAME_PAUSED]) )
            return
        if (not game.isInProgress()):
            client.gameError(gameId, ServerMessage([NOT_IN_PROGRESS]) )
            return
            
        
        # Validate word in dictionary and not on the board alread
        words = []
        for move in moves:
            word = util.getUnicode( move.getWord() )
            if word not in self.dicts[ game.options[OPTION_RULES] ]:
                client.gameError( gameId, ServerMessage([word, NOT_IN_DICT]) )
                return
            words.append( word )
        
        
        
        client.acceptMove(gameId)
        score = self.getMovesScore(game, moves)
        letters = self.getLettersFromMove(onboard)
        player.removeLetters( letters )
        player.addScore( score )
        
        self.removeModifiers(game, moves)
        
        game.addMoves(moves, player)
        
        game.resetPassCount()
        
        if len(game.getPlayers()) > 1:
            self.checkServerStats(player, moves)
        
        # If the player used all 7 of his/her letters, give them an extra 50
        if onboard.length() == 7:
            player.addScore( constants.BINGO_BONUS_SCORE )
            self.sendGameInfoMessage(gameId, [player.getUsername(), MADE_A_BINGO, '(%s)' % str(constants.BINGO_BONUS_SCORE)], client=None, level=constants.GAME_LEVEL)
        
        for p in game.getPlayers():
            c = self.getPlayerClient(p)
            c.sendMoves( gameId, moves )
        
        for s in game.getSpectators():
            c = self.getPlayerClient(s)
            c.sendMoves( gameId, moves )
            
        for move in moves:
            self.sendGameInfoMessage(gameId, [player.getUsername(), HAS_ADDED, ' %s (%d)' % (move.getWord(), move.getScore())], client=None, level=constants.GAME_LEVEL)
            
            # If the player used all his/her letters and there are no more letters in the bag, the game is over
        if (len(player.getLetters()) == 0 and game.isBagEmpty()):
            
            # Subtract everyones letter points
            # Give points to the person who went out
            players = game.getPlayers()
            for p in players:
                if p == player: 
                    continue # Skip winner
                    
                letters = p.getLetters()
                lmsg = ''
                wmsg = ''
                for letter in letters:
                    p.addScore( letter.getScore() * -1 )
                    player.addScore( letter.getScore() )
                    lmsg = lmsg + '%s(%d) ' % (letter.getLetter(), letter.getScore() * -1)
                    wmsg = wmsg + '%s(%d) ' % (letter.getLetter(), letter.getScore())
                
                self.sendGameInfoMessage(gameId, [p.getUsername(), LOSES, lmsg], client=None, level=constants.GAME_LEVEL)
                self.sendGameInfoMessage(gameId, [player.getUsername(), GAINS, wmsg, FROM, p.getUsername()], client=None, level=constants.GAME_LEVEL)
            
            self.sendGameScores(game.getGameId())
            
            self.gameOver(game)
            return

        letters = game.getLetters( player.getNumberOfLettersNeeded() )
        if (len(letters) > 0):
            player.addLetters(letters)
            client.sendLetters(gameId, player.getLetters())
        
        
        if game.options.has_key(OPTION_TIMED_GAME):
            player.time = player.time - (datetime.datetime.now() - player.stamp)
            player.time = datetime.timedelta(days=player.time.days, seconds=player.time.seconds+1 ) # +1 account for error
            if game.timer is not None and game.timer.active():
                game.timer.cancel()
        
        self.sendGameScores(game.getGameId())
        
        if game.isBagEmpty() or game.getCountLetters() < 7:
            for p in game.getPlayers():
                c = self.getPlayerClient(p)
                c.gameBagEmpty(gameId)
            for s in game.getSpectators():
                c = self.getPlayerClient(s)
                c.gameBagEmpty(gameId)
            

        # Next player
        self.doGameTurn(gameId)

    # Get the letters from the moves
    def getLettersFromMove(self, move):
        '''
        Get the letters in a move
        
        @param move: Move
        @return: List of letters in C{move}
        '''
        
        letters = []
        
        for letter, x, y in move.getTiles():
            letters.append( letter.clone() )

        return letters
    
    # Get the score of the list of moves
    def getMovesScore(self, game, moves):
        '''
        Get the total score for a list of Moves
        
        @param game: ScrabbleGame
        @param moves: List of Moves
        @return: Total score for the list of Moves
        '''
        
        total = 0
        
        for move in moves:
            score = 0
            apply = 0
            modifier = constants.TILE_NORMAL
            m_x = -1
            m_y = -1
            for letter,x,y in move.getTiles():
                m = util.getTileModifier(x,y)
                if m in constants.LETTER_MODIFIERS and not game.hasUsedModifier((x,y)):
                    score = score + (m * letter.getScore())
                else:
                    score = score + letter.getScore()
    
                if (m >= modifier and not game.hasUsedModifier((x,y))):
                    modifier = m
                    if m in constants.WORD_MODIFIERS:
                        apply = apply + 1
                    m_x = x
                    m_y = y

            if modifier in constants.WORD_MODIFIERS and not game.hasUsedModifier((m_x,m_y)):
                
                if util.isCenter(m_x, m_y):
                    if game.options[OPTION_CENTER_TILE]:
                        score = score * (modifier/2)
                else:
                    score = score * ((modifier/2) ** apply)
                
            move.setScore( score )
            total = total + score
        
        return total
    
    def removeModifiers(self, game, moves):
        '''
        Mark off modifiers that are used in this move
        
        @param game: Game
        @param moves: List of moves
        '''
        for move in moves:
            for letter,x,y in move.getTiles():
                m = util.getTileModifier(x,y)
                if m in constants.LETTER_MODIFIERS and not game.hasUsedModifier((x,y)):
                    game.addUsedModifier( (x,y) )
                if m in constants.WORD_MODIFIERS and not game.hasUsedModifier((x,y)):
                    game.addUsedModifier( (x,y) )
        


    # Player passes a move.  If all players pass, the game is over
    def gamePassMove(self, gameId, client):
        '''
        Player passes his/her turn
        
        @param gameId: Game ID
        @param client: ScrabbleServer Protocol
        '''
        
        game = self.gameList[ gameId ]
        player = game.getPlayer( self.clients[client] )
        
        if not player == game.getCurrentPlayer():
            return
        
        if (not game.isInProgress()):
            client.gameError(gameId, ServerMessage([NOT_IN_PROGRESS]) )
            return
        
        if (game.isPaused()):
            client.gameError(gameId, ServerMessage([PASS_PAUSED]) )
            return
        
        try:
            self.sendGameInfoMessage(gameId, [player.getUsername(), HAS_PASSED], client=None, level=constants.GAME_LEVEL)
            
            if game.options.has_key(OPTION_TIMED_GAME):
                player.time = player.time - (datetime.datetime.now() - player.stamp)
                player.time = datetime.timedelta(days=player.time.days, seconds=player.time.seconds)
                if game.timer is not None and game.timer.active():
                    game.timer.cancel()
            
            game.passMove()
            
            self.sendGameScores(gameId)
            
            self.doGameTurn(gameId)
        except exceptions.GameOverException:
            # If everyone has passed, assume that everyone still has letters
            # Subtract everyones letter points
            players = game.getPlayers()
            for player in players:
                letters = player.getLetters()
                msg = ''
                for letter in letters:
                    player.addScore( letter.getScore() * -1 )
                    msg = msg + '%s(%d) ' % (letter.getLetter(), letter.getScore() * -1)
                
                self.sendGameInfoMessage(gameId, [player.getUsername(), LOSES, msg], client=None, level=constants.GAME_LEVEL)
            
            self.sendGameScores(game.getGameId())
            self.gameOver(game)
    
    def sendGameScores(self, gameId):
        '''
        Send game scores
        '''
        game = self.gameList[ gameId ]
        players = game.getPlayers()
        
        for p in players:
            c = self.getPlayerClient(p)
            if (c):
                c.sendGameUserList( game.getGameId(), self.getGamePlayerInfo(gameId, removePending=True) )
        
        for s in game.getSpectators():
            c = self.getPlayerClient(s)
            if (c):
                c.sendGameUserList( game.getGameId(), self.getGamePlayerInfo(gameId, removePending=False) )
    
    def getGamePlayerInfo(self, gameId, removePending=False):
        '''
        Return list of PlayerInfo objects for each Player in a game
        
        @param gameId: Game ID
        @param removePending: True to remove pending players from list
        '''
        game = self.gameList[ gameId ]
        players = game.getPlayers()
        
        if removePending:
            if (game.isPaused()):
                x = []
                pending = game.getPending()
                for p in players:
                    if p not in pending:
                        x.append(p)
                players = x
        
        return [ PlayerInfo(p.getUsername(), p.getScore(), len(p.getLetters()), util.TimeDeltaWrapper(p.time)) for p in players ]
        
        

    def gameOver(self, game, exclude=None):
        '''
        Declare game over for C{game}
        
        @param game: Game
        @param exclude: Player to exclude
        '''
        
        if game.timer is not None and game.timer.active():
            game.timer.cancel()
        
        if game.options.has_key(OPTION_TIMED_GAME):
            for p in game.getPlayers():
                t = p.time
                if t.days < 0:
                    t = -t
                    mins = math.ceil( (t.seconds / 60.0) )
                    score = int((mins * constants.OVERTIME_PENALTY) * -1)
                    p.addScore( score )
                    msg = '%d points' % score
                    self.sendGameInfoMessage(game.getGameId(), [p.getUsername(), LOSES, msg], client=None, level=constants.GAME_LEVEL)
            
        winners = game.getWinners(exclude)
        
        if len(winners) > 0:
            newdisp = '%s by ' % (winners[0].getScore())
            for w in winners:
                newdisp = newdisp + w.getUsername() + ' '
            
            if len(game.getPlayers()) > 1:    
                if self.db.stats.has_key(STAT_HIGHEST_TOTAL_SCORE):
                    data,disp = self.db.stats[STAT_HIGHEST_TOTAL_SCORE]
                    if (winners[0].getScore() > data):
                        self.db.stats[STAT_HIGHEST_TOTAL_SCORE] = winners[0].getScore(),newdisp
                else:
                    self.db.stats[STAT_HIGHEST_TOTAL_SCORE] = winners[0].getScore(),newdisp
        
        winners = game.getWinners(exclude)
        
        # If there is less than one player in the game, don't count the score
        count = len(game.getPlayers()) > 1
        
        if len(winners) > 0:
            if len(winners) == 1:
                winner = winners[0]
                self.sendGameInfoMessage(game.getGameId(), ['%s (%d)' % (winner.username, int(winner.score)), HAS_WON], client=None, level=constants.GAME_LEVEL)
                if count:
                    if game.options[OPTION_RANKED]:
                        self.db.users[ winner.getUsername() ].addWin( game.getPlayers() )
                        self.setRankForPlayer( winner.getUsername() )
                        self.auditUser( winner.getUsername(), audit.GameWinAction(winner, game.name, game.getPlayers()), False )
            else:
                msg = ''
                for winner in winners:
                    msg += '%s (%d)' % (winner.username, int(winner.score))
                    msg += ', '
                    if count:
                        if game.options[OPTION_RANKED]:
                            self.db.users[ winner.getUsername() ].addTie( winners )
                            self.auditUser( winner.getUsername(), audit.GameTieAction(winner, game.name, winners), False )
                msg = msg[:-2]
                self.sendGameInfoMessage(game.getGameId(), [msg, HAVE_TIED], client=None, level=constants.GAME_LEVEL)
        
        for p in game.getPlayers():
            if p not in winners:
                if count:
                    if game.options[OPTION_RANKED]:
                        self.db.users[ p.getUsername() ].addLoss(winners)
                        self.auditUser( p.getUsername(), audit.GameLossAction(winners, game.name, p), False )
            c = self.getPlayerClient( p )
            c.gameOver( game.getGameId() )
        
        for s in game.getSpectators():
            c = self.getPlayerClient( s )
            c.gameLeave( game.getGameId() )
        
        self.sendGameScores(game.getGameId())
        
        self.gameList[ game.getGameId() ].setComplete()
        self.refreshGameList()
        
        self.db.sync()

    # Player trades in current set of letters for new letters
    def tradeLetters(self, command, client):
        '''
        Player trades letters
        
        @param command: GameCommand
        @param client: ScrabbleServer Protocol
        '''
        
        game = self.gameList[ command.getGameId() ]
        player = game.getPlayer( self.clients[client] )
        
        if not player == game.getCurrentPlayer():
            return
        
        if (not game.isInProgress()):
            client.gameError(gameId, ServerMessage([NOT_IN_PROGRESS]) )
            return
        
        l = command.getData()
        num = len(l)
        player.removeLetters( l )
        letters = game.getLetters( player.getNumberOfLettersNeeded() )
        player.addLetters( letters )
        game.returnLetters( l )
        client.sendLetters( game.getGameId(), player.getLetters() )
        
        game.resetPassCount()
        
        if game.options.has_key(OPTION_TIMED_GAME):
            player.time = player.time - (datetime.datetime.now() - player.stamp)
            player.time = datetime.timedelta(days=player.time.days, seconds=player.time.seconds)
            if game.timer is not None and game.timer.active():
                game.timer.cancel()
        
        self.sendGameScores(command.getGameId())
        
        self.sendGameInfoMessage(game.getGameId(), [player.getUsername(),HAS_TRADED, '%s' % str(num), util.ternary(num == 1, LETTER, LETTERS)], client=None, level=constants.GAME_LEVEL)
        
        self.doGameTurn(game.getGameId())
    
    
    def spectatorJoinGame(self, command, client):
        '''
        Spectator joins the game
        
        @param command:
        @param client:
        '''
        game = self.gameList[ command.getGameId() ]
        
        command.setCommand( constants.GAME_SPECTATE_JOIN_OK )
        client.acceptJoinGame( command, game.options  )

        player = self.clients[client].clone()
        game.addSpectator( player )
        
        client.sendGameUserList( game.getGameId(), self.getGamePlayerInfo(game.getGameId()) )
        client.sendMoves( game.getGameId(), game.getMoves() )
        client.gameInfo( game.getGameId(), game.getLog() )
        client.sendGameStats( game.getGameId(), game.getStats() )
        client.sendGameOptions( game.getGameId(), game.getOptions() )
        
        if game.options.has_key(OPTION_TIMED_GAME) or game.options.has_key(OPTION_MOVE_TIME):
            if not game.isPaused():
                p = game.getCurrentPlayer()
                if p is not None:
                    time = p.time - (datetime.datetime.now() - p.stamp)
                    time = datetime.timedelta(days=p.time.days, seconds=time.seconds+1 ) # +1 account for error
                    client.gameTurnOther( game.getGameId(), PlayerInfo(p.getUsername(), p.getScore(), len(p.getLetters()), time ))
            
        self.sendGameInfoMessage(game.getGameId(), [player.getUsername(), IS_SPECTATING], client=None, level=constants.GAME_LEVEL)
        self.sendSpectatorList( game.getGameId() )
    
    def spectatorLeaveGame(self, command, client):
        '''
        Spectator leaves the game
        
        @param command:
        @param client:
        '''
        game = self.gameList[ command.getGameId() ]
        game.spectatorLeave(self.clients[client])
        
        self.sendGameInfoMessage(game.getGameId(), [self.clients[client].getUsername(), NO_LONGER_SPECTATING], client=None, level=constants.GAME_LEVEL)
        self.sendSpectatorList( game.getGameId() )
        if len(game.getPlayers()) == 0 and len(game.getSpectators()) == 0 and not game.isPaused():
            if game.timer is not None and game.timer.active():
                game.timer.cancel()
            del self.gameList[ game.getGameId() ]
            self.refreshGameList()
    
    def getStats(self):
        '''
        Retrieve list of game stats and the list of users
        
        @return: List of tuples (stat_name, stat_value), users
        '''
        
        s = []
        s.append( (ServerMessage([NUMBER_USERS]), str(len(self.db.users))) )
        s.append( (ServerMessage([MOST_USERS]), str(self.maxUsersLoggedIn)) )
        s.append( (ServerMessage([UPTIME]), self.startDate) )
        s.append( (ServerMessage([SERVER_VERSION]), constants.VERSION) )
        
        for key,value in self.db.stats.iteritems():
            data,disp = value
            s.append ( (ServerMessage([key]), disp) )
        
        users = []
        for user in self.db.users.values():
            users.append( (user.getUsername(), int(user.getNumericStat(constants.STAT_WINS)), int(user.getNumericStat(constants.STAT_LOSSES)), int(user.getNumericStat(constants.STAT_TIES)), user.rankName) )
        
            
        return s, users, self.rankings.getRankInfo()
    
    def setRankForPlayer(self, username):
        '''
        Set rank for a player
        
        @param username: Username
        '''
        u = util.getUnicode(username)
        r = self.db.users[u].getNumericStat(constants.STAT_RANK)
        rank = self.rankings.getRankByWins(r)
        self.db.users[u].rankName = rank.name
    
    def getUserStatus(self, username):
        '''
        Get users status
        
        @param username:
        @return: ServerMessage detailing users activity on the system
        '''
        p,c = None,None
        for client,player in self.clients.iteritems():
            if player.getUsername() == username:
                p,c = player,client
                break
        
        if p is not None and c is not None:
            message = []
            playing = []
            watching = []
            for game in self.gameList.itervalues():
                if game.hasPlayer(p):
                    playing.append( game.getGameId() )
            
            for game in self.gameList.itervalues():
                if game.hasSpectator(p):
                    watching.append( game.getGameId() )
            
            if len(playing) > 0:
                message.append(PLAYING)
                count = 0
                for game in playing:
                    if count != 0:
                        message.append(',')
                    message.append(game)
                    count += 1
            
            if len(watching) > 0:
                if len(message) > 0:
                    message.append( '-' )
                    
                message.append(WATCHING)
                count = 0
                for game in watching:
                    if count != 0:
                        message.append(',')
                    message.append(game)
                    count += 1
            
            if len(message) == 0:
                message = [ ONLINE ]
            
            return ServerMessage(message)
        else:
            return ServerMessage([OFFLINE])


    def saveGame(self, command, client):
        '''
        Save the game
        
        @param command:
        @param client:
        '''
        game = self.gameList[ command.getGameId() ]
            
        if self.clients[client].getUsername() != game.creator:
            client.gameError(game.getGameId(), ServerMessage([NOT_CREATOR]))
            return
        
        if game.options.has_key(OPTION_TIMED_GAME) or game.options.has_key(OPTION_MOVE_TIME):
            player = game.getCurrentPlayer()
            player.time = player.time - (datetime.datetime.now() - player.stamp)
            player.time = datetime.timedelta(days=player.time.days, seconds=player.time.seconds+1 ) # +1 account for error
        
        game.pause()
        for player in game.getPlayers():
            c = self.getPlayerClient(player)
            c.pauseGame( game.getGameId() )
        self.refreshGameList()
        
        self.db.games[ game.getGameId() ] = game
        self.db.sync()
        self.sendGameInfoMessage(command.getGameId(), [GAME_SAVED], None, level=constants.GAME_LEVEL)
        
        for player in game.getPlayers():
            c = self.getPlayerClient(player)
            c.gameLeave( game.getGameId() )
        
        
        
        
        
        
        
        
        





class ScrabbleServer(NetstringReceiver):
    '''
    Server Protocol.
    
    This class is responsible for shuttling data to and from a client.
    
    There will be one instance of this class per client connected to the ServerFactory
    '''
    
    
    def __init__(self):
        '''
        Constructor
        '''
        
        self.command = helper.CommandCreator()
        self.username = None

    def doDiamondRefresh(self, gameId):
        command = self.command.createGameDiamondRefreshCommand(gameId)
        self.writeCommand(command)
        

    def stringReceived(self, data):
        '''
        Callback when data is received from the client
        
        Parse the data into a Command and handle it.
        
        @param data: Text data representing a command
        @see: L{pyscrabble.command.helper.Command}
        '''
        logger.debug('Incoming: %s %s %s' % (repr(self.username), self.addr.host, data))
        
        command = serialize.loads( data )
        
        if ( isinstance(command, helper.LoginCommand) ):
            self.handleLoginCommand( command )
            return
            
        if ( isinstance(command, helper.ChatCommand) ):
            self.factory.handleChatCommand( command, self )
            return

        if ( isinstance(command, helper.GameCommand) ):
            self.factory.handleGameCommand( command, self )
            return
        
        if ( isinstance(command, helper.PrivateMessageCommand) ):
            self.factory.handlePrivateMessageCommand(command, self)
            return

    def handleLoginCommand(self, command):
        '''
        Handle a login command
        
        Callback to the Factory to authenticate the user
        
        @param command: LoginCommand
        @see: L{pyscrabble.command.helper.LoginCommand}
        '''
        
        
        if (command.getCommand() == constants.LOGIN_INIT):
            
            # Check version
            version = command.getData()
            if (version is '' or version < constants.REQUIRED_VERSION):
                command.setCommand( constants.LOGIN_DENIED )
                command.setData( ServerMessage([REQ_VERSION, constants.REQUIRED_VERSION]) )
            else:
                
                player = Player( command.getUsername() )
                
                if (self.factory.authenticate(command.getUsername(), command.getPassword())):
                    if (self.factory.isLoggedIn(player)):
                        c = self.factory.getPlayerClient(player)
                        if c is not None:
                            self.factory.removeClient(c)
                            c.transport.loseConnection()
                            
                    self.username = command.getUsername()
                    command.setCommand( constants.LOGIN_OK )
                    self.factory.loginUser( player, self )
                else:
                    command.setData( ServerMessage([INVALID_USERNAME_PASSWORD]) )
                    command.setCommand( constants.LOGIN_DENIED )
                
            self.writeCommand( command )

        if (command.getCommand() == constants.NEW_USER):
            self.factory.createNewUser(command, self)
            return

        if (command.getCommand() == constants.CHANGE_PASSWORD):
            self.factory.changePassword(command, self)
            return
            
    def joinChat(self, username):
        '''
        User joins the chatroom
        
        @param username: Username
        '''
        
        command = self.command.createJoinChatCommand(username)
        self.writeCommand( command )

    def sendUserList(self, users):
        '''
        Send List of Players on the server
        
        @param users: List of Players
        @see: L{pyscrabble.game.player.Player}
        '''
        
        command = self.command.createGetChatUsersCommand( users )
        self.writeCommand( command )

    def postChatMessage(self, message):
        '''
        Post a chat message
        
        @param message: Message text
        '''
        
        command = self.command.createPostChatMessageCommand(message)
        self.writeCommand( command )

    def sendLetters(self, gameId, letters):
        '''
        Send Game Letters
        
        @param gameId: Game ID
        @param letters: List of Letters
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        command = self.command.createGetLettersCommand( gameId, letters )
        self.writeCommand( command )
    
    def sendLetterDistribution(self, gameId, distribution):
        '''
        Send letter distribution
        
        @param gameId: GameID
        @param distribution: dict(Letter,count)
        '''
        command = self.command.createGameDistributionCommand( gameId, distribution )
        self.writeCommand( command )
        

    def sendGameList(self, gameList):
        '''
        Send List of Games on the server
        
        @param gameList: List of Games on the server
        @see: L{pyscrabble.game.game.ScrabbleGameInfo}
        '''
        
        command = self.command.createGetGameListCommand( gameList )
        self.writeCommand( command )

    def sendGameUserList(self, gameId, users):
        '''
        Send List of Players in a game
        
        @param gameId: Game ID
        @param users: List of Players
        @see: L{pyscrabble.game.player.Player}
        '''
        
        command = self.command.createGameUserListCommand( gameId, users )
        self.writeCommand( command )

    def denyJoinGame(self, command):
        '''
        Deny the users join game request
        
        @param command: GameCommand
        @see: L{pyscrabble.command.helper.GameCommand}
        '''
        
        self.writeCommand( command )

    def acceptJoinGame(self, command, options):
        '''
        Accept the users join game request
        
        @param command: GameCommand
        @param options: Game options dict
        @see: L{pyscrabble.command.helper.GameCommand}
        '''
        command.setData( options )
        self.writeCommand( command )

    def gameTurnOther(self, gameId, player):
        '''
        Notify the user that it is C{player}'s turn
        
        @param gameId: Game ID
        @param player: Player who has control of the board
        '''
        
        command = self.command.createGameTurnOtherCommand(gameId, player)
        self.writeCommand( command )

    def gameTurnCurrent(self, gameId, time):
        '''
        Notify the user that he/she has control of the board
        
        @param gameId: Game ID
        @param time: Time left
        @see: L{pyscrabble.command.helper.GameCommand}
        '''
        
        command = self.command.createGameTurnCurrentCommand(gameId, time)
        self.writeCommand( command )

    def sendMoves(self, gameId, moves):
        '''
        Notify the user that C{moves} have been posted to a Game
        
        @param gameId: Game ID
        @param moves: List of Moves
        @see: L{pyscrabble.game.pieces.Move}
        '''
        
        command = self.command.createGameSendMoveCommand(gameId, moves)
        self.writeCommand( command )

    def acceptMove(self, gameId):
        '''
        Notify the user that their submitted moves have been accepted
        
        @param gameId: Game ID
        '''
        
        command = self.command.createGameAcceptMoveCommand(gameId)
        self.writeCommand( command )

    def gameError(self, gameId, msg):
        '''
        Notify the user of a Game Error
        
        @param gameId: Game ID
        @param msg: Error message
        @see: L{pyscrabble.command.helper.GameCommand}
        '''
        
        command = self.command.createGameErrorCommand(gameId, msg)
        self.writeCommand( command )
    
    def showError(self, msg):
        '''
        Show a General Error message
        
        @param msg: Error message
        '''
        
        command = self.command.createErrorCommand(msg)
        self.writeCommand( command )
    
    def showInfo(self, msg):
        '''
        Show a General Info message
        
        @param msg: Info message
        '''
        
        command = self.command.createInfoCommand(msg)
        self.writeCommand( command )

    def gameLeave(self, gameId, disableChat = False):
        '''
        Notify the user that they are leaving the game
        
        @param gameId: Game ID
        @see: L{pyscrabble.command.helper.GameCommand}
        '''
        
        command = self.command.createGameLeaveCommand(gameId, disableChat)
        self.writeCommand( command )
    
    def gameBoot(self, gameId):
        '''
        Boot a user from the game
        
        @param gameId: Game ID
        @see: L{pyscrabble.command.helper.GameCommand}
        '''
        
        command = self.command.createGameBootCommand(gameId)
        self.writeCommand( command )
    
    def gameOver(self, gameId):
        '''
        Notify the user the game is over
        
        @param gameId: Game ID
        @see: L{pyscrabble.command.helper.GameCommand}
        '''
        
        command = self.command.createGameOverCommand(gameId)
        self.writeCommand( command )

    def gameInfo(self, gameId, tup):
        '''
        Send a Game Info message
        
        @param gameId: Game ID
        @param tup: A Tuple containing (boolean, message).  If boolean is true, it is a Server info Message.  Else it is a Player info message.
        @see: L{pyscrabble.net.server.ServerFactory.sendGameInfoMessage}
        '''
        
        command = self.command.createGameInfoCommand(gameId, tup)
        self.writeCommand( command )

    def pauseGame(self, gameId):
        '''
        Notify the user that the Game is paused
        
        @param gameId: Game ID
        @see: L{pyscrabble.command.helper.GameCommand}
        '''
        
        command = self.command.createGamePauseCommand(gameId)
        self.writeCommand( command )

    def unPauseGame(self, gameId):
        '''
        Notify the user that the Game is unpaused
        
        @param gameId: Game ID
        '''
        
        command = self.command.createGameUnpauseCommand(gameId)
        self.writeCommand( command )

    def connectionLost(self, reason):
        '''
        This users client has been disconnected.
        
        Remove them from the server
        
        @param reason: Failure
        @see: L{twisted.python.failure.Failure}
        '''
        command = self.command.createLeaveChatCommand()
        command.setData( isinstance(reason.value, error.ConnectionDone) )
        self.factory.handleChatCommand(command, self)

    def logout(self):
        '''
        Log the user out of the game
        
        @see: L{pyscrabble.command.helper.LoginCommand}
        '''
        
        command = self.command.createGoodbyeCommand()
        self.writeCommand( command )
    
    def boot(self):
        '''
        Remove the user from the game
        
        @see: L{pyscrabble.command.helper.LoginCommand}
        '''
        
        command = self.command.createBootedCommand()
        self.writeCommand( command )
    
    def sendPrivateMessage(self, sender, data):
        '''
        Send a private message to this user
        
        @param sender: Username of the sender
        @param data: Message text
        @see: L{pyscrabble.command.helper.PrivateMessageCommand}
        ''' 
        
        command = self.command.createPrivateMessageCommand(sender, '', data)
        self.writeCommand( command )
    
    def setSpectatorChatEnabled(self, gameId, flag):
        '''
        Set Spectator Chat Enabeld
        
        @param gameId: Game ID
        @param flag: True to enable Spectator Chatting
        '''
        command = self.command.createGameSpectatorChatCommand(gameId, flag)
        self.writeCommand( command )
    
    def setSpectatorsAllowed(self, gameId, flag):
        '''
        Set Spectators allowed
        
        @param gameId: Game ID
        @param flag: True to allow Spectators
        '''
        command = self.command.createGameSpectatorSetCommand(gameId, flag)
        self.writeCommand( command )
    
    def sendGameStats(self, gameId, stats):
        '''
        Send Game stats
        
        @param gameId: Game ID
        @param stats: Stats
        '''
        command = self.command.createGameStatsCommand(gameId, stats)
        self.writeCommand( command )
    
    def sendGameOptions(self, gameId, options):
        '''
        Send Game options
        
        @param gameId: Game ID
        @param options: Pptions
        '''
        command = self.command.createGameSendOptionsCommand(gameId, options)
        self.writeCommand( command )
    
    def gameBagEmpty(self, gameId):
        '''
        Notify the client that the game bag is empty
        
        @param gameId: Game ID
        '''
        command = self.command.createGameBagEmptyCommand(gameId)
        self.writeCommand( command )
    
    def gameSendSpectators(self, gameId, list):
        '''
        Send the list of spectators in a game
        
        @param gameId: Game ID
        @param list: List
        '''
        command = self.command.createGameSendSpectatorsCommand(gameId, list)
        self.writeCommand( command )
    
    def sendUserInfo(self, user):
        '''
        Send user info
        
        @param user: Username
        '''
        command = self.command.createUserInfoCommand(user.getUsername(), user)
        self.writeCommand( command )
    
    def sendServerStats(self, stats):
        '''
        Send Server stats
        
        @param stats: Stats
        '''
        command = self.command.createServerStatsCommand(stats)
        self.writeCommand( command )
    
    def sendOfflineMessages(self, messages):
        '''
        Send offline messages
        
        @param messages: List of PrivateMessages
        '''
        command = self.command.createGetMessagesCommand(messages)
        self.writeCommand( command )
        
        
    def writeCommand(self, command):
        '''
        Write the command data to the client
        
        @param command:
        '''
        dumped = serialize.dumps(command)
        x = zlib.compress( dumped )
        logger.debug('Outgoing: %s %s %s' % (self.username, self.addr.host,str(command)))
        self.sendString( x )

    def __repr__(self):
        '''
        String representation of this Protocol.
        "username host"
        '''
        
        return "%s (%s) " % (self.username, self.addr.host)

