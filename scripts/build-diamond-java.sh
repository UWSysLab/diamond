DIAMOND_SRC_DIR=$1
DIAMOND_BACKEND_DIR="$DIAMOND_SRC_DIR/backend"
PROJECT_DIR=$2

if [ $# -ne 2 ]
    then echo "usage: ./build-diamond-java.sh <diamond-src-dir> <java-project-dir>"
    exit
fi

# copy libraries and JARs into the Java project
mkdir -p $PROJECT_DIR/libs
cp $DIAMOND_BACKEND_DIR/build/libdiamond.so $PROJECT_DIR/libs
cp $DIAMOND_BACKEND_DIR/src/bindings/java/target/classes/x86-lib/libjniDiamond.so $PROJECT_DIR/libs
cp $DIAMOND_BACKEND_DIR/src/bindings/java/target/classes/x86-lib/libjniDiamondUtil.so $PROJECT_DIR/libs
cp $DIAMOND_BACKEND_DIR/src/bindings/java/libs/javacpp.jar $PROJECT_DIR/libs
cp $DIAMOND_BACKEND_DIR/src/bindings/java/target/diamond-1.0-SNAPSHOT.jar $PROJECT_DIR/libs
