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
    bool printKeys = false;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("keys", po::value<std::string>(&keyFile)->required(), "file from which to read keys (required)")
        ("numkeys", po::value<int>(&nKeys)->required(), "number of keys to read (required)")
        ("time", po::value<int>(&numSeconds), "number of seconds to run (default 10)")
        ("printkeys", po::bool_switch(&printKeys), "number of seconds to run (default 10)")
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

    std::vector<DString> dstrings;
    for (auto &key : keys) {
        DString dstring;
        DObject::Map(dstring, key);
        dstrings.push_back(dstring);
    }

    std::string uniqueKey = getRandomKey();
    DString uniqueString;
    DObject::Map(uniqueString, uniqueKey);

    bool done = false;
    uint64_t globalStartTime = currentTimeMillis();

    std::cout << "start-time\tend-time\tcommitted\t";
    if (printKeys) {
        std::cout << "read-key\twrite-key\t";
    }
    std::cout << std::endl;

    while (!done) {
        std::string val(std::to_string(randInt(0, 1000000)));
        int varIndex = randInt(0, dstrings.size() - 1);

        uint64_t startTime = currentTimeMillis();

        // Read from a (randomly chosen) known key and write to the client's unique key
        DObject::TransactionBegin();
        std::string temp = dstrings[varIndex].Value();
        uniqueString.Set(val);
        int committed = DObject::TransactionCommit();

        uint64_t endTime = currentTimeMillis();

        std::cout << startTime << "\t"
                  << endTime << "\t"
                  << committed << "\t";
        if (printKeys) {
            std::cout << keys[varIndex] << "\t"
                      << uniqueKey << "\t";
        }
        std::cout << std::endl;

        double runtimeSeconds = (endTime - globalStartTime) / 1000.0;
        done = (runtimeSeconds >= numSeconds);
    }
}
