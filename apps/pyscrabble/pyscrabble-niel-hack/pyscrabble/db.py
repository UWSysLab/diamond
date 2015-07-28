from pyscrabble import constants
from pyscrabble import manager
from ZODB import FileStorage, DB as _DB
import transaction

class DB(object):
    '''
    Database class
    '''
    
    def __init__(self):
        '''
        Initialize the connection to the DB
        '''
        r = manager.ResourceManager()
        path = r["config"][constants.DB_LOCATION]
        
        storage = FileStorage.FileStorage(path)    
        db = _DB(storage)
        self._conn = db.open()
        self._root = self._conn.root()
    
    def __getattr__(self, key):
        '''
        Retrive a key from the database
        
        @param key: Key
        '''
        if key.startswith('_'):
            return object.__getattr__(self, key)
        
        if not self._root.has_key(key):
            self._root[key] = {}
            self.sync()
        return self._root[key]
    
    def __setattr__(self, key, value):
        '''
        Add a key to the database
        
        @param key: Key
        @param value: Value
        '''
        if key.startswith('_'):
            object.__setattr__(self, key, value)
        else:
            self._root[key] = value
    
    def __delattr__(self, key):
        '''
        Delete a key from the database
        
        @param key: key
        '''
        if key.startswith('_'):
            object.__delattr__(self, key)
        else:
            del self._root[key]
    
    def sync(self):
        '''
        Commit any open transactions
        '''
        self._root._p_changed = True
        transaction.commit()
    
    def close(self):
        '''
        Close the database
        '''
        self.sync()
        try :
            self._conn.close()
        except KeyError:
            pass
        
        
    