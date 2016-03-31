#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <includes/data_types.h>
#include <includes/transactions.h>
#include "benchmark_common.h"

namespace po = boost::program_options;
using namespace diamond;

std::vector<DString> dstrings;

int main(int argc, char ** argv) {
    std::string configPrefix;
    std::string keyFile;
    int nKeys = 0;
    int numSeconds = 10;
    bool printKeys = false;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("keys", po::value<std::string>(&keyFile)->required(), "file from which to read keys (required)")
        ("numkeys", po::value<int>(&nKeys)->required(), "number of keys to subscribe to (required)")
        ("time", po::value<int>(&numSeconds), "number of seconds to run (default 10)")
        ("printkeys", po::bool_switch(&printKeys), "print key accessed on each transaction")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help") || !vm.count("config") || !vm.count("keys") || !vm.count("numkeys")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    DiamondInit(configPrefix, 1, 0);
    StartTxnManager();
    initRand();

    std::vector<std::string> keys;
    parseKeys(keyFile, nKeys, keys);

    for (auto &key : keys) {
        DString dstring;
        DObject::Map(dstring, key);
        dstrings.push_back(dstring);
    }

    for (int i = 0; i < dstrings.size(); i++) {
        reactive_txn([i, keys, printKeys] () {
            uint64_t latency = currentTimeMillis() - atol(dstrings[i].Value().c_str());
            std::cout << latency << "\t";
            if (printKeys) {
                std::cout << keys[i] << "\t";
            }
            std::cout << std::endl;
        });

    }

    bool done = false;
    uint64_t globalStartTime = currentTimeMillis();

    std::cout << "latency\t";
    if (printKeys) {
        std::cout << "key\t";
    }
    std::cout << std::endl;

    while (!done) {
        sleep(1);
        double runtimeSeconds = (currentTimeMillis() - globalStartTime) / 1000.0;
        done = (runtimeSeconds >= numSeconds);
    }
}
