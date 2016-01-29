#include <boost/program_options.hpp>
#include <includes/data_types.h>

namespace po = boost::program_options;
using namespace diamond;

int main(int argc, char ** argv) {
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("config", po::value<std::string>(), "specify client config file (required)")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help") || !vm.count("config")) {
        std::cout << desc << std::endl;
        return 1;
    }

    DiamondInit(vm["config"].as<std::string>(), 1, 0);

    DString str1;
    DObject::TransactionBegin();
    DObject::Map(str1, "benchmark:str1");
    str1.Set("testing");
    std::cout << str1.Value() << std::endl;
    DObject::TransactionCommit();
}
