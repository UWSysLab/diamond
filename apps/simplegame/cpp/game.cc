#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

#include <includes/data_types.h>
#include <includes/transactions.h>

namespace po = boost::program_options;
using namespace std;
using namespace diamond;

DStringList players;
DLong score;
DCounter turn; 

int main(int argc, char ** argv) {
    string configPrefix;
    string myName, key;

    // gather commandline options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help",
         "produce help message")
        ("key",
         po::value<std::string>(&key)->required(),
         "game name")
        ("name",
         po::value<std::string>(&myName)->required(),
         "name to use in the game")
        ("config",
         po::value<std::string>(&configPrefix)->required(),
         "frontend config file prefix (required)");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        cout << desc << endl;
        exit(1);
    }
    po::notify(vm);

    //set up diamond
    DiamondInit(configPrefix, 1, 0);
    StartTxnManager();

    // Map game state
    DObject::Map(players, key + "100game:players");
    DObject::Map(score, key + "100game:score");
    DObject::Map(turn, key + "100game:turn");

    // Add user to the game
    execute_txn([myName] () {
            if (players.Index(myName) == -1) players.Append(myName);
        },
        // txn callback: If transaction fails, exit
        [myName] (bool committed) { 
            if (!committed) exit(1);
        });

    // Set up our reactive display
    uint64_t reactive_id =
        reactive_txn([myName] () {
                // Current player whose turn it is
                string cp = players[turn.Value() % players.Size()];
                cout << endl << "====== Welcome to 100 game, ";
                cout << myName << " ======" << endl;
                if (score.Value() >= 100) {
                    // if score over 100, game is over
                    cout << cp << "won the 100 game!";
                    exit(0);
                }
                // print out current store and players
                cout << "Total: " << score.Value() << endl;
                cout << "Turn: " << cp << endl;
                cout << "Players: ";
                for (auto &p : players.Members())
                    cout << p << " ";
                cout << endl;
                // print out a prompt if it is my turn
                if (cp == myName)
                    cout << "> Enter number between 1 and 10: " << flush;
            });

    // Cycle on user input
    while (1) {
        int inc = 0; cin >> inc;
        if (inc >= 1 && inc <= 10) {
            execute_txn([myName, inc] () {
                    // If it's the user's turn, make move
                    if (players[turn.Value() % players.Size()] == myName) {
                        score += inc;
                        if (score.Value() < 100) ++turn;
                    } else {
                        abort_txn();
                    }
                },
                // txn callback: If we can't make a move, just exit
                [] (bool committed) {
                    if (!committed) exit(1);
                });
        } else {
            cout << "Invalid input";
            exit(1);
        }
    }
    return 0;
}
