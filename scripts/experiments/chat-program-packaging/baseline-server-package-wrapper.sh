if [[ $# -ne 2 ]]
then
    echo "usage: ./baseline-server-package-wrapper.sh port package_dir"
    exit
fi

PORT=$1
DIR=$2
PROG_DIR=$DIR/baseline-server
$DIR/jre/bin/java -cp $PROG_DIR/bin:$PROG_DIR/libs/jedis-2.4.2.jar:$PROG_DIR/libs/commons-pool2-2.0.jar:$PROG_DIR/libs/gson-2.3.1.jar Main $PORT
