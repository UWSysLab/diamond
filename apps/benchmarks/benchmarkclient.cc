#include <boost/program_options.hpp>
#include <cstdlib>
#include <ctime>
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
        DObject::Map(dstrings[i], "benchmark:var" + i);
    }

    bool done = false;
    time_t startTime = time(NULL);
    int numTransactions = 0;
    while (!done) {
        DObject::TransactionBegin();
        for (int j = 0; j < numVarsRead; j++) {
            int varIndex = rand() % numVarsTotal;
            std::string val("testing");
            dstrings[varIndex].Set(val);
        }
        for (int k = 0; k < numVarsWrite; k++) {
            int varIndex = rand() % numVarsTotal;
            std::string temp = dstrings[varIndex].Value();
        }
        DObject::TransactionCommit();

        time_t currentTime = time(NULL);
        double seconds = difftime(currentTime, startTime);
        done = (seconds >= numSeconds);
    }

    std::cout << numTransactions << std::endl;
}
