#include <jni.h>
#include <dlfcn.h>
#include "jniDiamond.h"
#include "diamondnotify.h"

void notifyReactiveUpcall() {
    notifyReactive();
}
