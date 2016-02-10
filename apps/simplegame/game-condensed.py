# THIS IS NOT A WORKING PYTHON PROGRAM
#
# It's an example of how game.py might look if we cut out boilerplate Python
# code and cleaned up the Diamond interface/bindings slightly

def playGame(myName, configFile):
    DiamondInit(configFile, 0, 1);

    players = DStringList("simplegame:players")
    move = DCounter("simplegame:move")

    ReactiveManager.txn_execute(addPlayer, players, myName)

    ReactiveManager.add(displayGame, players, move, myName)

    while True:
        inputWord = getCommandLineInput()
        if inputWord == "reset":
            ReactiveManager.txn_execute(resetGame, players, move)
        elif inputWord == "exit":
            sys.exit()
        else:
            action = int(inputWord)
            ReactiveManager.txn_execute(takeTurn, players, move, myName, action)

def resetGame(players, move):
    move.Set(0)
    for name in players.Members():
        score = DLong("simplegame:" + name + ":score")
        score.Set(0)

def addPlayer(players, name):
    if players.Index(name) == -1:
        players.Append(name)

def takeTurn(players, move, name, incr):
    currentPlayer = players.Value(move.Value() % players.Size())
    if currentPlayer == name:
        score = DLong("simplegame:" + name + ":score")
        score.Set(score.Value() + incr)
        move.Increment()

def displayGame(players, move, myName):
    for name in players.Members():
        score = DLong("simplegame:" + name + ":score")
        currentPlayer = players.Value(move.Value() % players.Size())
        print "Player: " + name + " score: " + repr(score.Value())
    if currentPlayer == myName:
        print "It's your turn! Enter your move:"
    else:
        print "It's " + currentPlayer + "'s turn"
