#!/usr/bin/python

import argparse
import sys
sys.path.append("../../platform/build/bindings/python/")
sys.path.append("../../platform/bindings/python/")
from libpydiamond import *
import ReactiveManager

def main():
    parser = argparse.ArgumentParser(description='Play a silly game.')
    parser.add_argument('name', help='your name')
    parser.add_argument('config_prefix', help='frontend config file prefix')
    args = parser.parse_args()

    DiamondInit(args.config_prefix, 0, 1);
    ReactiveManager.start()

    myName = args.name

    players = DStringList()
    scores = DList()
    move = DCounter()
    DStringList.Map(players, "simplegame:players")
    DList.Map(scores, "simplegame:scores")
    DCounter.Map(move, "simplegame:move")

    ReactiveManager.txn_execute(addPlayer, players, scores, myName)

    ReactiveManager.add(displayGame, players, scores, move, myName)

    while True:
        words = sys.stdin.readline().split()
        if words[0].lower() == "reset".lower():
            ReactiveManager.txn_execute(resetGame, players, scores, move)
        elif words[0].lower() == "exit".lower():
            sys.exit()
        else:
            action = int(words[0])
            ReactiveManager.txn_execute(takeTurn, players, scores, move, myName, action)

def resetGame(players, scores, move):
    move.Set(0)
    for i in range(scores.Size()):
        scores.Set(i, 0)

def addPlayer(players, scores, name):
    if players.Index(name) == -1:
        players.Append(name)
        scores.Append(0)

def takeTurn(players, scores, move, name, incr):
    currentIndex = move.Value() % players.Size()
    currentPlayer = players.Value(currentIndex)
    if currentPlayer == name:
        scores.Set(currentIndex, scores.Value(currentIndex) + incr)
        move.Set(move.Value() + 1)

def displayGame(players, scores, move, myName):
    for i in range(players.Size()):
        print "Player: " + players.Value(i) + " score: " + repr(scores.Value(i))
    if players.Size() > 0:
        currentPlayer = players.Value(move.Value() % players.Size())
        if currentPlayer == myName:
            print "It's your turn! Enter your move:"
        else:
            print "It's " + currentPlayer + "'s turn"

if __name__ == "__main__": main()
