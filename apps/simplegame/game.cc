#include "../../platform/includes/data_types.h"
#include <string>

diamond::DStringList players;
diamond::DLong score;
diamond::DCounter move;

void addPlayer(std::string name) {
   players.Append(name);
}

void takeTurn(std::string name, int inc) {
   std::string currentPlayer = players[move % players.Size()];
   if (currentPlayer == name) {
      score += inc;
      if (score == 100) {
         cout << "You won!\n";
      } else {
         move++;
      }
   } else {
      diamond::abort_txn();
   }
}

void displayGame(std::string myName) {
   cout << "Current total: " << score;
   std::string currentPlayer = players[move % players.Size()];
  
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
   
   std::string myName = std::string(argv[2]);
   int inc;
   
   diamond::map(players, "100game:players");
   diamond::map(score, "100game:score");
   diamond::map(move, "100game:move");
    
   diamond::execute_txn(&addPlayer, myName);
   diamond::reactive_txn(&displayGame, myName);

   while (score < 100) {
      cin >> inc;
      diamond::execute_txn(&takeTurn, inc);
   }

   return 0;
}
