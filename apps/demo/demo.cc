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

DString shared;

bool setup(int argc, char **argv, string &key) {
    string configPrefix;
    // gather commandline options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help",
         "produce help message")
        ("key",
         po::value<std::string>(&key)->required(),
         "shared key to use")
        ("config",
         po::value<std::string>(&configPrefix)->required(),
         "frontend config file prefix (required)");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        cout << desc << endl;
        return false;
    }
    po::notify(vm);

    //set up diamond
    DiamondInit(configPrefix, 1, 0);
    StartTxnManager();
    return true;
}


int main(int argc, char **argv) {
    string key;
    if (!setup(argc, argv, key)) {
        exit(1);
    }
    
    // Map game state
    DObject::Map(shared, key);

    // Set up our reactive display
    uint64_t reactive_id =
        reactive_txn([] () {
                if (shared.Value() != "") {
                    cout << endl << "current status: " << shared.Value() << endl;
                }
                cout << "set status: " << flush;
            });

    // Cycle on user input
    while (1) {
        string in = "";
        cin >> in;
        if (in != "") {
            execute_txn([in] () {
                    shared = in;
                }, [] (bool committed) { if (!committed) exit(1);});
        }
    }
    return 0;
}
