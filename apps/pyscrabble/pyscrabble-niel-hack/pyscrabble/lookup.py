import __builtin__
if not hasattr(__builtin__, '_'):
    from gettext import gettext as _

from pyscrabble.constants import MAX_NAME_LENGTH

### Server messages
INVALID_OLD_PASSWORD            = 2
USER_ALREADY_EXISTS             = 3
USERNAME_MUST_BE_LESS_THAN      = 4
CHARACTERS                      = 5
GAME_NAME_MUST_BE_LESS_THAN     = 6
GAME_ALREADY_EXISTS             = 7
SERVER_DELETE_GAME              = 8
LOGGED_OUT                      = 9
LOGGED_IN                       = 10
IS_NOT_LOGGED_IN                = 11
GAME_SAVED                      = 12
GAME_RESUMED                    = 13
REQUIRED_NOT_MET                = 14
SPECTATOR_CHAT_DISABLED         = 15
ENABLE_SPEC_CHAT                = 16
DISABLE_SPEC_CHAT               = 17
CANNOT_JOIN_STARTED             = 18
GAME_FULL                       = 19
GAME_ALREADY_STARTED            = 20
STARTED                         = 21
TURN                            = 22
LEFT_GAME                       = 23
MOVE_GAME_PAUSED                = 24
NOT_IN_PROGRESS                 = 25
NOT_IN_DICT                     = 26
ALREADY_ON_BOARD                = 27
HAS_ADDED                       = 28
CANNOT_BE_ADDED_TWICE           = 29
MADE_A_BINGO                    = 30
PASS_PAUSED                     = 31
HAS_PASSED                      = 32
HAS_TRADED                      = 33
HAS_WON                         = 34
HAVE_TIED                       = 35
IS_SPECTATING                   = 36
NO_LONGER_SPECTATING            = 37
REQ_VERSION                     = 38
ALREADY_LOGGED_IN               = 39
INVALID_USERNAME_PASSWORD       = 40
OPTION_CENTER_TILE              = 41
OPTION_RANKED                   = 42
STAT_LETTERS_LEFT               = 43
STAT_HIGHEST_SCORING_WORD       = '44' # Must be String because this is stored in a shelf
STAT_LONGEST_WORD               = '45'
MOST_USERS                      = 46
NUMBER_USERS                    = 47
UPTIME                          = 48
STAT_HIGHEST_SCORING_MOVE       = '49'
STAT_MOST_WORDS_IN_MOVE         = '50'
STAT_HIGHEST_TOTAL_SCORE        = '51'
OPTION_RULES                    = 52
MESSAGE_SENT                    = 55
DOES_NOT_EXIST                  = 56
NOT_CREATOR                     = 57
CREATOR                         = 58
MESSAGES_AVAILABLE              = 59
STATUS_STARTED                  = 60
NOT_STARTED                     = 61
COMPLETE                        = 62
SAVED                           = 63
LOSES                           = 64
GAINS                           = 65
FROM                            = 66
USERNAME_NOT_ALLOWED            = 67
SUCCESS                         = 68
OPTION_TIMED_GAME               = 69
OPTION_TIMED_LIMIT              = 70
OUT_OF_TIME                     = 71
OPTION_MOVE_TIME                = 72
MOVE_OUT_OF_TIME                = 73
OLD_MESSAGES_AVAILABLE          = 74
OFFLINE                         = 75
ONLINE                          = 76
PLAYING                         = 77
WATCHING                        = 78
MINUTE                          = 79
MINUTES                         = 80
LETTERS                         = 81
LETTER                          = 82
ENABLE_SPEC                     = 84
DISABLE_SPEC                    = 85
SPECTATORS_BANNED               = 86
OPTION_SHOW_COUNT               = 87
SERVER_VERSION                  = 88
DEFEATED                        = 89
TIED_WITH                       = 90
LOST_TO                         = 91
PLAYING_IN                      = 92

SERVER_MESSAGE_LOOKUP = {
    PLAYING_IN                   : _('playing in'),
    LOST_TO                      : _('lost to'),
    TIED_WITH                    : _('tied with'),
    DEFEATED                     : _('defeated'),
    SERVER_VERSION               : _('Server version'),
    OPTION_SHOW_COUNT            : _('Show letter distribution'),
    SPECTATORS_BANNED            : _('Spectators are not allowed in this game.'),
    DISABLE_SPEC                 : _('has banned spectators'),
    ENABLE_SPEC                  : _('has allowed spectators'),
    LETTER                       : _('letter'),
    LETTERS                      : _('letters'),
    MINUTES                      : _('minutes'),
    MINUTE                       : _('minute'),
    WATCHING                     : _('Watching'),
    PLAYING                      : _('Playing in'),
    ONLINE                       : _('Online'),
    OFFLINE                      : _('Offline'),
    OLD_MESSAGES_AVAILABLE       : _('You have messages'),
    MOVE_OUT_OF_TIME             : _('has run out of time for this move'),
    OPTION_MOVE_TIME             : _('Timed Moves'),
    OUT_OF_TIME                  : _('has run out of time'),
    OPTION_TIMED_LIMIT           : _('Optional overtime limit'),
    OPTION_TIMED_GAME            : _('Timed Game'),
    USERNAME_NOT_ALLOWED         : _('Username not allowed'),
    FROM                         : _('from'),
    GAINS                        : _('gains'),
    LOSES                        : _('loses'),
    SAVED                        : _('Saved'),
    NOT_STARTED                  : _('Not Started'),
    COMPLETE                     : _('Complete'),
    STATUS_STARTED               : _('Started'),
    MESSAGES_AVAILABLE           : _('You have new messages'),
    CREATOR                      : _('Creator'),
    NOT_CREATOR                  : _('You must be the game creator to perform this action'),
    DOES_NOT_EXIST               : _('does not exist'),
    MESSAGE_SENT                 : _('Offline message sent'),
    OPTION_RULES                 : _('Rules'),
    STAT_LETTERS_LEFT            : _("Letters left"),
    STAT_HIGHEST_SCORING_WORD    : _("Highest scoring word"),
    STAT_LONGEST_WORD            : _("Longest word"),
    MOST_USERS                   : _("Most users logged in"),
    NUMBER_USERS                 : _("Number of registered users"),
    UPTIME                       : _("Server up since"),
    STAT_HIGHEST_SCORING_MOVE    : _("Highest scoring move"),
    STAT_MOST_WORDS_IN_MOVE      : _("Most words in move"),
    STAT_HIGHEST_TOTAL_SCORE     : _("Highest total score"),
    OPTION_CENTER_TILE           : _("Center Tile is Double Word Score"), 
    OPTION_RANKED                : _("Official Game"),
    INVALID_USERNAME_PASSWORD    : _('Invalid username/password'),
    ALREADY_LOGGED_IN            : _('You are already logged in'),
    REQ_VERSION                  : _('You must have PyScrabble version '),
    ALREADY_LOGGED_IN            : _('You are already logged in'),
    HAS_WON                      : _('has won the game'),
    HAVE_TIED                    : _('have tied the game'),
    IS_SPECTATING                : _('is spectating'),
    NO_LONGER_SPECTATING         : _('is no longer spectating'),
    HAS_TRADED                   : _('has traded'),
    HAS_PASSED                   : _('has passed'),
    PASS_PAUSED                  : _('Cannot pass move. Game is saved'),
    MADE_A_BINGO                 : _('made a bingo!'),
    CANNOT_BE_ADDED_TWICE        : _('cannot be added twice'),
    HAS_ADDED                    : _('has added'),
    ALREADY_ON_BOARD             : _('is already on the board'),
    NOT_IN_DICT                  : _('not in dictionary'),
    NOT_IN_PROGRESS              : _('Game is not started'),
    MOVE_GAME_PAUSED             : _('Cannot send move. Game is saved'),
    LEFT_GAME                    : _('has left the game'),
    TURN                         : _('now has control of the board'),
    STARTED                      : _('started'),
    GAME_ALREADY_STARTED         : _('Game already started'),
    SUCCESS                      : _('Success'),
    IS_NOT_LOGGED_IN             : _('is not logged in'),
    GAME_ALREADY_EXISTS          : _('Game already exists'),
    USER_ALREADY_EXISTS          : _("User already exists"),
    USERNAME_MUST_BE_LESS_THAN   : '%s %s %s' % (_("User name must be less than"), str(MAX_NAME_LENGTH), _("characters")),
    GAME_NAME_MUST_BE_LESS_THAN  : _('Game name must be less than'),
    CHARACTERS                   : _('characters'),
    LOGGED_OUT                   : _('has logged out'),
    LOGGED_IN                    : _('has logged in'),
    INVALID_OLD_PASSWORD         : _('Invalid old password'),
    IS_NOT_LOGGED_IN             : _('is not logged in'),
    GAME_SAVED                   : _('The game is now saved.'),
    GAME_RESUMED                 : _('The game is now resumed.'),
    REQUIRED_NOT_MET             : _('All required players have not joined yet'),
    SPECTATOR_CHAT_DISABLED      : _('Spectator Chat is disabled for this game.'),
    ENABLE_SPEC_CHAT             : _('has enabled spectator chat'),
    DISABLE_SPEC_CHAT            : _('has disabled spectator chat'),
    CANNOT_JOIN_STARTED          : _('Cannot join game that has started'),
    GAME_FULL                    : _('Game is full')
}

class ServerMessage(object):
    
    def __init__(self, data=None, timeData=None):
        self.data = data
        self.timeData = timeData
    
    def __repr__(self):
        buf = ''
        
        if hasattr(self, 'timeData'):
            if self.timeData is not None:
                buf = buf + '[%s] ' % (str(self.timeData)) #time.strftime(format, time.localtime(secs))
        
        for item in self.data:
            if isinstance(item, int) and not isinstance(item, bool) and SERVER_MESSAGE_LOOKUP.has_key(item):
                buf = buf + SERVER_MESSAGE_LOOKUP[item]
            else:
                if isinstance(item, str) and SERVER_MESSAGE_LOOKUP.has_key(item):
                    buf = buf + SERVER_MESSAGE_LOOKUP[item]
                else:
                    buf = buf + str(item)
            
            if not item == self.data[ len(self.data) - 1]:
                buf = buf + ' '
                
        return buf