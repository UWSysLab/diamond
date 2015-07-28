from pyscrabble import lookup
from pyscrabble import util
import time

# Types
AUDIT_LOGON = 1
AUDIT_LOGOFF = 2
AUDIT_WIN_GAME = 3
AUDIT_LOSE_GAME = 4
AUDIT_TIE_GAME = 5

def fromType(type):
    '''
    Create and return an AuditAction based on type
    
    @param type:
    '''
    
    if type == AUDIT_LOGON: return LogonAction()
    if type == AUDIT_LOGOFF: return LogoffAction()
    if type == AUDIT_WIN_GAME: return GameWinAction()
    if type == AUDIT_LOSE_GAME: return GameLossAction()
    if type == AUDIT_TIE_GAME: return GameTieAction()

class Action(object):
    '''
    Base audit Action
    '''
    
    def __init__(self, type=None):
        '''
        Constructor
        '''
        self.date = util.Time(seconds=time.time(), dispDate=True)
        self.type = type


class LogonAction(Action):
    '''
    Logon action
    '''
    
    def __init__(self, username=None):
        '''
        Constructor
        
        @param username:
        '''
        
        Action.__init__(self, AUDIT_LOGON)
        self.username = username
    
    def __repr__(self):
        '''
        Print message
        '''
        return "%s %s" % (self.username, lookup.SERVER_MESSAGE_LOOKUP[lookup.LOGGED_IN])

class LogoffAction(Action):
    '''
    Logoff action
    '''
    
    def __init__(self, username=None):
        '''
        Constructor
        
        @param username:
        '''
        
        Action.__init__(self, AUDIT_LOGOFF)
        self.username = username
    
    def __repr__(self):
        '''
        Print message
        '''
        return "%s %s" % (self.username, lookup.SERVER_MESSAGE_LOOKUP[lookup.LOGGED_OUT])


class GameWinAction(Action):
    '''
    Game win action
    '''
    
    def __init__(self, winner=None, gameName = None, losers=None):
        '''
        Constructor
        
        @param winner:
        @param gameName:
        @param losers:
        '''
        Action.__init__(self, AUDIT_WIN_GAME)
        self.gameName = gameName
        
        if winner is not None:
            self.winner = '%s (%d)' % (winner.username, int(winner.score))
        
        if losers is not None:
            self.losers = ''
            for l in losers:
                if l != winner:
                    self.losers += '%s (%d)' % (l.username, int(l.score))
                    self.losers += ','
            self.losers = self.losers[:-1]
    
    def __repr__(self):
        '''
        Print message
        '''
        return "%s %s %s %s %s" % (self.winner, lookup.SERVER_MESSAGE_LOOKUP[lookup.DEFEATED], self.losers, lookup.SERVER_MESSAGE_LOOKUP[lookup.PLAYING_IN], self.gameName)

class GameLossAction(Action):
    '''
    Game loss action
    '''
    
    def __init__(self, winners=None, gameName = None, loser=None):
        '''
        
        @param winners:
        @param gameName:
        @param loser:
        '''
        Action.__init__(self, AUDIT_LOSE_GAME)
        self.gameName = gameName
        
        if winners is not None:
            self.winners = ''
            for w in winners:
                self.winners += '%s (%d)' % (w.username, int(w.score))
                self.winners += ','
            self.winners = self.winners[:-1]
        
        if loser is not None:
            self.loser = '%s (%d)' % (loser.username, int(loser.score))
        
    
    def __repr__(self):
        '''
        Print message
        '''
        return "%s %s %s %s %s" % (self.loser, lookup.SERVER_MESSAGE_LOOKUP[lookup.LOST_TO], self.winners, lookup.SERVER_MESSAGE_LOOKUP[lookup.PLAYING_IN], self.gameName)
        
class GameTieAction(Action):
    '''
    Game tie action
    '''
    
    def __init__(self, winner=None, gameName = None, other=None):
        '''
        
        @param winner:
        @param gameName:
        @param other:
        '''
        Action.__init__(self, AUDIT_TIE_GAME)
        self.gameName = gameName
        
        if winner is not None:
            self.winner = '%s (%d)' % (winner.username, int(winner.score))
        
        if other is not None:
            self.other = ''
            for o in other:
                self.other += '%s (%d)' % (o.username, int(o.score))
                self.other += ', '
            self.other = self.other[:-2]
    
    def __repr__(self):
        '''
        Print message
        '''
        return "%s %s %s %s %s" % (self.winner, lookup.SERVER_MESSAGE_LOOKUP[lookup.TIED_WITH], self.other, lookup.SERVER_MESSAGE_LOOKUP[lookup.PLAYING_IN], self.gameName)
        
      