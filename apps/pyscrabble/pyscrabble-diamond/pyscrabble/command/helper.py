from pyscrabble.constants import *
from string import split

LOGIN_COMMAND = 0
GAME_COMMAND = 1
CHAT_COMMAND = 2
PRIVATE_MESSAGE_COMMAND = 3

def fromType(type):
    if type == 0: return LoginCommand()
    if type == 1: return GameCommand()
    if type == 2: return ChatCommand()
    if type == 3: return PrivateMessageCommand()


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
    
    def createChangePasswordCommand(self, newpassword, oldpassword):
        '''
        
        @param newpassword:
        @param oldpassword:
        @return LoginCommand
        '''
        
        return self.parser.parseLoginCommand( ['', newpassword, CHANGE_PASSWORD, oldpassword] )
        
    def createGoodbyeCommand(self):
        '''
        
        @return: LoginCommand
        '''
        
        return self.parser.parseLoginCommand( ['','',LOGOUT,''] )
    
    def createJoinChatCommand(self, username):
        '''
        
        @param username:
        @return: ChatCommand
        '''
        
        return self.parser.parseChatCommand( [username, CHAT_JOIN, ''] )
    
    def createLeaveChatCommand(self):
        '''
        @return: ChatCommand
        '''
        
        return self.parser.parseChatCommand( ['', CHAT_LEAVE, ''] )
    
    def createPostChatMessageCommand(self, msg, username='' ):
        '''
        
        @param msg:
        @param username:
        @return: ChatCommand
        '''
        
        return self.parser.parseChatCommand( [username, CHAT_MESSAGE, msg] )
    
    def createGetMessagesCommand(self, data=''):
        '''
        
        @param data: Data
        @return: ChatCommand
        '''
        
        return self.parser.parseChatCommand( ['', GET_MESSAGES, data] )
    
    def createCheckMessagesCommand(self, data=''):
        '''
        
        @param data: Data
        @return: ChatCommand
        '''
        
        return self.parser.parseChatCommand( ['', CHECK_MESSAGES, data] )
    
    def createDeleteMessageCommand(self, data=''):
        '''
        
        @param data: Data
        @return: ChatCommand
        '''
        
        return self.parser.parseChatCommand( ['', DELETE_MESSAGE, data] )
    
    def createGetChatUsersCommand(self, users=''):
        '''
        
        @param users: List of users
        @return: ChatCommand
        '''
        
        return self.parser.parseChatCommand( ['', CHAT_USERS, users] )
    
    def createGameDiamondRefreshCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_DIAMOND_REFRESH, gameId, data] )
    
    def createGameDiamondRequestRefreshCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_DIAMOND_REQUEST_REFRESH, gameId, data] )
    
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
    
    def createGameInfoCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        
        return self.parser.parseGameCommand( [GAME_INFO, gameId, data] )
    
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
    
    def createGameChatMessageCommand(self, gameId, data=''):
        '''
        Send a game chat message to the server
        
        @param gameId: Game ID
        @param data: Chat message
        '''
        
        return self.parser.parseGameCommand( [GAME_CHAT_MESSAGE, gameId, data] )
    
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
    
    def createInfoCommand(self, data=''):
        '''
        Show an Info message
        
        @param data: Info message
        '''
        return self.parser.parseChatCommand( ['', INFO, data] )
    
    def createPrivateMessageCommand(self, sender, recipient, data):
        '''
        Send a private message
        
        @param sender:
        @param recipient:
        @param data:
        '''
        
        return self.parser.parsePrivateMessageCommand( [PRIVATE_MESSAGE_SEND, sender, recipient, data] )
    
    def createGameSpectatorJoinCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        return self.parser.parseGameCommand( [GAME_SPECTATOR_JOIN, gameId, data] )
    
    def createGameSpectatorLeaveCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        return self.parser.parseGameCommand( [GAME_SPECTATOR_LEAVE, gameId, data] )
    
    def createGameSpectatorChatCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        return self.parser.parseGameCommand( [GAME_SPECTATOR_CHAT_SET, gameId, data] )
    
    def createGameStatsCommand(self, gameId, data=''):
        '''
        Send game stats
        
        @param gameId: Game ID
        @param data: Stat data
        '''
        return self.parser.parseGameCommand( [GAME_SEND_STATS, gameId, data] )
    
    def createGameBagEmptyCommand(self, gameId, data=''):
        '''
        Game bag is empty 
        
        @param gameId: Game ID
        @param data:
        '''
        return self.parser.parseGameCommand( [GAME_BAG_EMPTY, gameId, data] )
    
    def createGameSendSpectatorsCommand(self, gameId, data=''):
        '''
        Send list of spectators
        
        @param gameId: Game ID
        @param data:
        '''
        return self.parser.parseGameCommand ( [GAME_SEND_SPECTATORS, gameId, data] )
    
    
    def createUserInfoCommand(self, username, data=''):
        '''
        User info
        
        @param username:
        @return: ChatCommand
        '''
        return self.parser.parseChatCommand( [username, USER_INFO, data] )
    
    def createServerStatsCommand(self, data=''):
        '''
        Server stats
        
        @return: ChatCommand
        '''
        return self.parser.parseChatCommand( ['', SERVER_STATS, data] )
    
    def createGetNumServerUsersCommand(self, data=''):
        '''
        Get the number of users on a server
        
        @return: LoginCommand
        '''
        
        return self.parser.parseLoginCommand( ['', '', SERVER_NUM_USERS, data] )
    
    def createGameSendOptionsCommand(self, gameId, data=''):
        '''
        Send game options
        
        @param gameId: Game ID
        @param data:
        '''
        return self.parser.parseGameCommand ( [GAME_SEND_OPTIONS, gameId, data] )
    
    def createGameOverCommand(self, gameId, data=''):
        '''
        Game Over
        
        @param gameId: Game ID
        @param data:
        '''
        return self.parser.parseGameCommand ( [GAME_OVER, gameId, data] )
    
    def createGameTimeExpireCommand(self, gameId, data=''):
        '''
        Game Time has expired for a player
        
        @param gameId: Game ID
        @param data:
        '''
        return self.parser.parseGameCommand ( [GAME_TIME_EXPIRE, gameId, data] )
    
    def createMoveTimeExpireCommand(self, gameId, data=''):
        '''
        Move Time has expired for a player
        
        @param gameId: Game ID
        @param data:
        '''
        return self.parser.parseGameCommand ( [GAME_MOVE_TIME_EXPIRE, gameId, data] )
    
    def createGameSpectatorSetCommand(self, gameId, data=''):
        '''
        
        @param gameId:
        @param data:
        @return: GameCommand
        '''
        return self.parser.parseGameCommand( [GAME_SPECTATOR_SET, gameId, data] )
    
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
    
    def parsePrivateMessageCommand(self, elements):
        '''
        Parse a PrivateMessageCommand
        
        @param elements: List of command elements
        '''
        
        return PrivateMessageCommand( elements.pop(0), elements.pop(0), elements.pop(0), elements.pop(0) )
        
            

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


class PrivateMessageCommand(Command):
    
    def __init__(self, command=None, sender=None, recipient=None, data=None):
        '''
        Private message command
        
        @param command:
        @param sender:
        @param recipient:
        @param data:
        '''
        Command.__init__(self, PRIVATE_MESSAGE_COMMAND)
        self.command = command
        self.sender = sender
        self.recipient = recipient
        self.data = data
    
    def getSender(self):
        '''
        Get sender
        
        @return: Sender
        '''
        
        return self.sender
    
    def setSender(self, sender):
        '''
        Set sender
        
        @param sender:
        '''
        
        self.sender = sender
    
    def getRecipient(self):
        '''
        Get Recipient
        
        @return: Recipient
        '''
        
        return self.recipient
    
    def setRecipient(self, recipient):
        '''
        Set recipient
        
        @param recipient: Recipient
        '''
        
        self.recipient = recipient
        