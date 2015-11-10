from twisted.internet import protocol, reactor, error
from twisted.protocols.basic import NetstringReceiver
from pyscrabble.command import helper
from pyscrabble.game.player import Player, User,PlayerInfo
from pyscrabble.game.game import ScrabbleGame, ScrabbleGameInfo
from pyscrabble.lookup import *
from pyscrabble.game import rank
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
        
    
    def getUsers(self):
        '''
        Return user objects
        
        @return: Dictionary of users
        '''
        x = self.db.users.values()[:]
        x.sort( lambda x,y : cmp(x.username, y.username) )
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
                
        for player in game.getPlayers():
            c = self.getPlayerClient(player)
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
        
    def loginUser(self, player, client):
        '''
        Log a user into the system. Join the main chat
        
        @param player:
        @param client:
        '''
        
        self.clients[client] = player
        
        if len(self.clients) > self.maxUsersLoggedIn:
            self.maxUsersLoggedIn = len(self.clients)
        

    def handleGameCommand(self, command, client):
        '''
        Handle a game command
        
        @param command: GameCommand
        @param client: ScrabbleServer Protocol
        '''
        
        
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
            self.doGameTurn(game.getGameId(), wasUnpaused=True)
            
            del self.db.games[ game.getGameId() ]
            self.db.sync()

        if (command.getCommand() == constants.GAME_TRADE_LETTERS):
            self.tradeLetters(command, client)
            
    def handleChatCommand(self, command, client):
        print "Received chat command: " + repr(command.command) + " " + repr(command.data)
    
    # Send the list of users to the client
    def sendUserList(self, client):
        '''
        Send the client a list of all users on the server
        
        @param client:
        '''
        
        client.sendUserList( [ self.clients[c] for c in self.clients.keys()] )

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
        client.acceptJoinGame( command, {} )
        
        #players = game.getPlayers()
        #pending = game.getPending()
        
        self.sendGameScores(game.getGameId())
        
        client.sendMoves( game.getGameId(), game.getMoves() )
        
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
            
        for player in game.getPlayers():
            c = self.getPlayerClient(player)
            letters = game.getLetters( player.getNumberOfLettersNeeded() )
            player.addLetters(letters)
            c.sendLetters( game.getGameId(), letters )
        
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
        
        client = self.getPlayerClient(player)
        
        time = datetime.timedelta(seconds=0)
            
        client.gameTurnCurrent(gameId, time)
        
        for _player in game.getPlayers():
            _client = self.getPlayerClient(_player)
            if (_player != player):
                _client.gameTurnOther( gameId, PlayerInfo(player.getUsername(), player.getScore(), len(player.getLetters()), time ))
    
    def sendLetterDistribution(self, gameId):
        '''
        Send the letter distribution
        
        @param gameId: Game ID
        '''
        game = self.gameList[ gameId ]
        
        for p in game.getPlayers():
            c = self.getPlayerClient(p)
            c.sendLetterDistribution( gameId, game.getDistribution() )

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
        
        game.playerLeave(player)
        
        self.sendGameScores(gameId)

        # If there are no more players left, remove the game
        if len(game.getPlayers()) == 0 and not game.isPaused():
            #for s in game.getSpectators():
            #    c = self.getPlayerClient( s )
            #    c.gameLeave( game.getGameId() )
            del self.gameList[ gameId ]
        elif not game.isComplete() and not game.isPaused():
            if game.isCurrentPlayer( player ):
                self.doGameTurn( gameId )

        self.refreshGameList()
        

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
            if word not in self.dicts[ 'en' ]:
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
        
        # If the player used all 7 of his/her letters, give them an extra 50
        if onboard.length() == 7:
            player.addScore( constants.BINGO_BONUS_SCORE )
        
        for p in game.getPlayers():
            c = self.getPlayerClient(p)
            c.sendMoves( gameId, moves )
            
            # If the player used all his/her letters and there are no more letters in the bag, the game is over
        if (len(player.getLetters()) == 0 and game.isBagEmpty()):
            
            # Subtract everyones letter points
            # Give points to the person who went out
            players = game.getPlayers()
            for p in players:
                if p == player: 
                    continue # Skip winner
                    
                letters = p.getLetters()
                for letter in letters:
                    p.addScore( letter.getScore() * -1 )
                    player.addScore( letter.getScore() )
            
            self.sendGameScores(game.getGameId())
            
            self.gameOver(game)
            return

        letters = game.getLetters( player.getNumberOfLettersNeeded() )
        if (len(letters) > 0):
            player.addLetters(letters)
            client.sendLetters(gameId, player.getLetters())
        
        self.sendGameScores(game.getGameId())
        
        if game.isBagEmpty() or game.getCountLetters() < 7:
            for p in game.getPlayers():
                c = self.getPlayerClient(p)
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
            game.passMove()
            
            self.sendGameScores(gameId)
            
            self.doGameTurn(gameId)
        except exceptions.GameOverException:
            # If everyone has passed, assume that everyone still has letters
            # Subtract everyones letter points
            players = game.getPlayers()
            for player in players:
                letters = player.getLetters()
                for letter in letters:
                    player.addScore( letter.getScore() * -1 )
                
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
            
        winners = game.getWinners(exclude)
        
        if len(winners) > 0:
            newdisp = '%s by ' % (winners[0].getScore())
            for w in winners:
                newdisp = newdisp + w.getUsername() + ' '
        
        winners = game.getWinners(exclude)
        
        # If there is less than one player in the game, don't count the score
        count = len(game.getPlayers()) > 1
        
        for p in game.getPlayers():
            c = self.getPlayerClient( p )
            c.gameOver( game.getGameId() )
        
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
        
        self.sendGameScores(command.getGameId())
                
        self.doGameTurn(game.getGameId())


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
        
        game.pause()
        for player in game.getPlayers():
            c = self.getPlayerClient(player)
            c.pauseGame( game.getGameId() )
        self.refreshGameList()
        
        self.db.games[ game.getGameId() ] = game
        self.db.sync()
        
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

    def sendUserList(self, users):
        '''
        Send List of Players on the server
        
        @param users: List of Players
        @see: L{pyscrabble.game.player.Player}
        '''
        
        command = self.command.createGetChatUsersCommand( users )
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
        pass

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
    
    def gameBagEmpty(self, gameId):
        '''
        Notify the client that the game bag is empty
        
        @param gameId: Game ID
        '''
        command = self.command.createGameBagEmptyCommand(gameId)
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

