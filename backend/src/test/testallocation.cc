#include "storage/cloud.h"
#include "includes/data_types.h"

using namespace diamond;

int main(void){
    DiamondInit();

    DLong long1;
    DObject::Map(long1, "unusedkey");

    long lastReadValue = 0;

    int committed = 0;
    while(true) {
        DObject::TransactionBegin();
        if (lastReadValue == long1.Value()) {
            DObject::TransactionRetry();
            continue;
        }
        lastReadValue = long1.Value();
        DObject::TransactionCommit();
    }

    return 0;
}

