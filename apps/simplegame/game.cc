#include "../../platform/includes/data_types.h"
#include <string>

DStringList players;
DLong score;
DCounter move;

using std;
using diamond;

void addPlayer(string name) {
   players.Append(name);
}

void takeTurn(string name, int inc) {
   string currentPlayer = players[move % players.Size()];
   if (currentPlayer == name) {
      score += inc;
      if (score == 100) {
         cout << "You won!\n";
      } else {
         move++;
      }
   }
}

void displayGame(string myName) {
   cout << "Current total: " << score;
   string currentPlayer = players[move % players.Size()];
  
   if (score >= 100) {
      cout << " Game Ended.\n";
   } else if (currentPlayer == myName) {
      cout << " It's your turn. Enter number between 1 and 10: ";
   } else {
      cout << "It's " << currentPlayer << "'s turn. \n";
   }
}

int main(int argc, char **argv) {
   DiamondInit(argv[1], 0, 1);
   
   string myName = string(argv[2]);
   int inc;
   
   map(players, "100game:players");
   map(score, "100game:score");
   map(move, "100game:move");
    
   TxnExecute(&addPlayer, myName);
   TxnReactive(&displayGame, myName);

   while (score < 100) {
      cin >> inc;
      TxnExecute(&takeTurn, inc);
   }

   return 0;
}
