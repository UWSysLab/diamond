DIAMOND_SRC_DIR=$(cd ..; pwd)
DIAMOND_BACKEND_DIR="$DIAMOND_SRC_DIR/backend"

# build binaries
cd $DIAMOND_BACKEND_DIR/build
make -j8
cd $DIAMOND_BACKEND_DIR/build-arm
make -j8

# build Java bindings
cd $DIAMOND_BACKEND_DIR/src/bindings/java
mvn clean
mvn package
