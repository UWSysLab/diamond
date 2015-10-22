DIAMOND_SRC=/home/nl35/research/diamond-src
JRE_DIR=/home/nl35/research/jdk1.8.0_60/jre

BASELINE_CHAT_DIR=$DIAMOND_SRC/apps/chat/BaselineChatServer

mkdir chat-program-package
mkdir chat-program-package/baseline-server
mkdir chat-program-package/baseline-server/libs
mkdir chat-program-package/baseline-server/bin

cp -r $JRE_DIR chat-program-package/jre
cp $BASELINE_CHAT_DIR/libs/* chat-program-package/baseline-server/libs
cp $BASELINE_CHAT_DIR/bin/* chat-program-package/baseline-server/bin
cp baseline-server-package-wrapper.sh chat-program-package/baseline-server

tar cf chat-program-package.tar chat-program-package
gzip chat-program-package.tar
rm -r chat-program-package
