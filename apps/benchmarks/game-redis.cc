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

#include <redox.hpp>

using namespace std;
using namespace redox;
namespace po = boost::program_options;
using namespace diamond;
using namespace std;

DStringList players;
DLong score;
DCounter currentMove; //naming this variable "move" caused some weird collision with some stdlib function

std::mutex m;
std::condition_variable cv;
bool notificationReceived = true;
bool done = false;
bool twoPlayers = false;
uint64_t prevTurnTime;

int main(int argc, char ** argv) {
    std::string configPrefix;
    std::string myName;
    std::string keyPrefix;
    bool noCaching = false;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("name", po::value<std::string>(&myName)->required(), "name to use in the game (required)")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("keyprefix", po::value<std::string>(&keyPrefix)->required(), "key prefix (required)")
        ("nocaching", po::bool_switch(&noCaching), "disable caching (enabled by default)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help") || !vm.count("config")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    vector<string> players;
    int sum, turn;

    string cp, myname = string(argv[2]);
    Redox rdx; Subscriber sub;

   // Set up redis and pubsub connections
   if (!rdx.connect(string(argv[1]), 6379)) exit(1);
   if (!sub.connect(string(argv[1]), 6379)) exit(1);

   rdx.command<int>({"ZADD", keyPrefix+"players", "1", myname},
                    cb);
   rdx.publish(keyPrefix + ":ping", "players");
   
   // Set up our print outs
   sub.subscribe("ping",
                 [&] (const string& topic, const string& msg) {
                    if (msg == "players") {
                       Command<vector<string>>& r =
                          rdx.commandSync<vector<string>>({"ZRANGE",
                                   keyPrefix+"players", "0", "-1"});
                       if (r.ok()) {
                          players = r.reply();
                       } else {
                          exit(1);
                       }
                    } else {
                       int numPlayers = players.Size();
                       Command<vector<string>>& r =
                          rdx.commandSync<vector<string>>({"MGET",
                                   keyPrefix+"sum",
                                   keyPrefix+"turn"});
                       if (r.ok()) {
                          sum = stoi(r.reply()[0]);
                          turn = stoi(r.reply()[1]);
                       } else {
                          exit(1);
                       }

                       if (numPlayers > 0) {
                          string cp = players[turn % numPlayers];
                          if (sum >= 100) {
                             //cout << cp << " won! Game Over!" << endl;
                             {
                                std::unique_lock<std::mutex> lock(m);
                                notificationReceived = true;
                                done = true;
                                cv.notify_all();
                             }
                          } else if (cp == myName) {
                             //cout << " Enter number between 1 and 10: " << endl;
                             {
                                std::unique_lock<std::mutex> lock(m);
                                notificationReceived = true;
                                if (numPlayers >= 2) {
                                   twoPlayers = true;
                                }
                                cv.notify_all();
                             }
                          }
                       }
                    }
                 });
                     
   // Cycle on user input
   while (1) {
      int inc; cin >> inc;
      cp = players[turn % players.size()];
      if (cp != myname) {
         cout << "Not your turn!\n"; continue;
      }
      if (inc < 1 || inc > 10) {
         cout << "Incorrect input\n"; continue;
      }
      sum += inc; if (sum < 100) turn++;
   }
   rdx.disconnect(); sub.disconnect(); return 0;

   
 
    // Cycle on user input
    prevTurnTime = currentTimeMillis();
    while (1) {
        while (!notificationReceived) {
           std::unique_lock<std::mutex> lock(m);
           cv.wait(lock);
        }
        notificationReceived = false;
        if (done) {
           break;
        }
        if (!twoPlayers) {
           continue;
        }
        //cout << "Got a signal from the CV" << endl;

        int inc = 1;
        string cp = players[move % players.Size()];
	    // If it's the user's turn, make move
	    if (cp == myName && inc >= 1 && inc <= 10) {
           //cout << "It's my turn" << endl;
           score += inc;
           if (score.Value() < 100) {
              ++move;
           }
        } else {
           //cout << "It's " << cp << "'s turn (interactive txn)" << endl;
        }
        rdx.command<int>({"INCRBY", keyPrefix+"sum",
                 to_string(inc)}, cb);
        rdx.command<int>({"INCR", keyPrefix+"turn"}, cb);
        rdx.publish(keyPrefix+"ping", "move");

        uint64_t turnTime = currentTimeMillis();
        
        std::cout << prevTurnTime << "\t" << turnTime << std::endl;

        prevTurnTime = turnTime;
    }
    sleep(5);
    return 0;
}
