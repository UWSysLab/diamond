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

diamond::DStringList players;
diamond::DLong score;
diamond::DCounter move; 

int main(int argc, char ** argv) {
    std::string configPrefix;
    std::string myName;

    // gather commandline options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help",
         "produce help message")
        ("name",
         po::value<std::string>(&myName)->required(),
         "name to use in the game")
        ("config",
         po::value<std::string>(&configPrefix)->required(),
         "frontend config file prefix (required)");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    //set up diamond
    diamond::DiamondInit(configPrefix, 1, 0);
    diamond::StartTxnManager();

    // Map game state
    diamond::DObject::Map(players, "100game:players");
    diamond::DObject::Map(score, "100game:score");
    diamond::DObject::Map(move, "100game:move");

    // Add user to the game
    diamond::execute_txn(
        [myName] () {
            if (players.Index(myName) == -1) {
                players.Append(myName);
            }},
        // txn callback: If transaction fails, exit
        [myName] (int committed) { 
            if (committed) {
                std::cout << "Welcome to the 100 game, " << myName << "!\n";	
            } else {
                std::cout << "Failed adding player to game\n";
                exit(1);
            }
        });

    // Set up our reactive print out
    uint64_t reactive_id = diamond::reactive_txn(
        [myName] () {
        if (players.Size() > 0) {
            std::cout << "======= Current Game Status =======\n";
            std::cout << "Total = " << score.Value() << "\n";
            std::cout << "Current players: ";
            for (auto &p : players.Members())
                std::cout << p << " ";
            std::cout << "\n";

            std::string cp = players[move.Value() % players.Size()];  
            if (score.Value() >= 100)
                std::cout << cp << " won! Game Over!\n";
            else if (cp == myName)
                std::cout << "Enter number between 1 and 10: \n";
            else
                std::cout << "It's " << cp << "'s turn. \n";
            std::cout << "===================================\n";
        }
    });

    // Cycle on user input
    while (1) {
        int inc = 0;
        std::cin >> inc;
        diamond::execute_txn(
            [myName, inc] () {
                // If it's the user's turn, make move
                std::string cp = players[move.Value() % players.Size()];                
                if (cp == myName && inc >= 1 && inc <= 10) {
                    score += inc;
                    if (score.Value() < 100) ++move;
                } else {
                    diamond::abort_txn();
                }
            },
            // txn callback: If we can't make a move, just exit
            [reactive_id] (int committed) {
                if (!committed) {
                    std::cout << "Taking turn failed\n";
                    diamond::reactive_stop(reactive_id);
                    exit(1);
                }
            });
    }
    return 0;
}
