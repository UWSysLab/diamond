#include "includes/diamondnotify.h"

#include <iostream>

#include "lib/message.h"

void notifyReactiveUpcall() {
    std::cout << "Reactive call into C++ code\n" << std::endl;
}
