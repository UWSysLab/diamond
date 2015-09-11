DIAMOND_SRC_DIR=$1
DIAMOND_BACKEND_DIR="$DIAMOND_SRC_DIR/backend"
PROJECT_DIR=$2

if [ $# -ne 2 ]
    then echo "usage: ./build-diamond-java.sh <diamond-src-dir> <java-project-dir>"
    exit
fi

# build Java binaries
cd $DIAMOND_BACKEND_DIR/build
make

# build Java bindings
cd $DIAMOND_BACKEND_DIR/src/bindings/java
mvn package

# copy libraries and JARs into the Java project
cp $DIAMOND_BACKEND_DIR/build/libdiamond.so $PROJECT_DIR/libs
cp $DIAMOND_BACKEND_DIR/src/bindings/java/target/classes/x86-lib/libjniDiamond.so $PROJECT_DIR/libs
cp $DIAMOND_BACKEND_DIR/src/bindings/java/libs/javacpp.jar $PROJECT_DIR/libs
cp $DIAMOND_BACKEND_DIR/src/bindings/java/target/diamond-1.0-SNAPSHOT.jar $PROJECT_DIR/libs
