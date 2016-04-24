#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <ctime>
#include <unistd.h>
#include <condition_variable>
#include <mutex>

#include <includes/data_types.h>
#include <includes/transactions.h>
#include "benchmark_common.h"

namespace po = boost::program_options;
using namespace diamond;
using namespace std;

DStringList players;
DLong score;
DCounter currentMove; //naming this variable "move" caused some weird collision with some stdlib function

std::mutex m;
std::condition_variable cv;
bool notificationReceived = true;
uint64_t prevTurnTime;

int main(int argc, char ** argv) {
    std::string configPrefix;
    std::string myName;
    std::string keyPrefix;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("name", po::value<std::string>(&myName)->required(), "name to use in the game (required)")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("keyprefix", po::value<std::string>(&keyPrefix)->required(), "key prefix (required)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help") || !vm.count("config")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    DiamondInit(configPrefix, 1, 0);
    StartTxnManager();

    // Map game state
    DObject::Map(players, keyPrefix + "100game:players");
    DObject::Map(score, keyPrefix + "100game:score");
    DObject::Map(currentMove, keyPrefix + "100game:move");

    // Add user to the game
    int committed = 0;
    while (!committed) {
        DObject::TransactionBegin();
        if (players.Index(myName) == -1) {
            players.Append(myName);
        }
        committed = DObject::TransactionCommit();
    }

    // Set up our reactive print outs
    uint64_t reactive_id = reactive_txn([myName] () {
        int value = score.Value();
        //cout << "Current total: " << value << endl;
        if (players.Size() > 0) {
            string cp = players[currentMove.Value() % players.Size()];  
            if (score.Value() >= 100) {
                //cout << cp << " won! Game Over!" << endl;
                sleep(2);
                exit(0);
            }
            else if (cp == myName) {
                //cout << " Enter number between 1 and 10: " << endl;
                {
                    std::unique_lock<std::mutex> lock(m);
                    notificationReceived = true;
                    cv.notify_all();
                }
            }
            else {
                //cout << " It's " << cp << "'s turn. " << endl;
            }
        }
    });

    // Cycle on user input
    prevTurnTime = currentTimeMillis();
    while (1) {
        while (!notificationReceived) {
            std::unique_lock<std::mutex> lock(m);
            cv.wait(lock);
        }
        notificationReceived = false;
        //cout << "Got a signal from the CV" << endl;

        int inc = 1;
        int committed = 0;
        while (!committed) {
            DObject::TransactionBegin();
	    string cp = players[currentMove.Value() % players.Size()];
	    // If it's the user's turn, make move
	    if (cp == myName && inc >= 1 && inc <= 10) {
                //cout << "It's my turn" << endl;
	        score += inc;
	        if (score.Value() < 100) {
                    ++currentMove;
                }
            }
            else {
                //cout << "It's " << cp << "'s turn (interactive txn)" << endl;
            }
            committed = DObject::TransactionCommit();
        }        

        uint64_t turnTime = currentTimeMillis();

        std::cout << prevTurnTime << "\t" << turnTime << std::endl;

        prevTurnTime = turnTime;
    }
    return 0;
}
