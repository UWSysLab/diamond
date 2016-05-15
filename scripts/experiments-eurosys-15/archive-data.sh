if [[ $# -ne 1 ]]; then
    echo "usage: ./archive-data.sh archive_file"
    exit 0
fi

ARCHIVE=$1

if [[ -a $ARCHIVE ]]; then
    echo "error: file already exists"
    exit 0
fi

TEMPDIR="$ARCHIVE-temp"
mkdir $ARCHIVE
cp -r android-chat-latency $ARCHIVE
cp -r twitter-latency $ARCHIVE
cp -r desktopchat-latency $ARCHIVE
cp -r desktopchat-throughput $ARCHIVE
tar -cf $ARCHIVE.tar $ARCHIVE
gzip $ARCHIVE.tar
rm -r $ARCHIVE
