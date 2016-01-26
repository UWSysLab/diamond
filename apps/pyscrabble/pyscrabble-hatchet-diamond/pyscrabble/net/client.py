from twisted.internet import reactor, protocol, defer, error
from twisted.protocols.basic import NetstringReceiver
from twisted.python.failure import *
from pyscrabble.command.helper import *
from pyscrabble.constants import *
from pyscrabble import exceptions
from pyscrabble import manager
from pyscrabble import serialize
from pyscrabble import util
import re
import zlib

class ScrabbleClient(object):
    '''
    The ScrabbleClient is basically the "glue" between the GUI and the Server.
    
    It passes requests from the GUI to the Server and callsback to various components in the GUI
    when it receives a Command from the server
    '''

    def __init__(self, host, port, win):
        '''
        Constructor
        
        Initialize the client and connect to the server
        
        @param host: Hostname
        @param port: Port
        @param win: LoginWindow instance
        @see: L{pyscrabble.gui.login.LoginWindow}
        '''
        self.mainWin = win
        self.command = CommandCreator()
        self.loggingOut = False # Set to True when we are logging out, used to ignore the Error that will be raise when the connection is lost
        
        self.errback = self.mainWin.error
        
        c = ConnectionManager(self, host, port)
        self.defer = c.defer
        c.connect()
        
        self.chatWin = None
        self.gameWins = {}
    
    
    # Client commands
    def setChatWindow(self, win):
        '''
        Set the ChatWindow callback
        
        @param win: ChatFrame instance
        @see: L{pyscrabble.gui.chat.ChatFrame}
        '''
        
        self.chatWin = win
        self.chatErrback = self.chatWin.error
    
    def addGameWindow(self, win, gameId):
        '''
        Add a GameFrame callback
        
        @param win: GameFrame instance
        @param gameId: Game ID that the GameFrame holds
        @see: L{pyscrabble.gui.game.GameFrame}
        '''
        
        self.gameWins[gameId] = win
    
    def setMainWindow(self, win):
        '''
        Set the MainWindow callback
        
        @param win: LoginWindow or MainWindow instance
        @see: L{pyscrabble.gui.login.LoginWindow}
        '''
        
        self.mainWin = win
        self.errback = self.mainWin.stopReactor
    
    # Login/Logout Commands
    def login(self, user, password, version):
        '''
        Send the LoginCommand to the server
        
        @param user: Username
        @param password: Password
        '''
        
        command = self.command.createLoginCommand(user, password, version )
        
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.errback)
    
    # Add a new user
    def createNewUser(self, username, password, isAdmin):
        '''
        Create a new user on the server
        
        @param username: Username
        @param password: Password
        @param isAdmin: Is Administrator (True or False)
        '''
        
        command = self.command.createNewUserCommand(username, password, isAdmin)
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.chatErrback)
    
    def logout(self):
        '''
        Initiate the logout process
        '''
        self.defer.addCallbacks(self.doDisconnect, errback=self.errback)
    
    
    # Chat Commands    
    def getUserList(self):
        '''
        Get all users on the server
        '''
        
        command = self.command.createGetChatUsersCommand()
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.chatErrback)  
    
    # Game commands
    def getGameList(self):
        '''
        Get all games on the server
        '''
        
        command = self.command.createGetGameListCommand()
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.chatErrback)
    
    def joinGame(self, gameId):
        '''
        Current user joins game
        
        @param gameId: Game ID to join
        '''
        
        command = self.command.createGameJoinCommand(gameId)
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.chatErrback)
    
    def startGame(self, gameId):
        '''
        Current user starts game
        
        @param gameId: Game ID to start
        '''
        
        command = self.command.createGameStartCommand(gameId)
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.gameWins[gameId].error)
    
    def leaveGame(self, gameId):
        '''
        Current user leaves game
        
        @param gameId: Game ID to leave
        '''
        
        command = self.command.createGameLeaveCommand(gameId)
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.gameWins[gameId].error)
    
    def getLetters(self, gameId, numLetters):
        '''
        Current user gets letters from a game
        
        @param gameId: Active Game ID
        @param numLetters: Number of letters requested
        '''
        
        command = self.command.createGetLettersCommand(gameId, numLetters)
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.gameWins[gameId].error)
        
    def sendMoves(self, gameId, moves, lettersOnBoard):
        '''
        Send moves to the server
        
        @param gameId: Game ID
        @param moves: List of Moves
        @param lettersOnBoard: List of Letters in the Moves
        @see: L{pyscrabble.game.pieces.Letter}
        @see: L{pyscrabble.game.pieces.Move}
        '''
        
        command = self.command.createGameSendMoveCommand(gameId, (lettersOnBoard, moves) )
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.gameWins[gameId].error)
    
    def createGame(self, gameId, options):
        '''
        Current user creates a new game
        
        @param gameId: Game ID
        @param options: Options
        '''
        
        command = self.command.createCreateGameCommand(gameId, options)
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.chatErrback)
    
    def passMove(self, gameId):
        '''
        Current user passes his turn
        
        @param gameId: Game ID
        '''
        
        command = self.command.createGamePassMoveCommand(gameId)
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.gameWins[gameId].error)
    
    def pauseGame(self, gameId):
        '''
        Current user pauses game
        
        @param gameId: Game ID
        '''
        
        command = self.command.createGamePauseCommand(gameId)
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.gameWins[gameId].error)
    
    def unPauseGame(self, gameId):
        '''
        Current user unpauses game
        
        @param gameId: Game ID
        '''
        
        command = self.command.createGameUnpauseCommand(gameId)
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.gameWins[gameId].error)
    
    def tradeLetters(self, gameId, letters):
        '''
        Current user trades letters in a game
        
        @param gameId: Game ID
        @param letters: List of Letters to trade
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        command = self.command.createGameTradeLettersCommand(gameId, letters)
        self.defer.addCallbacks(self.sendCommand, callbackArgs=[command], errback=self.gameWins[gameId].error)


    # Protocol callback
    def defaultCallback(self, data):
        '''
        Callback from the Client Protocol.
        
        This is called whenever we receive data from the server.
        
        Parse the data into a Command object and figure out what to do with it
        
        @param data: Text data that was received from the server
        @see: L{pyscrabble.command.helper.helper.Command}
        '''
        
        
        # Callback to MainWindow, it should always be available.  Errors occur when the connection
        # to the server is lost.
        if (isinstance(data, Failure)):
            if isinstance(data.value, error.ConnectionDone): # Connection closed cleanly
                self.mainWin.stopReactor()
            else:
                self.mainWin.fatalError(util.ErrorMessage("Connection to server has been lost"))
            return
        
        command = serialize.loads(data)
        
        if (isinstance(command, LoginCommand)):
            self.processLoginCommand(command)
            return
        
        if (isinstance(command, ChatCommand)):
            self.processChatCommand(command)
            return
        
        if (isinstance(command, GameCommand)):
            self.processGameCommand(command)
            return
    
    # Protocol response handlers
    def processLoginCommand(self, command):
        '''
        Process a LoginCommand
        
        @param command: LoginCommand
        @see: L{pyscrabble.command.helper.LoginCommand}
        '''
        
        
        if ( command.getCommand() == BOOTED):
            self.mainWin.fatalError(util.ErrorMessage(_("Your account has been removed from the server.")))
            return
        
        if ( command.getCommand() == LOGIN_OK ):
            self.mainWin.loginOK()
            return
        if ( command.getCommand() == LOGOUT ):
            self.mainWin.stopReactor()
            return
        if ( command.getCommand() == LOGIN_DENIED ):
            self.mainWin.error( util.ErrorMessage(command.getData()) )
            return
            
        self.mainWin.error(util.ErrorMessage(command.getData()))
    
    def processChatCommand(self, command):
        '''
        Process a Game Chat command
        
        @param command: ChatCommand
        @see: L{pyscrabble.command.helper.ChatCommand}
        '''
        
        
        if (command.getCommand() == ERROR):
            self.chatWin.error( util.ErrorMessage(command.getData()) )
    
    def processGameCommand(self, command):
        '''
        Process a GameCommand
        
        @param command: GameCommand
        @see: L{pyscrabble.command.helper.GameCommand}
        '''
        
        try:
            if (command.getCommand() == GAME_GET_LETTERS):
                self.gameWins[command.getGameId()].showLetters( command.getData() )
            
            if (command.getCommand() == GAME_LIST):
                self.chatWin.showGameList( command.getData() )
            
            if (command.getCommand() == GAME_USER_LIST):
                self.gameWins[command.getGameId()].refreshUserList( command.getData() )
            
            if (command.getCommand() == GAME_JOIN_OK):
                self.chatWin.newGame( command.getGameId(), False, command.getData() )
            
            if (command.getCommand() == GAME_JOIN_DENIED):
                self.chatWin.error(util.ErrorMessage( command.getData() ), True)
            
            if (command.getCommand() == GAME_TURN_CURRENT):
                #self.gameWins[command.getGameId()].setCurrentTurn( command.getData() )
                print "Get rid of the server code doing this: GAME_TURN_CURRENT called with " + repr(command.getData())
            
            if (command.getCommand() == GAME_TURN_OTHER):
                self.gameWins[command.getGameId()].otherTurn( command.getData() )
            
            if (command.getCommand() == GAME_ERROR):
                self.gameWins[command.getGameId()].error( util.ErrorMessage(command.getData()) )
            
            if (command.getCommand() == GAME_SEND_MOVE):
                #self.gameWins[command.getGameId()].applyMoves( command.getData() )
                print "Get rid of the server code doing this: GAME_SEND_MOVE called with " + repr(command.getData())
            
            if (command.getCommand() == GAME_ACCEPT_MOVE):
                self.gameWins[command.getGameId()].acceptMove()
            
            if (command.getCommand() == GAME_INFO):
                #type, msg = command.getData()
                log = command.getData()
                #self.gameWins[command.getGameId()].info( type,msg )
                self.gameWins[command.getGameId()].info( log )
            
            if (command.getCommand() == GAME_LEAVE):
                self.gameWins[command.getGameId()].leaveGame( None, clientLeaveGame = False, disableChat=command.getData() )
            
            if (command.getCommand() == GAME_BOOT):
                self.gameWins[command.getGameId()].leaveGame( None, clientLeaveGame = True, disableChat=True )
            
            if (command.getCommand() == GAME_PAUSE):
                self.gameWins[command.getGameId()].pauseGame()
            
            if (command.getCommand() == GAME_UNPAUSE):
                self.gameWins[command.getGameId()].unpauseGame()
            
            if (command.getCommand() == GAME_BAG_EMPTY):
                self.gameWins[command.getGameId()].gameBagEmpty()
            
            if (command.getCommand() == GAME_OVER):
                self.gameWins[command.getGameId()].gameOver()
        
        except KeyError: pass
    
    # Protocol command, called by Deferred
    def sendCommand(self, sock, command):
        '''
        Send command data to the server
        
        @param sock: ScrabbleClientProtocol instance
        @param command: Command
        @see: L{pyscrabble.command.helper.Command}
        @return: C{sock}
        '''
        sock.sendData( command )
        return sock
    
    def doDisconnect(self, sock):
        '''
        Disconnect
        
        @param sock: ScrabbleClientProtocol instance
        '''
        sock.disconnect()
        return sock
        

class DefaultProtocol(NetstringReceiver):
    '''
    Protocol instance responsible for the actual sending and receiving of data from the server
    '''
    
    def __init__(self, connectionManager):
        '''
        Constructor
        
        @param connectionManager: ConnectionManager
        '''
        self._manager = connectionManager
        self.defaultCallback = connectionManager.defaultCallback
        
    def sendData(self, data):
        '''
        Send data to the server
        
        @param data: Text
        '''
        self.sendString( serialize.dumps(data) )
    
    def stringReceived(self, data):
        '''
        Callback when a data string is received from the server
        
        Callback to the C{defaultCallback} with the data that was received.
        
        @param data: String received from the server
        '''
        self.defaultCallback( zlib.decompress(data) )
    
    def connectionLost(self, reason):
        '''
        Callback when a connection to the server is lost.
        
        Callback to the C{defaultCallback} with the failture
        
        @param reason: Failure
        @see: {twisted.python.failure.Failure}
        '''
        self.defaultCallback( reason )
    
    def connectionMade(self):
        '''
        Connection made
        '''
        NetstringReceiver.connectionMade(self)
        self._manager.connected(self)
        
    
    def disconnect(self):
        '''
        Disconnect
        '''
        self.transport.loseConnection()


class ConnectionManager(object):
    '''
    ConnectionManager
    '''
    
    def __init__(self, client, host, port):
        '''
        
        @param client:
        @param host:
        @param port:
        '''
        self._client = client
        self.defaultCallback = client.defaultCallback
        self._host = host
        self._port = port
        self.defer = defer.Deferred()
        self._user = self.getProxyUser()
        self._password = self.getProxyPass()
        self._sendCredentials = False
        self._proxyHost = None
        self._proxyPort = None
    
    def getProxyType(self):
        '''
        @return: Proxy type
        '''
        o = manager.OptionManager()
        return o.get_default_option(OPTION_PROXY_TYPE, OPTION_PROXY_HTTP)
    
    def getProxyUser(self):
        '''
        @return: Proxy user name
        '''
        o = manager.OptionManager()
        return o.get_default_option(OPTION_PROXY_USER, None)
    
    def getProxyPass(self):
        '''
        @return: Proxy password
        '''
        o = manager.OptionManager()
        return o.get_default_option(OPTION_PROXY_PASSWORD, None)
    
    def getProxyHost(self):
        '''
        @return: Proxy password
        '''
        o = manager.OptionManager()
        return o.get_default_option(OPTION_PROXY_HOST, '')
    
    def isUsingProxy(self):
        '''
        @return: True if we should use a prox
        '''
        o = manager.OptionManager()
        return o.get_default_bool_option(OPTION_USE_PROXY, False)
        
    def connect(self):
        '''
        Initiate the connection
        '''
        if ( self.isUsingProxy() ):
            self._proxyHost, port = self.getProxyHost().split(':')
            try:
                self._proxyPort = int(port)
            except ValueError:
                self._client.errback(util.ErrorMessage(_("Proxy Host must be: Hostname:Port.")))
                return
             
        if self._user is None or self._password is None:
            self._sendCredentials = True
        self.makeConnection()
    
    def makeConnection(self):
        '''
        Make a connection
        '''
        c = self.get_protocol()
        if self.isUsingProxy() and self._proxyHost is not None:
            d = c.connectTCP(self._proxyHost, self._proxyPort)
        else:
            d = c.connectTCP(self._host, self._port)
            
        d.addErrback( self.errback )
    
    def errback(self, data):
        '''
        Errback
        
        @param data:
        '''
        if isinstance(data, exceptions.ProxyAuthorizationRequiredException):
            if not self._sendCredentials:
                self._sendCredentials = True
                self.makeConnection()
                return
        self._client.errback(util.ErrorMessage(data.getErrorMessage()))
    
    def connected(self, inst):
        '''
        Start running the callbacks, we've got a live connection
        
        @param inst:
        '''
        self.defer.callback(inst)
    
    def get_protocol(self):
        '''
        Get the appropriate protocol
        '''
        o = manager.OptionManager()
        if ( self.isUsingProxy() ):
            type = self.getProxyType()
            if (type == OPTION_PROXY_HTTP):
                if not self._sendCredentials:
                    return protocol.ClientCreator(reactor, HttpProxyProtocol, self, self._host, self._port, None, None)
                else:
                    return protocol.ClientCreator(reactor, HttpProxyProtocol, self, self._host, self._port, self._user, self._password)
            else:
                self._client.errback(util.ErrorMessage(_("Invalid proxy type")))
        
        return protocol.ClientCreator(reactor, DefaultProtocol, self)

class HttpProxyProtocol(DefaultProtocol):
    '''
    Protocol to connect through an HTTP Proxy
    '''
    def __init__(self, connectionManager, host, port, user, password):
        '''
        
        @param connectionManager:
        @param host:
        @param port:
        @param user:
        @param password:
        '''
        DefaultProtocol.__init__(self, connectionManager)
        self._host = host
        self._port = port
        self._user = user
        self._password = password
        self._manager = connectionManager
        self._errback = connectionManager.errback
        self._proxyConnected = False
    
    def connectionLost(self, reason):
        '''
        Callback when a connection to the server is lost.
        
        If we're not connected to the server, ignore it
        
        @param reason: Failure
        @see: {twisted.python.failure.Failure}
        '''
        if self._proxyConnected:
            DefaultProtocol.connectionLost( self,reason )
    
    def connectionMade(self):
        '''
        Connection made, send the proxy request
        '''
        if self._user is None and self._password is None:
            s = 'CONNECT %s:%d HTTP/1.0\r\n\r\n' % (self._host, self._port)
        else:
            s = util.b64encode('%s:%s' % (self._user, self._password))
            s = 'CONNECT %s:%d HTTP/1.0\r\nProxy-Authorization: Basic %s \r\n\r\n' % (self._host, self._port, s)
        self.transport.write(s)
    
    def dataReceived(self, data):
        '''
        Data received handler
        
        @param data: Incoming data
        '''
        if not self._proxyConnected:
            code = self.getReponseCode(data)
            if code == 200:
                self._proxyConnected = True
                self._manager.connected(self)
            if code == 407:
                type, realm = self.getAuthMethod(data)
                self._errback( exceptions.ProxyAuthorizationRequiredException(type,realm) )
        else:
            DefaultProtocol.dataReceived(self, data)
    
    def getReponseCode(self, data):
        '''
        Get proxy http response code
        @param data: Response data
        @return: response code
        '''
        return int( data.split()[1] )
    
    def getAuthMethod(self, data):
        '''
        Get proxy http response code
        @param data: Response data
        @return: tuple of auth-type,auth-realm
        '''
        lines = data.split('\r\n')
        for line in lines:
            if line.startswith('Proxy-Authenticate'):
                pattern = re.compile('Proxy-Authenticate: (.*) realm=\"(.*)\"')
                match = pattern.match(line)
                if match:
                    type, match = match.group(1), match.group(2)
                return type, match
                
    