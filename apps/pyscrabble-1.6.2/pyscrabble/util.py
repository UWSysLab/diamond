import base64
import random
import re
import sha
import time
import webbrowser
from pyscrabble import constants
from pyscrabble import manager

MIN_RAND     = 0
MAX_RAND     = 100000

EMAIL_RE     = re.compile("^.+@.+\..{2,3}$")
WEB_RE       = re.compile("^www\..+\..{2,3}$")
HTTP_RE      = re.compile("^https*.+")
URL_RE       = re.compile("(https*://[-A-Za-z0-9._~:/?#\[\]@!$&'()*+,;=]+[A-Za-z0-9_/])(.*)")

def getUnicode(data):
    if not isinstance(data, unicode):
        return unicode(data, 'utf-8')
    return data

def getAdditionalHosts():
    result = []
    o = manager.OptionManager(section=constants.HOSTS_SECTION)
    data = o.get_default_option(constants.OPTION_HOSTS, None)
    if data is not None:
        chunks = data.split('/')
        for chunk in chunks:
            if chunk is not '':
                result.append( chunk.split(':') )
    return result

def showUrl(dialog, x, y=None):
    webbrowser.open( x, new=True )

def showHelp(menu):
    showUrl( None, constants.ONLINE_HELP_SITE, None )

def isCenter(x, y):
    return (x+1, y+1) in constants.CENTER

# Get the modifier for a certain tile position
def getTileModifier(x, y):
    
    if (x+1, y+1) in constants.DOUBLE_LETTERS:
        return constants.TILE_DOUBLE_LETTER
        
    if (x+1, y+1) in constants.TRIPLE_LETTERS:
        return constants.TILE_TRIPLE_LETTER
    
    if (x+1, y+1) in constants.DOUBLE_WORDS:
        return constants.TILE_DOUBLE_WORD
    
    if (x+1, y+1) in constants.TRIPLE_WORDS:
        return constants.TILE_TRIPLE_WORD
    
    if (x+1, y+1) in constants.CENTER:
        return constants.TILE_DOUBLE_WORD
    
    return constants.TILE_NORMAL

def b64encode(data):
    '''
    Base64-encode some data
    
    @param data:
    '''
    if hasattr(base64, 'b64encode'):
        return base64.b64encode(data)
    else:
        return base64.encodestring(data)[:-1]

def hashPassword(password):
    '''
    SHA-1 Hash password
    
    @param password: Plaintext
    @return: SHA-1 hashed plaintext
    '''
    digest = sha.new(password).digest()
    return b64encode(digest)

def ternary(exp, a, b):
    '''
    Ternary operator
    
    @param exp: expression to evaluate
    @param a: Return a if exp is true
    @param b: Return b if exp is false
    @return: a if exp is true, b if exp is false
    '''
    if exp:
        return a
    return b
    

def formatTimeDelta(td):
    '''
    Format timedelta
    
    @param td: datetime.timedelta object
    '''
    if td.days >= 0:
        return str(td)
    else:
        return '-%s' % str(-td)
    

def getRandomId():
    '''
    Generate a random id
    
    @return: Random ID number
    '''
    return random.randint(MIN_RAND, MAX_RAND)
    

def isEmail(data):
    '''
    Match email address
    
    @param data: String data
    @return: Match object or None
    '''
    data,trailing = getEmail(data)
    return EMAIL_RE.match(data)

def isURL(data):
    '''
    Match URLs
    
    @param data: String data
    @return: Match object or None
    '''
    data,trailing = getURL(data)
    return HTTP_RE.match(data) or WEB_RE.match(data)

def getURL(data):
    '''
    Get URL from String
    
    Return a Tuple->URL, Trailing data(like punctuation)
    
    @param data: String data
    @return: Return a Tuple->URL, Trailing data(like punctuation)
    '''
    m = URL_RE.search(data)
    if m is not None:
        return m.groups()
    else:
        return '',''
    
    
def getEmail(data):
    '''
    Get Email from String
    
    Return a Tuple->Email, Trailing data(like punctuation)
    
    @param data: String data
    @return: Return a Tuple->Email, Trailing data(like punctuation)
    '''
    end = data[-1:]
    end_data = []
    while (not end.isalpha() and end):
        end_data.append(end)
        data = data[:-1]
        end = data[-1:]
    
    return data,end_data

def colorToHexString(red, green, blue):
    return '#%s%s%s' % ( toHex(red), toHex(green), toHex(blue) )

def toHex(num):
    num = hex( num / 256 )[2:]
    if len(num) == 1:
        return '0%s' % num
    else:
        return num


class TimeDeltaWrapper(object):
    '''
    Wrapper for datetime.timedelta
    
    Basically to pretty print timedeltas when they are < 0
    '''
    
    def __init__(self, td=None):
        self.td = td
    
    def __repr__(self):
        return formatTimeDelta(self.td)
    
    def getSeconds(self):
        return self.td.seconds
    
    def getDays(self):
        return self.td.days
    
    def __sub__(self, obj):
        return self.td - obj
    
    def __add__(self, obj):
        return self.td + obj
    
    seconds=property(getSeconds)
    days=property(getDays)
    

class ServerBulletin(object):
    '''
    Server Bulletin
    '''
    
    def __init__(self, data=None, id=None, seconds=None):
        
        self.data = data
        self.createdDate = Time(seconds=seconds, dispDate=True)
        self.id = id
    
    def getCreatedDate(self):
        return str(self.createdDate)
    
    date = property(getCreatedDate)

class Time(object):
    '''
    Time object
    '''
    
    def __init__(self, seconds=None ,dispDate=False):    
        self.seconds = seconds
        self.dispDate = dispDate
    
    def __repr__(self):
        if self.seconds is not None:
            format = ''
            if self.dispDate:
                format += '%m/%d/%Y '
            
            o = manager.OptionManager()
            if o.get_default_bool_option(constants.OPTION_24_HOUR, False):
                format += '%H:%M:%S'
            else:
                format += '%I:%M:%S %p'
            return time.strftime(format, time.localtime(self.seconds))
        else:
            return 'N/A'
    
    def __cmp__(self, obj):
        '''
        Compare
        
        @param obj:
        '''
        if isinstance(obj, Time):
            return self.seconds - obj.seconds
        
        return 1

class PrivateMessage(object):
    '''
    Private Message sent offline
    '''
    
    def __init__(self, sender=None, data=None, id=None, time=None):
        
        self.sender = sender
        self.data = data
        self.time = time
        self.id = id
        self.isRead = False
    
    def getCreatedDate(self):
        if hasattr(self, 'createdDate'):
            return time.strftime("%m/%d/%y %I:%M:%S %p", self.createdDate)
        else:
            return str(self.time)
    
    def getRead(self):
        if hasattr(self, 'isRead'):
            return self.isRead
        else:
            return False
    
    def setRead(self, val):
        self.isRead = val
    
    date = property(getCreatedDate)
    read = property(getRead, setRead)

class ErrorMessage:
    
    def __init__(self, msg):
        self.errorMessage = msg
    
    def getErrorMessage(self):
        if isinstance(self.errorMessage, str):
            return self.errorMessage
        return repr(self.errorMessage)

        
class RingList(object):
    '''
    Fixed size list
    '''
    
    def __init__(self, size=0):
        
        self._data = []
        self._size = size
    
    def __append__(self, item):
        self.append(item)
    
    def append(self, item):
        if len(self._data) == self._size:
            self._data = self._data[1:]
        self._data.append( item )
    
    def clear(self):
        self._data = []
        
    def __iter__(self):
        return iter(self._data)
    
    def __reversed__(self):
        return reversed(self._data)
    
    def __getitem__(self, item):
        return self._data[item]
    
    def __setitem__(self, item, value):
        self._data[item] = value
    
    def __contains__(self, item):
        return item in self._data
    
    def __repr__(self):
        return repr(self._data)
    
    def __str__(self):
        return str(self._data)