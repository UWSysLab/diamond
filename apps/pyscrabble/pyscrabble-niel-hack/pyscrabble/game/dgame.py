from pyscrabble.game.pieces import Bag, Letter
from pyscrabble.exceptions import GameOverException, BagEmptyException
from pyscrabble.constants import *
from pyscrabble.lookup import *
from random import shuffle
from pyscrabble.game.dplayer import DPlayer
from pyscrabble import util
from pyscrabble import constants
from pyscrabble import manager

import sys
sys.path.append("/Users/Niel/systems/diamond-src/backend/build/src/bindings/python")
sys.path.append("/home/nl35/research/diamond-src/backend/build/src/bindings/python")
from libpydiamond import *

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
         
        #self.players = []
        self.name = name
        self.started = False
        self.paused = False
        self.complete = False
        self.moves = []
        self.words = []
        #self.usedModifiers = []
        self.passedMoves = 0
        #self.currentPlayer = ""
        self.spectators = []
        self.spectatorChatEnabled = True
        self.log = []
        self.pending = [] #Now holds username strings
        self.stats = {}
        self.options = options
        #self.bag = Bag( "en" )
        self.creator = None
        self.timer = None
        self.spectatorsAllowed = True
        
        self.players = DStringList()
        self.currentPlayer = DString()
        self.turnNumber = DCounter()
        self.usedModifiersX = DList()
        self.usedModifiersY = DList()
        
        keyPrefix = "game:" + self.name
        DStringList.Map(self.players, keyPrefix + ":players")
        DString.Map(self.currentPlayer, keyPrefix + ":currentplayer")
        DCounter.Map(self.turnNumber, keyPrefix + ":turnnumber")
        DList.Map(self.usedModifiersX, keyPrefix + ":usedmodifiersx")
        DList.Map(self.usedModifiersY, keyPrefix + ":usedmodifiersy")
        
        self.bag = DBag(self.name)

        
    def resetGame(self):
        self.players.Clear()
        self.currentPlayer.Set("")
        self.turnNumber.Set(0)
        self.usedModifiersX.Clear()
        self.usedModifiersY.Clear()
        
        self.bag.reset()
        
    def getTurnNumber(self):
        return self.turnNumber.Value()
     
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
        self.usedModifiersX.Append(pos[0])
        self.usedModifiersY.Append(pos[1])
    
    def hasUsedModifier(self, pos):
        '''
        Check to see if we've used a modifier
        
        @param pos: (x,y) position
        '''
        for i in range(0, len(self.usedModifiersX.Members())):
            if pos[0] == self.usedModifiersX.Value(i) and pos[1] == self.usedModifiersY.Value(i):
                return True
        return False
        
    
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
        
        self.players.Append( username )
        player = DPlayer(username)
        player.reset()
    
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
        
        return len(self.players.Members())
    
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
        
        #TODO: figure out how to shuffle DList
        #shuffle(self.players)
    
    def getCurrentPlayer(self):
        '''
        Get current player
        
        @return: Player who has control of the board.
        @see: L{pyscrabble.game.player.Player}
        '''
        return DPlayer(self.currentPlayer.Value())
        
    
    def getNextPlayer(self):
        '''
        Get the next player who has control of the board.
        
        @return: Next Player who has control of the board.
        @see: L{pyscrabble.game.player.Player}
        '''
        
        if (len(self.players.Members()) == 0):
            return None
        
        self.currentPlayer.Set(self.players.Value(0))
        self.players.Erase(0)
        self.players.Append(self.currentPlayer.Value())
        self.turnNumber.Set(self.turnNumber.Value() + 1)
        return DPlayer(self.currentPlayer.Value())
    
    def getPlayers(self):
        '''
        Return a copy of the list of Players
        
        @return: List of Players
        @see: L{pyscrabble.game.player.Player}
        '''
        
        #return self.players[:]
        result = []
        for username in self.players.Members():
            result.append(DPlayer(username))
        return result
    
    def hasPlayer(self, username):
        '''
        Check to see if a player is in the game
        
        @param username: string
        @return: True if player with the given username is in the game
        '''
        
        return (self.players.Index(username) >= 0)
    
    def playerLeave(self, username):
        '''
        Remove a player from the game and return his/her Letters to the bag
        
        @param username: Username of player to remove
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        self.players.Remove(username)
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
        if (self.passedMoves == len(self.players.Members())):
            raise GameOverException
    
    def getWinners(self, exclude=None):
        '''
        Return the player(s) with the highest score
        
        @param exclude: Player to exclude from winning (say, if they timed out)
        @return: List of players with the highest score.
        @see: L{pyscrabble.game.player.Player}
        '''
        
        tmp = self.players.Members()[:]
        
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
        
        self.players.Remove( self.currentPlayer.Value() )
        self.players.Insert(0, self.currentPlayer.Value() )
        
        for player in self.players.Members():
            if player not in self.pending:
                self.pending.Append( player )
        
    
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
    
#     def isCurrentPlayer(self, player):
#         '''
#         Check to see if C{player} is the current player
#         
#         @param player: Player to checl
#         @return: True if C{player} is the current player.
#         '''
#         return self.currentPlayer == player
    
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
   
    
    def getMovesScore(self, moves):
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
                if m in constants.LETTER_MODIFIERS and not self.hasUsedModifier((x,y)):
                    score = score + (m * letter.getScore())
                else:
                    score = score + letter.getScore()
    
                if (m >= modifier and not self.hasUsedModifier((x,y))):
                    modifier = m
                    if m in constants.WORD_MODIFIERS:
                        apply = apply + 1
                    m_x = x
                    m_y = y

            if modifier in constants.WORD_MODIFIERS and not self.hasUsedModifier((m_x,m_y)):
                
                if util.isCenter(m_x, m_y):
#                     if self.options[OPTION_CENTER_TILE]:
#                         score = score * (modifier/2)
                    score = score * (modifier/2)
                else:
                    score = score * ((modifier/2) ** apply)
                
            move.setScore( score )
            total = total + score
        
        return total

    def removeModifiers(self, moves):
        '''
        Mark off modifiers that are used in this move
        
        @param moves: List of moves
        '''
        for move in moves:
            for letter,x,y in move.getTiles():
                m = util.getTileModifier(x,y)
                if m in constants.LETTER_MODIFIERS and not self.hasUsedModifier((x,y)):
                    self.addUsedModifier( (x,y) )
                if m in constants.WORD_MODIFIERS and not self.hasUsedModifier((x,y)):
                    self.addUsedModifier( (x,y) )

                    
class DBag:
    
    def __init__(self, gameId, rules="en"):
        '''
        Initialize the Letter bag
        
        @see: L{Letter}
        '''
        self.rules = rules
        
        self.letterStrs = DStringList()
        self.letterScores = DList()
        self.gameId = gameId
        
        keyPrefix = self.gameId + ":bag"
        DStringList.Map(self.letterStrs, keyPrefix + ":letterstrs")
        DList.Map(self.letterScores, keyPrefix + ":letterscores")

    def reset(self):
        self.letterStrs.Clear()
        self.letterScores.Clear()
        
        letters = []
        l = manager.LettersManager()
        for letter,count,score in l.getLetters(self.rules):
            for x in range(count):
                letters.append( Letter(letter, score) )
        
        # Shuffle the letters in the bag
        shuffle(letters)
        
        for letter in letters:
            self.letterStrs.Append(letter.getLetter().encode("utf-8"))
            self.letterScores.Append(letter.getScore())
    
    def getDistribution(self):
        return []
    
    def getLetters(self, count = 7):
        '''
        Get C{count} number of letters from the bag. If C{count} is greater than the number of Letters
        left in the Bag, the remaining number of Letters are returned
        
        @param count: Number of letters to get
        @raise BagEmptyException: If the Bag is empty.
        @see: L{Letter}
        '''
        
        if self.isEmpty():
            raise BagEmptyException()
        
        # Take "count" number letters from bag, or remaining number of letters
        if (self.getCount() < count):
            count = self.getCount()
            
        result = []
        for x in range(count):
            letter = Letter(self.letterStrs.Value(0), self.letterScores.Value(0))
            self.letterStrs.Erase(0)
            self.letterScores.Erase(0)
            result.append(letter)
        return result
            
    def isEmpty(self):
        '''
        Check to see if the Bag is empty
        
        @return: True if the Bag is empty
        '''
        
        return (self.getCount() == 0)
    
    def getCount(self):
        '''
        Get the number of Letters in the Bag
        
        @return: Number of letters in the Bag
        '''
        
        return len( self.letterStrs.Members() )
    
    def returnLetters(self, letters):
        '''
        Return a list of Letters to the Bag.
        
        @param letters: List of Letters to return to the Bag
        @see: L{Letter}
        '''
        
        for i in range(0, len(self.letterStrs.Members())):
            letters.append(Letter(self.letterStrs.Value(i), self.letterScores.Value(i)))
        
        shuffle(self.letters)
        
        self.letterStrs.Clear()
        self.letterScores.Clear()
        
        for letter in letters:
            self.letterStrs.Append( letter.getLetter() )
            self.letterScores.Append( letter.getScore())