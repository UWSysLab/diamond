DIAMOND_SRC=/home/nl35/research/diamond-src
JRE_DIR=/home/nl35/research/jdk1.8.0_60/jre

BASELINE_SERVER_DIR=$DIAMOND_SRC/apps/chat/BaselineChatServer
BASELINE_CLIENT_DIR=$DIAMOND_SRC/apps/chat/BaselineChatClient
DIAMOND_CLIENT_DIR=$DIAMOND_SRC/apps/chat/DesktopChat

mkdir chat-program-package
mkdir chat-program-package/baseline-server
mkdir chat-program-package/baseline-server/libs
mkdir chat-program-package/baseline-server/bin
cp $BASELINE_SERVER_DIR/libs/* chat-program-package/baseline-server/libs
cp $BASELINE_SERVER_DIR/bin/* chat-program-package/baseline-server/bin
cp baseline-server-package-wrapper.sh chat-program-package/baseline-server

mkdir chat-program-package/baseline-client
mkdir chat-program-package/baseline-client/libs
mkdir chat-program-package/baseline-client/bin
cp $BASELINE_CLIENT_DIR/libs/* chat-program-package/baseline-client/libs
cp $BASELINE_CLIENT_DIR/bin/* chat-program-package/baseline-client/bin
cp baseline-client-package-wrapper.sh chat-program-package/baseline-client

mkdir chat-program-package/diamond-client
mkdir chat-program-package/diamond-client/libs
mkdir chat-program-package/diamond-client/bin
cp $DIAMOND_CLIENT_DIR/libs/* chat-program-package/diamond-client/libs
cp $DIAMOND_CLIENT_DIR/bin/* chat-program-package/diamond-client/bin
cp diamond-client-package-wrapper.sh chat-program-package/diamond-client
cp $DIAMOND_SRC/backend/build/hiredis/src/libhiredis.so.0.13 chat-program-package/diamond-client/libs
cp /usr/lib/x86_64-linux-gnu/libuv.so.0.10 chat-program-package/diamond-client/libs

cp -r $JRE_DIR chat-program-package/jre

tar cf chat-program-package.tar chat-program-package
gzip chat-program-package.tar
rm -rf chat-program-package
