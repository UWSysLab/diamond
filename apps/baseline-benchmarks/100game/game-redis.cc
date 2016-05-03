#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <ctime>
#include <unistd.h>
#include <condition_variable>
#include <mutex>

#include "benchmark_common.h"

#include <redox.hpp>

using namespace std;
using namespace redox;
namespace po = boost::program_options;

std::mutex m;
std::condition_variable cv;
bool notificationReceived = true;
bool done = false;
bool twoPlayers = false;
uint64_t prevTurnTime;

int main(int argc, char ** argv) {
   std::string host;
   std::string myName;
   std::string keyPrefix;

    po::options_description desc("Allowed options");
    desc.add_options()
       ("help", "produce help message")
       ("name", po::value<std::string>(&myName)->required(), "name to use in the game (required)")
       ("host", po::value<std::string>(&host)->required(), "Redis host (required)")
       ("keyprefix", po::value<std::string>(&keyPrefix)->required(), "key prefix (required)");
       
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help") || !vm.count("config")) {
       std::cout << desc << std::endl;
       return 1;
    }
    po::notify(vm);

    vector<string> players;
    int sum, turn;

    string cp;
    Redox rdx; Subscriber sub;

    auto cb = [](Command<int>& c) {
       if(!c.ok()) {
          exit(1);
       }
    };
    
   // Set up redis and pubsub connections
   if (!rdx.connect(host, 6379)) exit(1);
   if (!sub.connect(host, 6379)) exit(1);

   rdx.command<int>({"ZADD", keyPrefix+":players", "1", myName}, cb);
   rdx.publish(keyPrefix + ":ping", "players");
   
   // Set up our print outs
   sub.subscribe("ping",
                 [&] (const string& topic, const string& msg) {
                    if (msg == "players") {
                       Command<vector<string>>& r =
                          rdx.commandSync<vector<string>>({"ZRANGE",
                                   keyPrefix+":players", "0", "-1"});
                       if (r.ok()) {
                          players = r.reply();
                       } else {
                          exit(1);
                       }
                    } else {
                       int numPlayers = players.size();
                       Command<vector<string>>& r =
                          rdx.commandSync<vector<string>>({"MGET",
                                   keyPrefix+":sum",
                                   keyPrefix+":turn"});
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
       string cp = players[turn % players.size()];
       // If it's the user's turn, make move
       if (cp == myName && inc >= 1 && inc <= 10) {
          //cout << "It's my turn" << endl;
          rdx.command<int>({"INCRBY", keyPrefix+":sum",
                   to_string(inc)}, cb);
	  
          if (sum < 100) {
             rdx.command<int>({"INCR", keyPrefix+":turn"}, cb);
          }
          rdx.publish(keyPrefix+"ping", "move");
       }
       uint64_t turnTime = currentTimeMillis();
        
       std::cout << prevTurnTime << "\t" << turnTime << std::endl;

       prevTurnTime = turnTime;
    }
    sleep(5);
    return 0;
}
