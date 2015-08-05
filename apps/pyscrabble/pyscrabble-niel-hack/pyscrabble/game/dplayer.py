import datetime
import time
from pyscrabble import constants
from pyscrabble import util
from pyscrabble.game.pieces import Letter

import sys
sys.path.append("/Users/Niel/systems/diamond-src/backend/build/src/bindings/python")
sys.path.append("/home/nl35/research/diamond-src/backend/build/src/bindings/python")
from libpydiamond import *

class DPlayer(object):
    '''
    Player class.
    
    A Player in the game.
    '''
    
    def __init__(self, username=''):
        '''
        Initialize the player
        
        @param username: Username of Player
        '''
        
        #self.username = username
        #self.score = 0
        #self.letters = []
        #self.u_time = None
        
        self.letterStrs = DList()
        self.letterScores = DList()
        self.score = DLong()
        self.username = DString()
        
        username = username.encode("utf-8")
        DList.Map(self.letterStrs, "player:" + username + ":letterslist")
        DList.Map(self.letterScores, "player:" + username + ":scoreslist")
        DLong.Map(self.score, "player:" + username + ":score")
        DString.Map(self.username, "player:" + username + ":username")
        
        self.username.Set(username)

    
    def setInitialTime(self, minutes):
        '''
        Set initial time
        
        @param minutes: Number of minutes
        '''
        self.time = datetime.timedelta(minutes=minutes)
    
    def addScore(self, score):
        '''
        Add C{score} to this players score.
        
        @param score: Score to add to Player's score.
        '''
        
        self.score.Set(self.score.Value() + score)
    
    def getScore(self):
        '''
        
        @return: Player's score
        '''
        
        return self.score.Value()
    
    def getUsername(self):
        '''
        
        @return: Player's username
        '''
        
        return util.getUnicode(self.username.Value())
        
    def addLetters(self, letters):
        '''
        Add Letters to this Player's letterbox
        
        @param letters: List of Letters to add.
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        #self.letters.extend( letters )
        for letter in letters:
            self.letterStrs.append(letter.getLetter())
            self.letterScores.append(letter.getScore())
    
    def getNumberOfLettersNeeded(self):
        '''
        Calculate the number of Letters need to make sure this player has 7 Letters
        
        @return: Number of Letters need to fill the Players letterbox
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        #return 7 - len(self.letters)
        return 7 - len(self.letterStrs)
    
    def removeLetters(self, list):
        '''
        Remove Letters from this Player's letterbox
        
        @param list: List of Letters to remove.
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
#         for letter in list:
#             if (letter.isBlank()):
#                 letter.setLetter("")
#             self.letters.remove( letter )
        for letter in list:
            index = self.letterStrs.index(letter.getLetter())
            del self.letterStrs[index]
            del self.letterScores[index]
        
    def getLetters(self):
        '''
        
        @return: List of Players Letters
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        letters = []
        for i in range(0, len(self.letterStrs)):
            letters.append(Letter(self.letterStrs[i], self.letterScores[i])) 
        return self.letters
    
    def reset(self):
        '''
        Reset this Player.
        
        Set the Player' score to zero.
        Remove all Letters from the Player's letterbox.
        '''
        
        self.score.Set(0)
        #self.letters = []
        self.letterStrs.Clear()
        self.letterScores.Clear()
    
    def __eq__(self, other):
        '''
        Check if this player equals another. Tests to see if the usernames are equals
        
        @param other: Other player to test.
        @return: True if this Player equals C{other}
        '''
        
        if (isinstance(other, DPlayer)):
            return self.username.Value() == other.username.Value()
        return False
    
    def __lt__(self, other):
        '''
        Check if this player is less than another.  Test to see if this Player's score is < C{other}'s score.
        
        @param other: Other player to test
        @return: True if this Player < C{Other}
        '''
        
        if (isinstance(other, DPlayer)):
            return self.score.Value() < other.score.Value()
        return False
    
    def __gt__(self, other):
        '''
        Check if this player is greater than another.  Test to see if this Player's score is > C{other}'s score.
        
        @param other: Other player to test
        @return: True if this Player > C{Other}
        '''
        
        if (isinstance(other, DPlayer)):
            return self.score.Value() > other.score.Value()
        return False
    
    def __repr__(self):
        '''
        Format Player as a string::
            USERNAME ( SCORE )
        
        @return: Player formatted as a string
        '''
        
        return '%s(%d)' % (self.getUsername(), int(self.score.Value()))
    
    def clone(self):
        '''
        Clone this Player.
        
        Cloned fields::
            - Username
        
        @return: Cloned Player.
        '''
        
        return DPlayer(username=self.username.Value())
    
    def clearLetters(self):
        '''
        Clear the letters for this Player.
        '''
        
        #self.letters = []
        self.letterStrs.Clear()
        self.letterScores.Clear()
    
    def getTime(self):
        if hasattr(self, 'u_time') and self.u_time is not None:
            return self.u_time
        else:
            return datetime.timedelta(seconds=0.0)
    
    def setTime(self, val):
        self.u_time = val
    
    time = property(getTime,setTime)
        
    