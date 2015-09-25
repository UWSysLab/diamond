DIAMOND_SRC_DIR=$1
DIAMOND_BACKEND_DIR="$DIAMOND_SRC_DIR/backend"
PROJECT_DIR=$2

if [ $# -ne 2 ]
    then echo "usage: ./build-diamond-android.sh <diamond-src-dir> <android-project-dir>"
    exit
fi

# build ARM binaries
cd $DIAMOND_BACKEND_DIR/build-arm
make

# build Java bindings
cd $DIAMOND_BACKEND_DIR/src/bindings/java
mvn package

# copy libraries and JARs into the Android project
mkdir -p $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/build-arm/libdiamond.so $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/build-arm/hiredis/src/libhiredis.so $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/external/libuv/android/libuv.a $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/src/bindings/java/target/classes/arm-lib/libjniDiamond.so $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/src/bindings/java/target/classes/arm-lib/libjniDiamondUtil.so $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/src/bindings/java/libs/javacpp.jar $PROJECT_DIR/libs
cp $DIAMOND_BACKEND_DIR/src/bindings/java/target/diamond-1.0-SNAPSHOT.jar $PROJECT_DIR/libs
