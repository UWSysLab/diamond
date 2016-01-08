'''
Use me to upgrade the old shelve-backed database to ZODB
'''
import shelve
from pyscrabble import db
from pyscrabble import manager

USER_LIST_LOCATION         = 'pyscrabble.users.list'
MESSAGES_LOCATION          = 'messages.list'
GAME_LIST_LOCATION         = 'pyscrabble.games.list'
SERVER_STATS_LOCATION      = 'server.stats.list'

r = manager.ResourceManager()
db = db.DB()

s = shelve.open(r["config"][USER_LIST_LOCATION], writeback = True)
for k,v in s.iteritems():
    db.users[k] = v
s.close()

s = shelve.open(r["config"][MESSAGES_LOCATION], writeback = True)
for k,v in s.iteritems():
    db.messages[k] = v
s.close()

s = shelve.open(r["config"][GAME_LIST_LOCATION], writeback = True)
for k,v in s.iteritems():
    db.games[k] = v
s.close()

s = shelve.open(r["config"][SERVER_STATS_LOCATION], writeback = True)
for k,v in s.iteritems():
    db.stats[k] = v
s.close()

db.sync()
print 'Done'