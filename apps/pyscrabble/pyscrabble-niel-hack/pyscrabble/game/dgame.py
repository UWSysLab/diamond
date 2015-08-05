from pyscrabble.game.pieces import Bag
from pyscrabble.game.dplayer import DPlayer
from pyscrabble.exceptions import GameOverException, BagEmptyException
from pyscrabble.constants import *
from pyscrabble.lookup import *
from random import shuffle


class DScrabbleGame:
    '''
    ScrabbleGame class.  Keeps track of the players in the game, the letters and the moves played.
    '''
    
    def __init__(self, name = '', options=None):
        '''
        Initialize a new game.
        
        @param name: Game ID
        '''
        
        if not options:
            options = {}
         
        self.players = [] #Now holds Username strings
        self.name = name
        self.started = False
        self.paused = False
        self.complete = False
        self.moves = []
        self.words = []
        self.usedModifiers = []
        self.passedMoves = 0
        self.currentPlayer = "" #Now holds Username string
        self.spectators = []
        self.spectatorChatEnabled = True
        self.log = []
        self.pending = [] #Now holds Username strings
        self.stats = {}
        self.options = options
        self.bag = Bag( "en" )
        self.creator = None
        self.timer = None
        self.spectatorsAllowed = True
     
    def getDistribution(self):
        '''
        Get Letter distribution
        
        @return: dict(Letter,Count)
        '''
        
        return self.bag.getDistribution()
    
    def isBagEmpty(self):
        '''
        Check if the letter bag is empty
        
        @return: True if the Bag is empty
        '''
        return self.bag.isEmpty()
        
    
    def addUsedModifier(self, pos):
        '''
        Add a used modifier to the list.
        
        These are word or letter modifiers that we have already used.
        
        @param pos: (x,y) position
        '''
        self.usedModifiers.append(pos)
    
    def hasUsedModifier(self, pos):
        '''
        Check to see if we've used a modifier
        
        @param pos: (x,y) position
        '''
        
        return pos in self.usedModifiers;
        
    
    def getLetters(self, numLetters):
        '''
        Get letters from the bag
        
        @param numLetters: Number of letters to get
        @return: List of Letters or [] if there or no letters left
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        try:
            return self.bag.getLetters( numLetters )
        except BagEmptyException: pass
        
        return []
    
    def addPlayer(self, username):
        '''
        Add a player to the game
        
        @param username: Username of player to add to the game
        '''
        
        self.players.append( username )
    
    def getGameId(self):
        '''
        Get the Game ID
        
        @return: Game ID of this game
        '''
        
        return self.name
    
    def getNumberOfPlayers(self):
        '''
        Get number of players in the game
        
        @return: Number of players in the game
        '''
        
        return len(self.players)
    
    def isStarted(self):
        '''
        Check if the game is started.
        
        @return: True if the game is started and unpaused.
        '''
        
        if (self.isPaused()):
            return False
            
        return self.started
    
    def isComplete(self):
        '''
        Check if the game is complete.  The game is complete when all players cannot make a move and have to pass.
        
        @return: True if the game is complete.  
        '''
        
        return self.complete
    
    def start(self):
        '''
        Start the game.
        '''
        
        self.started = True
        
        shuffle(self.players)
    
    def getCurrentPlayer(self):
        '''
        Get current player
        
        @return: Player who has control of the board.
        @see: L{pyscrabble.game.player.Player}
        '''
        return DPlayer(self.currentPlayer)
        
    
    def getNextPlayer(self):
        '''
        Get the next player who has control of the board.
        
        @return: Next Player who has control of the board.
        @see: L{pyscrabble.game.player.Player}
        '''
        
        if (len(self.players) == 0):
            return None
        
        self.currentPlayer = self.players.pop(0)
        self.players.append(self.currentPlayer)
        return DPlayer(self.currentPlayer)
    
    def getPlayers(self):
        '''
        Return a copy of the list of Players
        
        @return: List of Players
        @see: L{pyscrabble.game.player.Player}
        '''
        
        #return self.players[:]
        result = []
        for username in self.players:
            result.append(DPlayer(username))
        return result
    
    def hasPlayer(self, username):
        '''
        Check to see if a player is in the game
        
        @param username: string
        @return: True if player with the given username is in the game
        '''
        
        return username in self.players
    
    def playerLeave(self, username):
        '''
        Remove a player from the game and return his/her Letters to the bag
        
        @param username: Username of player to remove
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        self.players.remove(username)
        self.returnLetters( DPlayer(username).getLetters() )
    
    def addMoves(self, moves, player):
        '''
        Add a move to the board
        
        @param moves: Moves to add to the board
        @param player: Player who added the move
        @see: L{pyscrabble.game.pieces.Move}
        '''
        
        self.passedMoves = 0 # Rest pass counter
        score = 0
        allDisp = ''
        for move in moves:
            word = move.getWord()
            score = score + move.getScore()
            self.words.append( word )
            self.moves.append( move )
            
            newdisp = '%s (%s) by %s' % (word, str(move.getScore()), player.getUsername())
            if self.stats.has_key(STAT_HIGHEST_SCORING_WORD):
                data,disp = self.stats[STAT_HIGHEST_SCORING_WORD]
                if (move.getScore() > data.getScore()):
                    self.stats[STAT_HIGHEST_SCORING_WORD] = move,newdisp
            else:
                self.stats[STAT_HIGHEST_SCORING_WORD] = move,newdisp
            
            newdisp = '%s (%s) by %s' % (word, str(move.length()), player.getUsername())
            if self.stats.has_key(STAT_LONGEST_WORD):
                data,disp = self.stats[STAT_LONGEST_WORD]
                if (move.length() > data.length()):
                    self.stats[STAT_LONGEST_WORD] = move,newdisp
            else:
                self.stats[STAT_LONGEST_WORD] = move,newdisp
            
            allDisp = allDisp + '%s (%s) ' % (word, str(move.getScore()))
        
        allDisp = allDisp + 'by %s' % player.getUsername()
        
        d = allDisp
        d = '%s, ' % str(score) + d
        if self.stats.has_key(STAT_HIGHEST_SCORING_MOVE):
            data, disp = self.stats[STAT_HIGHEST_SCORING_MOVE]
            if score > data:
                self.stats[STAT_HIGHEST_SCORING_MOVE] = score, d
        else:
            self.stats[STAT_HIGHEST_SCORING_MOVE] = score, d
        
        d = allDisp
        d = '%s, ' % str(len(moves)) + d
        if self.stats.has_key(STAT_MOST_WORDS_IN_MOVE):
            data, disp = self.stats[STAT_MOST_WORDS_IN_MOVE]
            if len(moves) > data:
                self.stats[STAT_MOST_WORDS_IN_MOVE] = len(moves), d
        else:
            self.stats[STAT_MOST_WORDS_IN_MOVE] = len(moves), d
        
    
    
    def getMoves(self):
        '''
        Return List of moves on the board
        
        @return: List of moves on the board
        @see: L{pyscrabble.game.pieces.Move}
        '''
        
        return self.moves
    
    def hasWord(self, word):
        '''
        Check to see if a word is on the board
        
        @param word: Word to check
        @return: True if the word is on the board.
        '''
        
        return word in self.words
    
    def resetPassCount(self):
        '''
        Reset the pass counter
        '''
        self.passedMoves = 0
        
    
    def passMove(self):
        '''
        Pass a move.
        
        If the number of passed moves is equal to the number of players, all players have passed their moves
        and the game is over.
        
        @raise GameOverException: If all players have passed their moves and the game is over.
        '''
        
        self.passedMoves = self.passedMoves + 1
        if (self.passedMoves == len(self.players)):
            raise GameOverException
    
    def getWinners(self, exclude=None):
        '''
        Return the player(s) with the highest score
        
        @param exclude: Player to exclude from winning (say, if they timed out)
        @return: List of players with the highest score.
        @see: L{pyscrabble.game.player.Player}
        '''
        
        tmp = self.players[:]
        
        if exclude is not None:
            tmp.remove(exclude)
        
        maxScore = None
        for username in tmp:
            maxScore = max(maxScore, DPlayer(username).getScore())
        
        winners = []
        for username in tmp:
            player = DPlayer(username)
            if player.getScore() == maxScore:
                winners.append( player )
        return winners
    
    def pause(self):
        '''
        Pause the game.
        '''
        
        self.paused = True
        
        self.players.remove( self.currentPlayer )
        self.players.insert(0, self.currentPlayer )
        
        for player in self.players:
            if player not in self.pending:
                self.pending.append( player )
        
    
    def unPause(self):
        '''
        Unpause the game.
        '''
        
        self.paused = False
        self.pending = []
    
    def isPaused(self):
        '''
        Check whether the game is paused.
        
        @return: True if the game is paused.
        '''
        
        return self.paused
    
    def returnLetters(self, letters):
        '''
        Return letters to the Bag.
        
        @param letters: Letters to put back in the back
        @see: L{pyscrabble.game.pieces.Letter}
        @see: L{pyscrabble.game.pieces.Bag}
        '''
        
        self.bag.returnLetters(letters)
    
    def isInProgress(self):
        '''
        Check whether the game is started.  The game is considered in progress even if it is paused.
        
        @return: True if the game has been started.
        '''
        
        return self.started
    
    def getPlayer(self, username):
        '''
        Get a player out of the game
        
        @param username: Username of player
        @return: Player in the game, or None if player is not in the game.
        @see: L{pyscrabble.game.player.Player}
        '''
        
        for _username in self.players:
            if username == _username:
                return DPlayer(_username)
        return None
    
    def isCurrentPlayer(self, player):
        '''
        Check to see if C{player} is the current player
        
        @param player: Player to checl
        @return: True if C{player} is the current player.
        '''
        return self.currentPlayer == player
    
    def addSpectator(self, spectator):
        '''
        Add a spectator to the game
        
        @param spectator: Spectator name
        '''
        self.spectators.append(spectator)
    
    def getSpectators(self):
        '''
        Return a copy of the list of spectors
        
        @return: List of Spectator names
        '''
        
        return self.spectators[:]
    
    def spectatorLeave(self, spectator):
        '''
        Remove a spectator from the game
        
        @param spectator: Spectator to remove
        '''
        self.spectators.remove(spectator)
    
    def hasSpectator(self, spectator):
        '''
        Check to see if a spectator is in the game
        
        @param spectator: Spectator
        @return: True if spectator is in the game
        '''
        
        return spectator in self.spectators
    
    def isSpectatorChatEnabled(self):
        '''
        Check if Spectator Chat is Enabled
        
        @return: True if Spectator Chat is enabled for this game
        '''
        return self.spectatorChatEnabled
    
    def setSpectatorChatEnabled(self, flag):
        '''
        Set Spectator Chat Enabled/Disabled
        
        @param flag: True to enable Spectator Chat
        '''
        self.spectatorChatEnabled = flag
    
    def isSpectatorsAllowed(self):
        '''
        Check if Spectator Chat is Enabled
        
        @return: True if Spectator Chat is enabled for this game
        '''
        return self.spectatorsAllowed
    
    def setSpectatorsAllowed(self, flag):
        '''
        Set Spectators Allowed
        
        @param flag: True to allow spectators
        '''
        self.spectatorsAllowed = flag
    
    def setComplete(self):
        '''
        Mark this game as complete
        '''
        self.complete = True
    
    def appendLogMessage(self, log):
        '''
        Add logging information
        
        Log is a tuple: (level, message)
        
        @param log: Log Message tuple
        '''
        self.log.append(log)
    
    def getLog(self):
        '''
        Retrieve log
        
        Log is a list of tuples: (level, message)
        
        @return: Game Log
        '''
        return self.log[:]
    
    def getPending(self):
        '''
        Get list of players pending to join the game
        
        @return: List of players still pending to re-join a paused game
        '''
        return self.pending
        
    def removePending(self, player):
        '''
        Remove a player from the pending list
        
        @param player: Player
        '''
        self.pending.remove( player )
    
    def addPending(self, player):
        '''
        Add pending player
        
        @param player: Player
        '''
        if player not in self.pending:
            self.pending.append( player )
    
    def getStats(self):
        '''
        Retrieve list of game stats
        
        @return: List of tuples (stat_name, stat_value)
        '''
        
        s = []
        s.append( (ServerMessage([STAT_LETTERS_LEFT]), str(self.bag.getCount())) )
        
        for key,value in self.stats.iteritems():
            data,disp = value
            s.append ( (ServerMessage([key]), disp) )
            
        return s
    
    def getOptions(self):
        '''
        Return list of options
        
        @return: List of tuples (option_name, option_value)
        '''
        
        s = []
        s.append( (ServerMessage([CREATOR]), self.creator) )
        
        for key,value in self.options.iteritems():
            if key in (OPTION_TIMED_GAME , OPTION_TIMED_LIMIT, OPTION_MOVE_TIME):
                if value == int(1):
                    s.append( (ServerMessage([key]), ServerMessage([value,MINUTE])) )
                else:
                    s.append( (ServerMessage([key]), ServerMessage([value,MINUTES])) )
            else:
                s.append( (ServerMessage([key]), ServerMessage([value])) )
        return s
        
    
    def getCountLetters(self):
        '''
        Return the number of letters left in the bag
        
        @return: Count of letters left in bag
        '''
        return self.bag.getCount()    