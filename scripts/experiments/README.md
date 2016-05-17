# Experiment scripts

This directory contains Diamond experiment scripts.

## Experimental setup

We assume that the clients and servers for an experiment will be run on a set
of machines or VMs with identical versions of Linux. One of these machines,
referenced as `SRC_HOST` in the scripts, will not be used to run any clients
or servers, but will instead perform a set of special functions:

* The Diamond platform and application code will be compiled on this
machine and copied from this machine to the other machines.
* Diamond servers will be started and stopped from this machine.
* An instance of Redis (called the data Redis instance) will run on this
machine, and clients will store their data in this Redis instance when they
finish executing.
* Scripts to parse the data in the above-mentioned Redis instance and compute
statistics will run on this machine.

Furthermore, we assume that a computer that is not a part of the set of
client/server machines, called the staging machine, will be used to launch
experiments and save the final results. For instance, if you are running
experiments on Google Compute Engine, the client/server machines might be GCE
VMs, while the staging machine might be your office desktop.

To summarize, we have the following computers/VMs:

1. A set of client/server machines.
2. A special machine called `SRC_HOST` running the same version of Linux
as the client/server machines and able to connect to all of them.
3. A staging machine able to connect to the client machines and `SRC_HOST`
where top-level wrapper scripts will be run from and the final results will be
stored.

## Script organization

There are three main categories of scripts in this folder:

1. Client scripts that run on the client machines, launching client processes
and storing their output to the data Redis instance on `SRC_HOST`.
    * run_retwis.py
    * run_baseline_retwis.py
    * run_game.py
    * run_redis_game.py
2. Data-parsing scripts that run on `SRC_HOST`. These scripts are used to
calculate throughputs and latencies from the transaction data in the data Redis
instance.
    * parse-scalability.py
    * parse-retwis.py
    * parse-game.py
3. Experiment scripts that run on the staging machine. They SSH into `SRC_HOST`
to start servers, SSH into client machines to launch the client scripts,
and SSH into `SRC_HOST` to compute statistics from the client output.
    * baseline-experiment.py
    * docc-experiment.py
    * caching-experiment.py

There are two Python modules containing common code, one for the client scripts
(`client_common.py`) and one for the experiment scripts (`experiment_common.py`).
In addition to holding common code, these modules also contain constants used
by every experiment that should be set by the user. These constants are near
the top of the file, and more information about setting them is included in
the setup instructions below.

### A note about file paths in scripts

Generally, the scripts assume that the `diamond-src` repo location on `SRC_HOST`
is in the home directory, and that there is one working directory path on the
client/server machines (specified by the constant `WORKING_DIR`) where binaries
will be copied and temporary files will be created. As a result, when the
functions in these scripts take paths to files on `SRC_HOST`, these paths
are relative to the `diamond-src` directory, and functions that copy files to
or execute commands on the client machines always do so in the working
directory.

## Setting things up

1. Install dependencies on the client/server machines. These dependencies
include, but are not necessarily limited to, the following:

    apt-get install libprotobuf-dev libevent-dev libpython-dev libboost-program-options-dev python-pip openjdk-8-jre-headless libhiredis-dev libev-dev
    pip install redis

2. Clone this repository in your home directory on the `SRC_HOST` machine.
Compile the Diamond platform code, the programs in `apps/benchmarks`, the
programs in `apps/tapir-benchmarks`, and the programs in each subfolder of
`apps/baseline-benchmarks`.

3. Change the constants at the top of `client_common.py` and `experiment_common.py`
so that `SRC_HOST` contains the right address or hostname, `DATA_REDIS_PORT`
contains a valid port on which to run Redis, `REDIS_DIR` points to a directory
containing `redis-cli` and `redis-server` binaries, and `WORKING_DIR` points to
a directory that exists on every client/server machine where binaries will be
copied and results will be stored.

4. Make a text file called `clients.txt` in the `experiments` directory on your
staging machine (this directory) that contains the addresses of each client
machine, one per line.