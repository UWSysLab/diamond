DIAMOND_SRC_DIR=$1
DIAMOND_BACKEND_DIR="$DIAMOND_SRC_DIR/backend"
PROJECT_DIR=$2

# build ARM binaries
cd $DIAMOND_BACKEND_DIR/build-arm
make

# build Java bindings
cd $DIAMOND_BACKEND_DIR/src/bindings/java
mvn package

# copy libraries and JARs into the Android project
cp $DIAMOND_BACKEND_DIR/build-arm/libdiamond.so $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/build-arm/hiredis/src/libhiredis.so $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/src/bindings/java/target/classes/arm-lib/libjniDiamond.so $PROJECT_DIR/libs/armeabi
cp $DIAMOND_BACKEND_DIR/src/bindings/java/libs/javacpp.jar $PROJECT_DIR/libs
cp $DIAMOND_BACKEND_DIR/src/bindings/java/target/diamond-1.0-SNAPSHOT.jar $PROJECT_DIR/libs
