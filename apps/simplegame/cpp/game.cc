#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <ctime>
#include <unistd.h>

#include <includes/data_types.h>
#include <includes/transactions.h>

namespace po = boost::program_options;
using namespace diamond;
using namespace std;

DStringList players;
DLong score;
DCounter currentMove; //naming this variable "move" caused some weird collision with some stdlib function

int main(int argc, char ** argv) {
    std::string configPrefix;
    std::string myName;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("name", po::value<std::string>(&myName)->required(), "name to use in the game")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    DiamondInit(configPrefix, 1, 0);
    StartTxnManager();

    // Map game state
    DObject::Map(players, "100game:players");
    DObject::Map(score, "100game:score");
    DObject::Map(currentMove, "100game:move");

    // Add user to the game
    int ret = execute_txn([myName] () {
        if (players.Index(myName) == -1) {
            players.Append(myName);
        }
    });
    // If transaction fails, exit
    // TODO: note from Niel: does ret have any meaningful info, or should we use a callback here?
    if (ret != 0) return -1;

    // Set up our reactive print outs
    txn_id id = reactive_txn([myName] () {
         cout << "Current total: " << score.Value() << "\n";
         string cp = players[currentMove.Value() % players.Size()];  
         if (score.Value() >= 100)
            cout << cp << " won! Game Over!\n";
         else if (cp == myName)
            cout << " Enter number between 1 and 10: \n";
         else
            cout << " It's " << cp << "'s turn. \n";
    });

    // Cycle on user input
    while (1) {
        int inc = 0;
        cin >> inc;
        int ret = execute_txn([myName, inc] () {
	    string cp = players[currentMove.Value() % players.Size()];
	    // If it's the user's turn, make move
	    if (cp == myName && inc >= 1 && inc <= 10) {
	       score += inc;
	       if (score.Value() < 100) ++currentMove;
	    } else {
	       abort_txn();
	    }
	});
        // If we can't make a move, just exit
        if (ret != 0) {
	   reactive_stop(id);
	   return -1;
        }
    }
    return 0;
}
