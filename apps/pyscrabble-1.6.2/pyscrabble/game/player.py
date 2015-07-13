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
        self.createdDate = util.Time(seconds=time.time(), dispDate=True)
        self.lastLogin = None
        self.__stats = {}
        self.__rankName = ''
        self.status = None
        self._audit = util.RingList(constants.AUDIT_SIZE)

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
    
    def addAuditAction(self, action):
        '''
        Add audit action
        
        @param action:
        '''
        
        self.audit.append( action )
    
    def getCreatedDate(self):
        '''
        Return timestamp account was created
        
        @return: Formatted date string
        '''
        
        if hasattr(self, 'createdDate'):
            if self.createdDate is not None:
                if not isinstance(self.createdDate, util.Time):
                    seconds = time.mktime(self.createdDate) + time.timezone
                    self.createdDate = util.Time(seconds=seconds, dispDate=True)
                else:
                    if not hasattr(self.createdDate, 'dispDate'):
                        self.createdDate.dispDate = True
                return self.createdDate
        
        return 'N/A'
    
    def getLastLoginDate(self):
        '''
        Return timestamp user last logged in
        
        @return: Formatted date string
        '''
        
        if hasattr(self, 'lastLogin'):
            if self.lastLogin is not None:
                if not isinstance(self.lastLogin, util.Time):
                    seconds = time.mktime(self.lastLogin) + time.timezone
                    self.lastLogin = util.Time(seconds=seconds, dispDate=True)
                else:
                    if not hasattr(self.lastLogin, 'dispDate'):
                        self.lastLogin.dispDate = True
                return self.lastLogin
        
        return 'N/A'
    
    def setLastLogin(self, seconds):
        '''
        Set last login date
        
        @param date: seconds
        '''
        self.lastLogin = util.Time(seconds=seconds, dispDate=True)
    
    def addWin(self, players):
        '''
        Add a win to the users stats
        
        @param players:
        '''
        self.stats[constants.STAT_WINS] = self.getNumericStat(constants.STAT_WINS) + 1
        self.stats[constants.STAT_RANK] = self.getNumericStat(constants.STAT_RANK) + 1
        for p in players:
            if p.username != self.username:
                r = self._getRecordFor( p.username )
                r["w"] = r["w"] + 1
    
    def addLoss(self, winners=None):
        '''
        Add a loss to the users stats
        
        @param winners:
        '''
        self.stats[constants.STAT_LOSSES] = self.getNumericStat(constants.STAT_LOSSES) + 1
        
        if winners is not None:
            for winner in winners:
                r = self._getRecordFor( winner.username )
                r["l"] = r["l"] + 1
    
    def addTie(self, winners):
        '''
        Add a tie to the users stats
        
        @param: winners
        '''
        self.stats[constants.STAT_TIES] = self.getNumericStat(constants.STAT_TIES) + 1
        
        for winner in winners:
            if winner.username != self.username:
                r = self._getRecordFor( winner.username )
                r["t"] = r["t"] + 1
    
    def addRank(self, rank):
        '''
        Add a value to the users rank
        
        @param rank: Numeric value to add.  This can be negative to subtract points
        '''
        self.stats[constants.STAT_RANK] = self.getNumericStat(constants.STAT_RANK) + int(rank)
    
    def _getRecordFor(self, username):
        '''
        Get the record for username
        
        @param username:
        '''
        if not self.record.has_key(username):
            self.record[username] = { "w" : 0, "l" : 0, "t" : 0 }
        return self.record[username]
    
    def getNumericStat(self, stat):
        '''
        Retrieve a numeric stat
        
        @param stat: Stat name
        @return: Numeric stat value
        '''
        if not self.stats.has_key(stat):
            self.stats[stat] = 0
        
        return self.stats[stat]
        
    def getStats(self):
        '''
        Get stats
        
        @return: Stats dict
        '''
        try:
            return self.__stats
        except AttributeError:
            self.__stats = {}
            return self.__stats
    
    def getAudit(self):
        '''
        Get audit log
        
        @return: Audit log
        '''
        try:
            return self._audit
        except AttributeError:
            self._audit = util.RingList(constants.AUDIT_SIZE)
            return self._audit
    
    def getRecord(self):
        '''
        Get Record data
        '''
        try:
            return self.stats[constants.STAT_RECORD]
        except KeyError:
            self.stats[constants.STAT_RECORD] = {}
            return self.stats[constants.STAT_RECORD]
    
    def setRank(self, rank):
        '''
        Set rank value
        
        @param rank: Rank value
        '''
        self.stats[constants.STAT_RANK] = rank
    
    def setRankName(self, rank):
        '''
        Rank Name
        
        @param rank: Name
        '''
        self.__rankName = rank
    
    def getRankName(self):
        '''
        Get Rank Name or '' if not set
        
        @return: Rank name
        '''
        try:
            return self.__rankName
        except:
            return ''
    
    def clone(self):
        '''
        Clone user
        
        @return: Cloned user
        '''
        x = User()
        x.__dict__ = self.__dict__.copy()
        return x
    
    audit = property(getAudit)    
    stats = property(getStats)
    rankName = property(getRankName, setRankName)
    record = property(getRecord)


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
            if (letter.isBlank()):
                letter.setLetter("")
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
        
    