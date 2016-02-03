#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include <includes/data_types.h>

namespace po = boost::program_options;
using namespace diamond;

int main(int argc, char ** argv) {
    std::string configPrefix;
    int numVarsTotal = 10;
    int numVarsRead = 2;
    int numVarsWrite = 2;
    int numSeconds = 10;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
        ("numvarstotal", po::value<int>(&numVarsTotal), "total number of shared variables to use")
        ("numvarsread", po::value<int>(&numVarsRead), "number of shared variables to read per transaction")
        ("numvarswrite", po::value<int>(&numVarsWrite), "number of shared variables to write per transaction")
        ("time", po::value<int>(&numSeconds), "number of seconds to run")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    DiamondInit(configPrefix, 1, 0);

    DString dstrings[numVarsTotal];
    for (int i = 0; i < numVarsTotal; i++) {
        std::string key("benchmark:var" + std::to_string(i));
        DObject::Map(dstrings[i], key);
    }

    bool done = false;
    time_t startTime = time(NULL);
    int numTransactions = 0;
    int totalNumAborts = 0;
    int numAbandoned = 0;
    while (!done) {
        int numAborts = 0;
        int committed = 0;
        bool doneTrying = false;
        std::string val("testing " + std::to_string(rand()));
        while (!committed && !doneTrying) {
            DObject::TransactionBegin();
            for (int j = 0; j < numVarsRead; j++) {
                int varIndex = rand() % numVarsTotal;
                std::string temp = dstrings[varIndex].Value();
            }
            for (int k = 0; k < numVarsWrite; k++) {
                int varIndex = rand() % numVarsTotal;
                dstrings[varIndex].Set(val);
            }
            committed = DObject::TransactionCommit();
            if (!committed) {
                numAborts++;
            }
            if (numAborts >= 100) {
                doneTrying = true;
            }
        }
        if (doneTrying) {
            numAbandoned++;
        }
        else {
            numTransactions++;
        }
        totalNumAborts += numAborts;

        time_t currentTime = time(NULL);
        double seconds = difftime(currentTime, startTime);
        done = (seconds >= numSeconds);
    }

    std::cout << "Transactions committed: " << numTransactions << std::endl;
    std::cout << "Abandoned transactions (aborted 100 times): " << numAbandoned << std::endl;
    std::cout << "Total num aborts: " << totalNumAborts << std::endl;
}
