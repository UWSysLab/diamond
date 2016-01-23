import datetime
import time
from pyscrabble import constants
from pyscrabble import util

class User(object):
    '''
    User on the server
    '''
    
    def __init__(self, username='', password='', isAdmin = False):
        '''
        Constructor
        
        @param username: Username
        @param password: Password
        @param isAdmin: Is Administrator? True or False
        '''
        
        self.username = username
        self.password = password
        self.__isAdmin = isAdmin
        self.status = None

    def getUsername(self):
        '''
        
        @return: Username
        '''
        
        return util.getUnicode(self.username)

    def getPassword(self):
        '''
        
        @return: Password
        '''
        
        return self.password

    def setPassword(self, password):
        '''
        Set the password
        
        @param password: Password
        '''
        
        self.password = password

    def isAdmin(self):
        '''
        
        @return: True if the user is an Administrator, False otherwise
        '''
        
        return self.__isAdmin
    
    def setIsAdmin(self, isAdmin):
        '''
        Set is Admin
        
        @param isAdmin: True if the user is an Administrator, False otherwise
        '''
        
        self.__isAdmin = isAdmin 
    
    def clone(self):
        '''
        Clone user
        
        @return: Cloned user
        '''
        x = User()
        x.__dict__ = self.__dict__.copy()
        return x
    

class Player(object):
    '''
    Player class.
    
    A Player in the game.
    '''
    
    def __init__(self, username=''):
        '''
        Initialize the player
        
        @param username: Username of Player
        '''
        
        self.username = username
        self.score = 0
        self.letters = []
        self.u_time = None
    
    def addScore(self, score):
        '''
        Add C{score} to this players score.
        
        @param score: Score to add to Player's score.
        '''
        
        self.score = self.score + score
    
    def getScore(self):
        '''
        
        @return: Player's score
        '''
        
        return self.score
    
    def getUsername(self):
        '''
        
        @return: Player's username
        '''
        
        return util.getUnicode(self.username)
        
    def addLetters(self, letters):
        '''
        Add Letters to this Player's letterbox
        
        @param letters: List of Letters to add.
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        self.letters.extend( letters )
    
    def getNumberOfLettersNeeded(self):
        '''
        Calculate the number of Letters need to make sure this player has 7 Letters
        
        @return: Number of Letters need to fill the Players letterbox
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        return 7 - len(self.letters)
    
    def removeLetters(self, list):
        '''
        Remove Letters from this Player's letterbox
        
        @param list: List of Letters to remove.
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        for letter in list:
            self.letters.remove( letter )
        
    def getLetters(self):
        '''
        
        @return: List of Players Letters
        @see: L{pyscrabble.game.pieces.Letter}
        '''
        
        return self.letters
    
    def reset(self):
        '''
        Reset this Player.
        
        Set the Player' score to zero.
        Remove all Letters from the Player's letterbox.
        '''
        
        self.score = 0
        self.letters = []
    
    def __eq__(self, other):
        '''
        Check if this player equals another. Tests to see if the usernames are equals
        
        @param other: Other player to test.
        @return: True if this Player equals C{other}
        '''
        
        if (isinstance(other, Player)):
            return self.username == other.username
        return False
    
    def __lt__(self, other):
        '''
        Check if this player is less than another.  Test to see if this Player's score is < C{other}'s score.
        
        @param other: Other player to test
        @return: True if this Player < C{Other}
        '''
        
        if (isinstance(other, Player)):
            return self.score < other.score
        return False
    
    def __gt__(self, other):
        '''
        Check if this player is greater than another.  Test to see if this Player's score is > C{other}'s score.
        
        @param other: Other player to test
        @return: True if this Player > C{Other}
        '''
        
        if (isinstance(other, Player)):
            return self.score > other.score
        return False
    
    def __repr__(self):
        '''
        Format Player as a string::
            USERNAME ( SCORE )
        
        @return: Player formatted as a string
        '''
        
        return '%s(%d)' % (self.getUsername(), int(self.score))
    
    def clone(self):
        '''
        Clone this Player.
        
        Cloned fields::
            - Username
        
        @return: Cloned Player.
        '''
        
        return Player(username=self.username)
    
    def clearLetters(self):
        '''
        Clear the letters for this Player.
        '''
        
        self.letters = []
    
    def getTime(self):
        if hasattr(self, 'u_time') and self.u_time is not None:
            return self.u_time
        else:
            return datetime.timedelta(seconds=0.0)
    
    def setTime(self, val):
        self.u_time = val
    
    time = property(getTime,setTime)


class PlayerInfo(object):
    '''
    Information about a Player
    '''
    
    def __init__(self, name=None, score=None, numLetters=None, time=None):
        '''
        
        @param name:
        @param score:
        @param numLetters:
        '''
        self.name = name
        self.score = score
        self.numLetters = numLetters
        self.time = time
        self.stamp = None
        #if time is not None:
        #    self.time = util.formatTimeDelta(time)
    
    def __repr__(self):
        '''
        Format Player as a string::
            USERNAME ( SCORE )
        
        @return: Player formatted as a string
        '''
        return unicode('%s(%d)'% (self.name, int(self.score)))
        
    