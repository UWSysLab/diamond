DIAMOND_SRC_DIR=$1
DIAMOND_BACKEND_DIR="$DIAMOND_SRC_DIR/platform"
PROJECT_DIR=$2

if [ $# -ne 2 ]
    then echo "usage: ./build-diamond-android.sh <diamond-src-dir> <android-project-dir>"
    exit
fi

# copy libraries and JARs into the Android project
mkdir -p $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/build-arm/libdiamond.so $PROJECT_DIR/libs/armeabi
cp $DIAMOND_SRC_DIR/external/protobuf-2.5.0/src/.libs/libprotobuf.so $PROJECT_DIR/libs/armeabi/libprotobuf.so
cp $DIAMOND_SRC_DIR/external/libevent/.libs/libevent_core.so $PROJECT_DIR/libs/armeabi/libevent_core.so
cp $DIAMOND_SRC_DIR/external/libevent/.libs/libevent_pthreads.so $PROJECT_DIR/libs/armeabi/libevent_pthreads.so
cp $DIAMOND_BACKEND_DIR/bindings/java/target/classes/arm-lib/libjniDiamond.so $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/bindings/java/target/classes/arm-lib/libjniDiamondUtil.so $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/bindings/java/libs/javacpp.jar $PROJECT_DIR/libs
cp $DIAMOND_BACKEND_DIR/bindings/java/target/diamond-1.0-SNAPSHOT.jar $PROJECT_DIR/libs
