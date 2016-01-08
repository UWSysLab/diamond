from pyscrabble.constants import *
from string import split

LOGIN_COMMAND = 0
GAME_COMMAND = 1
CHAT_COMMAND = 2

def fromType(type):
    if type == 0: return LoginCommand()
    if type == 1: return GameCommand()
    if type == 2: return ChatCommand()


class CommandCreator(object):
    '''
    CommandCreator is a utility class to create commands
    '''
    
    def __init__(self):
        '''
        Initializet the command parser
        '''
        
        self.parser = CommandParser()
    
    def createLoginCommand(self, username, password, version):
        '''
        
        @param username:
        @param password:
        @param version:
        @return: LoginCommand
        '''
        
        return self.parser.parseLoginCommand( [username, password, LOGIN_INIT, version] )
    
    def createNewUserCommand(self, username, password, isAdmin):
        '''
        
        @param username:
        @param password:
        @param isAdmin:
        @return LoginCommand
        '''
        
        return self.parser.parseLoginCommand( [username, password, NEW_USER, isAdmin] )
        
    def createGoodbyeCommand(self):
        '''
        
        @return: LoginCommand
        '''
        
        return self.parser.parseLoginCommand( ['','',LOGOUT,''] )
    
    def createGetLettersCommand(self, gameId, numLetters):
        '''
        
        @param gameId:
        @param numLetters:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_GET_LETTERS, gameId, numLetters] )
    
    def createGetGameListCommand(self, data=''):
        '''
        
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_LIST, '', data] )
    
    def createGameJoinCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_JOIN, gameId, data] )
    
    def createGameUserListCommand(self, gameId, data):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_USER_LIST, gameId, data] )
    
    def createGameStartCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_START, gameId, data] )
    
    def createGameLeaveCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_LEAVE, gameId, data] )
    
    def createGameTurnCurrentCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_TURN_CURRENT, gameId, data] )
    
    def createGameTurnOtherCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_TURN_OTHER, gameId, data] )
    
    def createGameErrorCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_ERROR, gameId, data] )
    
    def createGameSendMoveCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_SEND_MOVE, gameId, data] )
    
    def createGameAcceptMoveCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_ACCEPT_MOVE, gameId, data] )
    
    def createCreateGameCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_CREATE, gameId, data] )
    
    def createGamePassMoveCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_PASS, gameId, data] )
    
    def createGamePauseCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_PAUSE, gameId, data] )
    
    def createGameUnpauseCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_UNPAUSE, gameId, data] )
    
    def createGameTradeLettersCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data: List of Letters
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_TRADE_LETTERS, gameId, data] )
    
    def createBootedCommand(self, data=''):
        '''
        Command for the server to boot a user off the server
        
        @param data: Reason
        '''
        
        return self.parser.parseLoginCommand(['','', BOOTED, data])
    
    def createErrorCommand(self, data=''):
        '''
        Show an error message
        
        @param data: Error message
        '''
        return self.parser.parseChatCommand( ['', ERROR, data] )
    
    def createGameBagEmptyCommand(self, gameId, data=''):
        '''
        Game bag is empty 
        
        @param gameId: Game ID
        @param data:
        '''
        return self.parser.parseGameCommand( [GAME_BAG_EMPTY, gameId, data] )
    
    
    def createGameOverCommand(self, gameId, data=''):
        '''
        Game Over
        
        @param gameId: Game ID
        @param data:
        '''
        return self.parser.parseGameCommand ( [GAME_OVER, gameId, data] )
    
    def createGameBootCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        return self.parser.parseGameCommand( [GAME_BOOT, gameId, data] )
    
    def createGameDistributionCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        return self.parser.parseGameCommand( [GAME_DISTRIBUTION, gameId, data] )
        
        
        
        
        
    
        

class CommandParser(object):
    '''
    Command Parser.
    
    A command is structured as followed:
    HEADER | COMMAND | data1 | data2 | ... |
    '''
    
    def parseLoginCommand(self, elements):
        '''
        Parse a LoginCommand
        
        @param elements: List of command elements
        @return: LoginCommand
        '''
        
        return LoginCommand( elements.pop(0), elements.pop(0), elements.pop(0), elements.pop(0) )
    
    def parseChatCommand(self, elements):
        '''
        Parse a ChatCommand
        
        @param elements: List of command elements
        @return: ChatCommand
        '''
        
        return ChatCommand( elements.pop(0), elements.pop(0), elements.pop(0) )
    
    def parseGameCommand(self, elements):
        '''
        Parse a GameCommand
        
        @param elements: List of command elements
        @return: GameCommand
        '''
        
        return GameCommand( elements.pop(0), elements.pop(0), elements.pop(0) )
        
            

# Commands

class Command(object):
    '''
    Base Command class
    '''
    
    def __init__(self, type):
        '''
        Constructor
        
        @param type:
        '''
        self.type = type
        
    
    def getCommand(self):
        '''
        Retrieve the command information
        
        @see: L{constants}
        @return: Command information
        '''
        
        return self.command
    
    def setCommand(self, command):
        '''
        Set command information
        
        @param command: Command information
        @see: L{constants}
        '''
        
        self.command = command
    
    def getData(self):
        '''
        
        @return: Command data
        '''
        
        return self.data
    
    def setData(self, data):
        '''
        Set command data
        
        @param data: Command data
        '''
        
        self.data = data
    
    def getUsername(self):
        '''
        
        @return: Username of user executing command
        '''
        
        return self.username
    
    def setUsername(self, username):
        '''
        Set username for command
        
        @param username: Username
        '''
        
        self.username = username


class ChatCommand(Command):
    '''
    Chat Command class.  Used for chat functions
    '''
    
    def __init__(self, username=None, command=None, data=None):
        '''
        Initialize the ChatCommand
        
        @param username: Username
        @param command: Command
        @param data:
        @see: L{constants}
        '''
        Command.__init__(self, CHAT_COMMAND)
        self.command = command
        self.data = data
        self.username = username


class LoginCommand(Command):
    '''
    LoginCommand class.  Used for logging in/out of the system and changing password.
    '''
    
    def __init__(self, username=None, password=None, command=None, data=None):
        '''
        Initialize a new LoginCommand
        
        @param username:
        @param password:
        @param command:
        @param data:
        @see: L{constants}
        '''
        Command.__init__(self, LOGIN_COMMAND)
        self.username = username
        self.password = password
        self.command = command
        self.data = data
    
    def getPassword(self):
        '''
        
        @return: Password
        '''
        
        return self.password
    
    def setPassword(self, password):
        '''
        Set password
        
        @param password:
        '''
        
        self.password = password


class GameCommand(Command):
    '''
    GameCommand class for all Game functions
    '''
    
    def __init__(self, command=None, gameId=None,  data=None):
        '''
        Initialize a new GameCommand
        
        @param command:
        @param gameId:
        @param data:
        @see: L{constants}
        '''
        Command.__init__(self, GAME_COMMAND)
        self.command = command
        self.gameId = gameId
        self.data = data
    
    def getGameId(self):
        '''
        
        @return: Game ID
        '''
        
        return self.gameId
    
    def setGameId(self, gameId):
        '''
        Set Game ID
        
        @param gameId:
        '''
        
        self.gameId = gameId

