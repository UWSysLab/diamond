# Running Diamond servers

The script `manage-servers.py` automates the process of starting and stopping
servers on remote hosts using SSH. When starting servers, it also uses rsync to
copy the necessary config/executable files over to each of the hosts
beforehand. The script obtains the hostnames of the remote hosts from the
supplied config files.

You'll need to set two environment variables to use the script:
`DIAMOND_WORKING_DIR` specifies the directory *on the remote hosts* in which to
store the copied-over files, and `DIAMOND_SRC_DIR` should be set to the
location of the Diamond repository *on the machine that you are using to run
the script*. You should make sure each of the remote hosts is set up for public
key SSH authentication, to avoid needing to type your password repeatedly
during script execution. Note that by using config files with `localhost` hosts
(for example, the `local*.config` files in `platform/test`), you can use this
script to run servers locally as well.

## Launching servers

To start up the servers, run `manage-servers.py` with `start` as the first
argument and the config file prefix (including the path to the files) as the
second argument.

If our config files were all located in `platform/test` and had the
prefix `myconfig`, the command would be

    $ cd scripts
    $ ./manage-servers.py start ../platform/test/myconfig

This command will detect all of the config files with the prefix `myconfig` and
start all of the frontend servers and backend shards. If you only want to start
a smaller number of frontend servers, use the `--frontends` command (or the
`--shards` command to start a smaller number of shards).

## Killing servers

To kill the servers, run `manage-servers.py` with `kill` as the second argument.

    $ cd scripts
    $ ./manage-servers.py kill ../platform/test/myconfig
