class BagEmptyException(Exception): pass
class GameOverException(Exception): pass
class MoveException(Exception): pass

class MoveNotTouchingException(MoveException):
    
    def __init__(self):
        MoveException.__init__(self)
        self.message = _('Move must connect to an existing move')

class TilesNotConnectedException(MoveException):
    
    def __init__(self):
        MoveException.__init__(self)
        
        self.message = _('Tiles are not connected')

class ProxyAuthorizationRequiredException(Exception):
    
    def __init__(self, responseCode, realm):
        Exception.__init__(self)
        self.responseCode = responseCode
        self.realm = realm
    
    def getErrorMessage(self):
        return _('Invalid proxy credentials');