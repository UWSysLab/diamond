## Running Diamond servers

### Prerequisites

You'll need several config files: one for the timestamp server, one for each
backend shard, and one for each frontend server. These config files all have
a common prefix (our example will creatively use `prefix` as the prefix), and
they should all be located in the same directory.

Backend shard config files have names `prefix0.config`, `prefix1.config`, etc.
Each config file has the following layout:

    $f <num-failures>
    $replica <hostname>:<port>
    $replica <hostname>:<port>
    $...

where `<num-failures>` is a number representing the maximum number of failures
tolerated, and each `replica` line lists a replica's hostname and port.

The TSS config file has the name `prefix.tss.config`. It has the same format
as the backend shard config file.

The frontend config files have names `prefix.frontend0.config`,
`prefix.frontend1.config`, etc. They have the same format as the backend shard
config file, but they only have one `replica` entry, and `num-failures` should
be set to 1.

### Launching servers

To start up servers, run `manage-servers.py` with `start` as the first argument
and the config file prefix (including the path to the files) as the second
argument. If our config files from above were all located in `platform/test`,
the command would be

    $ cd scripts
    $ ./manage-servers.py start ../platform/test/prefix

This command will detect all of the config files with the given prefix and
start all of the frontend servers and backend shards. If you only want to start
a smaller number of frontend servers, use the `--frontends` command (or the
`--shards` command to start a smaller number of shards).

### Killing servers

To kill the servers, run `manage-servers.py` with `kill` as the second argument.

    $ cd scripts
    $ ./manage-servers.py kill ../platform/test/prefix