DIAMOND_SRC=/home/nl35/research/diamond-src
JRE_DIR=/home/nl35/research/jdk1.8.0_60/jre

BASELINE_SERVER_DIR=$DIAMOND_SRC/apps/chat/BaselineChatServer
BASELINE_CLIENT_DIR=$DIAMOND_SRC/apps/chat/BaselineChatClient

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

cp -r $JRE_DIR chat-program-package/jre

tar cf chat-program-package.tar chat-program-package
gzip chat-program-package.tar
rm -rf chat-program-package
