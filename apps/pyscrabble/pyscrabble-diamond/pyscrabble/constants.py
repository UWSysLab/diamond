REQUIRED_VERSION           = '1.6.1'
VERSION                    = '1.6.2'

### Misc
MAX_NAME_LENGTH        = 20
RECENT_MOVE_TIMEOUT    = 3
SERVER_MESSAGE_KEY     = "GAME"
RESERVED_NAMES         = [ SERVER_MESSAGE_KEY ]
AUDIT_SIZE             = 100
LOG_FILE               = 'pyscrabble.log'

### Stats
STAT_WINS     = "wins"
STAT_LOSSES   = "losses"
STAT_TIES     = "ties"
STAT_RANK     = "rank"
STAT_RECORD   = "record"

### Resource Definitions
DICT_DIR                   = 'dict'
LETTERS_DIR                = 'letters'
LOCALE_DIR                 = 'locale'
LOCALE_DOMAIN              = 'pyscrabble'
ONLINE_HELP_SITE           = 'http://pyscrabble.sourceforge.net/?q=help'
ONLINE_SITE                = 'http://pyscrabble.sourceforge.net'
SERVER_LISTING_LOCATION    = 'http://pyscrabble.sourceforge.net/server_listing'

SERVER_HISTORY             = 'server_history'
SERVER_CONSOLE_CONFIG      = 'server_console.cfg'
OPTION_CONFIG              = 'options.cfg'
RANK_CONFIG                = 'ranks.cfg'
DB_LOCATION                = 'pyscrabble.db.fs'

### GUI Definitions
DEFAULT_WIDTH           = 1024
DEFAULT_HEIGHT          = 768
TILE_HEIGHT             = 35
TILE_WIDTH              = TILE_HEIGHT
OPTION_WINDOW_HEIGHT    = 300
OPTION_WINDOW_WIDTH     = 800
REGISTER_WINDOW_HEIGHT  = 300
REGISTER_WINDOW_WIDTH   = 400

# Options
OPTIONS_SECTION       = "preferences"
HOSTS_SECTION         = "hosts"
OPTION_HOSTS          = "hosts"
OPTION_SOUND_TURN     = "sound_turn"
OPTION_POPUP_TURN     = "popup_turn"
OPTION_SOUND_MSG      = "sound_msg"
OPTION_POPUP_MSG      = "popup_msg"
SOUNDS_SECTION        = "sounds"
SOUND_GAME_OPTION     = "game"
SOUND_MSG_OPTION      = "message"
LOCALE_SECTION        = "locale"
LOCALE_OPTION         = "locale"
COLOR_NEW_TILE        = "c_new_tile"
USE_COLOR_NEW_TILE    = "enable_c_new_tile"
COLOR_RECENT_TILE     = "c_recent_tile"
USE_COLOR_RECENT_TILE = "enable_c_recent_tile"
COLOR_BLANK_TILE      = "c_blank_tile"
USE_COLOR_BLANK_TILE  = "enable_c_blank_tile"
OPTION_SWAP           = "option_swap"
OPTION_LETTER_SWAP    = "swap_letter"
OPTION_LETTER_INSERT  = "insert_letter"
OPTION_CLEAR_ON_ERROR = "clear_error"
OPTION_TILE_FONT      = "tile_font"
OPTION_ENABLE_T_TIPS  = "enable_tile_tips"
COLOR_NORMAL_TILE     = "c_normal_tile"
USE_COLOR_NORMAL_TILE = "enable_c_normal_tile"
COLOR_TEXT            = "c_text"
USE_COLOR_TEXT        = "enable_c_text"
COLOR_LETTER          = "c_letter"
USE_COLOR_LETTER      = "enable_c_letter"
OPTION_SAVE_LOGIN     = "save_login"
OPTION_SAVE_UNAME     = "s_uname"
OPTION_SAVE_PWORD     = "s_pword"
OPTION_SAVE_HOST      = "s_host"
OPTION_POPUP_TIMEOUT  = "p_timeout"
OPTION_TEXT_BOLD      = "show_bold"
OPTION_24_HOUR        = "show_24_hour"
OPTION_USE_PROXY      = "use_proxy"
OPTION_PROXY_TYPE     = "proxy_type"
OPTION_PROXY_HOST     = "proxy_host"
OPTION_PROXY_USER     = "proxy_user"
OPTION_PROXY_PASSWORD = "proxy_pass"
OPTION_PROXY_HTTP     = "proxy_http"
OPTION_PROXY_SOCKS4   = "proxy_socks4"
OPTION_PROXY_SOCKS5   = "proxy_socks5"
OPTION_SOUND_NEW_USER = "sound_new_user"
OPTION_POPUP_NEW_USER = "popup_new_user"
OPTION_SHOW_TIPS      = "show_tips"
OPTION_SHOW_PS        = "show_ps"

# Sounds
DEFAULT_SOUND = "notif.wav"
SOUNDS = [ SOUND_GAME_OPTION, SOUND_MSG_OPTION]

### Game Messaging
GAME_LEVEL         = 1
PLAYER_LEVEL       = 2
SPECTATOR_LEVEL    = 3

### Command Definitions
SEPARATOR = "|"

# Login Commands
LOGIN_HEADER         = "LOGIN"
LOGIN_INIT           = "login_init"
LOGIN_OK             = "login_ok"
LOGIN_DENIED         = "login_denied"
LOGOUT               = "logout"
CHANGE_PASSWORD      = "change_password"
NEW_USER             = "new_user"
BOOTED               = "booted"
SERVER_NUM_USERS     = "server_num_users"

# Chat Commands
CHAT_HEADER     = "CHAT"
CHAT_JOIN       = "chat_join"
CHAT_LEAVE      = "chat_leave"
CHAT_MESSAGE    = "chat_message"
CHAT_USERS      = "chat_users"
ERROR           = "error"
USER_INFO       = "user_info"
SERVER_STATS    = "server_stats"
INFO            = "info"
CHECK_MESSAGES  = "check_messages"
GET_MESSAGES    = "get_messages"
DELETE_MESSAGE  = "delete_message"

# Game Commands
GAME_HEADER                 = "GAME"
GAME_SEND_MOVE              = "game_send_move"
GAME_ACCEPT_MOVE            = "game_accept_move"
GAME_REJECT_MOVE            = "game_reject_move"
GAME_GET_LETTERS            = "game_get_letters"
GAME_JOIN                   = "game_join"
GAME_JOIN_OK                = "game_join_ok"
GAME_JOIN_DENIED            = "game_join_denied"
GAME_LEAVE                  = "game_leave"
GAME_CREATE                 = "game_create"
GAME_START                  = "game_start"
GAME_LIST                   = "game_list"
GAME_USER_LIST              = "game_user_list"
GAME_TURN_CURRENT           = "game_turn_current"
GAME_TURN_OTHER             = "game_turn_other"
GAME_ERROR                  = "game_error"
GAME_PASS                   = "game_pass"
GAME_INFO                   = "game_info"
GAME_PAUSE                  = "game_pause"
GAME_UNPAUSE                = "game_unpause"
GAME_TRADE_LETTERS          = "game_trade_letters"
GAME_CHAT_MESSAGE           = "game_chat"
GAME_SPECTATOR_JOIN         = "game_spectator_join"
GAME_SPECTATOR_LEAVE        = "game_spectator_leave"
GAME_SPECTATE_JOIN_OK       = "game_spectate_join_ok"
GAME_SPECTATOR_CHAT_SET     = "game_spectator_chat_set"
GAME_SEND_STATS             = "game_send_stats"
GAME_BAG_EMPTY              = "game_bag_empty"
GAME_SEND_SPECTATORS        = "game_send_specs"
GAME_SEND_OPTIONS           = "game_send_options"
GAME_OVER                   = "game_over"
GAME_TIME_EXPIRE            = "game_time_expire"
GAME_MOVE_TIME_EXPIRE       = "game_m_time_expire"
GAME_SPECTATOR_SET          = "game_spec_set"
GAME_BOOT                   = "game_boot"
GAME_DISTRIBUTION           = "game_dist"

# Private Message
PRIVATE_MESSAGE_HEADER     = "private_message"
PRIVATE_MESSAGE_SEND       = "private_message_send"

### Game Constants
MAX_PLAYERS      = 6
OVERTIME_PENALTY = 10
# Bonuses
BINGO_BONUS_SCORE = 50

# Colors
SERVER_MESSAGE        = "#990000"
SPECTATOR_MESSAGE     = "#006600"
DEFAULT_NEW_TILE      = "#CC66FF"
DEFAULT_RECENT_TILE   = "#CCCCCC"
DEFAULT_BLANK_TILE    = "#FF0300"
DEFAULT_COLOR_TEXT    = "#000000"


# Tile Constants
A = 1
B = 2
C = 3
D = 4
E = 5
F = 6
G = 7
H = 8
I = 9
J = 10
K = 11
L = 12
M = 13
N = 14
O = 15

TILE_LETTER          = 0
TILE_CENTER          = 5
TILE_NORMAL          = 1
TILE_DOUBLE_LETTER   = 2
TILE_TRIPLE_LETTER   = 3
TILE_DOUBLE_WORD     = 4
TILE_TRIPLE_WORD     = 6

LETTER_MODIFIERS     = [TILE_DOUBLE_LETTER, TILE_TRIPLE_LETTER]
WORD_MODIFIERS       = [TILE_DOUBLE_WORD, TILE_TRIPLE_WORD]
CENTER_MODIFIER      = [TILE_DOUBLE_LETTER]

TILE_COLORS = { 
                TILE_NORMAL            : "#FFFFFF", # White
                TILE_DOUBLE_LETTER     : "#CCCFFF", # Light Blue
                TILE_TRIPLE_LETTER     : "#3300CC", # Dark Blue
                TILE_DOUBLE_WORD       : "#FFCCFF", # Pink
                TILE_TRIPLE_WORD       : "#FF0300", # Red
                TILE_CENTER            : "#000000", # Black
                TILE_LETTER            : "#FFCC99"  # "Wood" 
               }

# Locations of special tiles
DOUBLE_LETTERS     = [(D,1), (L,1), (G,3), (I,3), (A,4), (H,4), (O,4), (G,7), (I,7), (C,7), (M,7), (D,8), (L,8), (C,9), (G,9), (I,9), (M,9), (A,12), (H,12), (O,12), (G,13), (I,13), (D,15), (L,15)]
TRIPLE_LETTERS     = [(F,2), (J,2), (B,6), (F,6), (J,6), (N,6), (B,10), (F,10), (J,10), (N,10), (F,14), (J,14)]
DOUBLE_WORDS       = [(B,2), (C,3), (D,4), (E,5), (K,5), (L,4), (M,3), (N,2), (B,14), (C,13), (D,12), (E,11), (K,11), (L,12), (M,13), (N,14)]
TRIPLE_WORDS       = [(A,1), (H,1), (O,1), (A,8), (O,8), (A,15), (H,15), (O,15)]
CENTER             = [(H, 8)]

# New constants
BOARD_WIDTH = 15