#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <includes/data_types.h>
#include "benchmark_common.h"

namespace po = boost::program_options;
using namespace diamond;

int main(int argc, char ** argv) {
    std::string configPrefix;
    std::string keyFile;
    int nKeys = 0;
    int numSeconds = 10;
    bool increment = false;
    bool printKeys = false;
    double readFrac = 0.0;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("keys", po::value<std::string>(&keyFile)->required(), "file from which to read keys (required)")
        ("numkeys", po::value<int>(&nKeys)->required(), "number of keys to read (required)")
        ("time", po::value<int>(&numSeconds), "number of seconds to run (default 10)")
        ("readfrac", po::value<double>(&readFrac), "fraction of operations that should be reads")
        ("increment", po::bool_switch(&increment), "use ++ operator instead of Value() and Set()")
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
    initRand();

    std::vector<std::string> keys;
    parseKeys(keyFile, nKeys, keys);

    std::vector<DCounter> dcounters;
    for (auto &key : keys) {
        DCounter dcounter;
        DObject::Map(dcounter, key);
        dcounters.push_back(dcounter);
    }

    bool done = false;
    uint64_t globalStartTime = currentTimeMillis();

    std::cout << "start-time\tend-time\tcommitted\toperation\t";
    if (printKeys) {
        std::cout << "key\t";
    }
    std::cout << std::endl;

    while (!done) {
        std::string val(std::to_string(randInt(0, 1000000)));
        int varIndex = randInt(0, dcounters.size() - 1);
        bool doRead = (randDouble(0, 1) <= readFrac);

        uint64_t startTime = currentTimeMillis();

        // Read from and write to a randomly chosen key from among the input keys
        DObject::TransactionBegin();
        if (doRead) {
            uint64_t temp = dcounters[varIndex].Value();
        }
        else {
            if (increment) {
                ++dcounters[varIndex];
            }
            else {
                uint64_t temp = dcounters[varIndex].Value();
                dcounters[varIndex].Set(temp + 1);
            }
        }
        int committed = DObject::TransactionCommit();

        uint64_t endTime = currentTimeMillis();

        std::cout << startTime << "\t"
                  << endTime << "\t"
                  << committed << "\t";
        if (doRead) {
            std::cout << "read" << "\t";
        }
        else {
            if (increment) {
                std::cout << "increment" << "\t";
            }
            else {
                std::cout << "read-write" << "\t";
            }
        }
        if (printKeys) {
            std::cout << keys[varIndex] << "\t";
        }
        std::cout << std::endl;

        double runtimeSeconds = (endTime - globalStartTime) / 1000.0;
        done = (runtimeSeconds >= numSeconds);
    }
}
