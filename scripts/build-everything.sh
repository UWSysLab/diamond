DIAMOND_SRC_DIR=$1
PROJECT_DIR=$2

if [ $# -ne 1 ]
    then echo "usage: ./build-everything.sh <diamond-src-dir>"
    exit
fi

./build-diamond-core.sh $DIAMOND_SRC_DIR

./build-diamond-java.sh $DIAMOND_SRC_DIR $DIAMOND_SRC_DIR/apps/test-apps/DiamondJavaTest
./build-diamond-java.sh $DIAMOND_SRC_DIR $DIAMOND_SRC_DIR/apps/twitter/backend-diamond
./build-diamond-java.sh $DIAMOND_SRC_DIR $DIAMOND_SRC_DIR/apps/chat/DesktopChat

./build-diamond-android.sh $DIAMOND_SRC_DIR $DIAMOND_SRC_DIR/apps/test-apps/DiamondAndroidTest
./build-diamond-android.sh $DIAMOND_SRC_DIR $DIAMOND_SRC_DIR/apps/twitter/twimight-diamond
./build-diamond-android.sh $DIAMOND_SRC_DIR $DIAMOND_SRC_DIR/apps/chat/DiMessage
./build-diamond-android.sh $DIAMOND_SRC_DIR $DIAMOND_SRC_DIR/apps/chat/AndroidChatBenchmark
