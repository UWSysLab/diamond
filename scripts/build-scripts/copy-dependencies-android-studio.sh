DIAMOND_SRC_DIR=$1
DIAMOND_BACKEND_DIR="$DIAMOND_SRC_DIR/platform"
PROJECT_DIR=$2

if [ $# -ne 2 ]
    then echo "usage: ./copy-dependencies-android-studio.sh <diamond-src-dir> <android-project-dir>"
    exit
fi

MAIN_DIR=$PROJECT_DIR/app/src/main

# copy libraries and JARs into the Android project
mkdir -p $MAIN_DIR/libs
mkdir -p $MAIN_DIR/jniLibs/armeabi
cp $DIAMOND_BACKEND_DIR/build-arm/libdiamond.so $MAIN_DIR/jniLibs/armeabi
cp $DIAMOND_SRC_DIR/external/protobuf-2.5.0/src/.libs/libprotobuf.so $MAIN_DIR/jniLibs/armeabi
cp $DIAMOND_SRC_DIR/external/libevent/.libs/libevent_core.so $MAIN_DIR/jniLibs/armeabi
cp $DIAMOND_SRC_DIR/external/libevent/.libs/libevent_pthreads.so $MAIN_DIR/jniLibs/armeabi
cp $DIAMOND_SRC_DIR/platform/toolchains/android/toolchain/arm-linux-androideabi/lib/libgnustl_shared.so $MAIN_DIR/jniLibs/armeabi/
cp $DIAMOND_BACKEND_DIR/bindings/java/target/classes/arm-lib/libjniDiamond.so $MAIN_DIR/jniLibs/armeabi
cp $DIAMOND_BACKEND_DIR/bindings/java/target/classes/arm-lib/libjniDiamondUtil.so $MAIN_DIR/jniLibs/armeabi
cp $DIAMOND_BACKEND_DIR/bindings/java/libs/javacpp.jar $MAIN_DIR/libs
cp $DIAMOND_BACKEND_DIR/bindings/java/target/diamond-1.0-SNAPSHOT.jar $MAIN_DIR/libs
