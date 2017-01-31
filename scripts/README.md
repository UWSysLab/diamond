# Running Diamond servers

## Prerequisites

You'll need several config files: one for the timestamp server, one for each
backend shard, and one for each frontend server. These config files all have
a common prefix (our example will creatively use `prefix` as the prefix), and
they should all be located in the same directory.

Backend shard config files have names `prefix0.config`, `prefix1.config`, etc.
Each config file has the following layout:

    host <hostname>:<port>
    host <hostname>:<port>
    ...

where `<num-failures>` is a number representing the maximum number of failures
tolerated, and each `replica` line lists a replica's hostname and port.

The TSS config file has the name `prefix.tss.config`. It has the same format
as the backend shard config file.

The frontend config files have names `prefix.frontend0.config`,
`prefix.frontend1.config`, etc. They have the same format as the backend shard
config file, but they only have one `replica` entry, and `num-failures` should
be set to 1.

## Launching servers

To start up servers on one or more remote hosts, run `manage-servers.py` with
`start` as the first argument and the config file prefix (including the path to
the files) as the second argument. This script will copy the necessary
config/executable files over to each of the hosts specified in the config files
using rsync and then start the servers using SSH. You'll need to set two
environment variables to use the script: `DIAMOND_WORKING_DIR` specifies the
directory *on the remote hosts* in which to store the copied-over files, and
`DIAMOND_SRC_DIR` should be set to the location of the Diamond repository *on
the machine that you are using to run the script*. You should make sure each
of the remote hosts is set up for public key SSH authentication, to avoid
needing to type your password repeatedly during script execution. Note that
by using config files with `localhost` hosts (for example, the `local*.config`
files in `platform/test`), you can use this script to run servers
locally as well.

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
