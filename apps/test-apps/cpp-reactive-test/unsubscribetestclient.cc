#include <iostream>
#include <string>
#include <unistd.h>
#include <boost/program_options.hpp>

#include <includes/data_types.h>
#include <includes/transactions.h>

using namespace diamond;
namespace po = boost::program_options;

DString dstrs[10];
int curStr = 0;

void callback() {
    std::cout << "Callback fired" << std::endl;
}

int main(int argc, char ** argv) {
    std::string configPrefix;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(&configPrefix)->required(), "frontend config file prefix (required)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }
    po::notify(vm);

    DiamondInit(configPrefix, 1, 0);
    StartTxnManager();
    DObject::NotificationInit(&callback);

    for (int i = 0; i < 10; i++) {
        DObject::Map(dstrs[i], "unsubscribetest:str" + std::to_string(i));
    }

    uint64_t reactive_id = reactive_txn([] () {
        std::cout << "String " << curStr << " value: " << dstrs[curStr].Value() << std::endl;
        curStr++;
    });

    while (1) {
        sleep(1);
    }
    return 0;
}
