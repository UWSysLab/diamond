DIAMOND_SRC_DIR=$1

if [ $# -ne 1 ]
    then echo "usage: ./build-everything.sh <diamond-src-dir>"
    exit
fi

./build-diamond-core.sh $DIAMOND_SRC_DIR
./build-apps.sh $DIAMOND_SRC_DIR
