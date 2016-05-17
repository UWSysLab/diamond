# Experiment scripts

This directory contains Diamond experiment scripts.

## Experimental setup

We assume that the clients and servers for an experiment will be run on a set
of machines or VMs with identical versions of Linux. One of these machines,
referenced as `SRC_HOST` in the scripts, will not be used to run any clients
or servers, but will instead perform a set of special functions:

* The Diamond platform and application code will be compiled on this
machine and copied from this machine to the other machines.
* The script to start and stop Diamond servers will run on this machine.
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
2. A special client/server machine called `SRC_HOST`.
3. A staging machine able to connect to the client machines and `SRC_HOST`
where top-level wrapper scripts will be run from and the final results will be
stored.

## Script organization

There are three main categories of scripts in this folder:

1. Client scripts that run on the client machines, launching client processes
and storing their output to the data Redis instance on `SRC_HOST`.
    * run\_retwis.py
    * run\_baseline\_retwis.py
    * run\_game.py
    * run\_redis\_game.py
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

### File paths, file copying, and command execution

The experiment and client scripts copy all the files they need from `SRC_HOST`
to a fixed working directory on the client/server machines. As a result, the
config prefix and key file paths passed into the client scripts are the paths
of these files on `SRC_HOST`. In general, all of the copying functions in
the scripts copy a file at an arbitrary path on `SRC_HOST`
(either an absolute path or a path relative to the user's home directory)
into the working directory of the client/server machines.

The individual experiment and client scripts explicitly copy the specific files
they require over to the client. The pathnames of these files are generally
given assuming the `diamond-src` repo has been cloned in the user's home directory
on `SRC_HOST` (the setup instructions below describe how to set things up to ensure
that the scripts work properly).

All commands executed by the scripts on `SRC_HOST` run in the user's home
directory there. Commands executed by the experiment scripts on client machines
with `runOnClientMachines()` run in the working directory on each client
machine.

## Setting things up

1. Install dependencies on the client/server machines. These dependencies
include, but are not necessarily limited to, the following:

        apt-get install libprotobuf-dev libevent-dev libpython-dev \
            libboost-program-options-dev python-pip openjdk-8-jre-headless \
            libhiredis-dev libev-dev
        pip install redis

2. Install dependencies on the `SRC_HOST` machine. In addition to the usual
dependencies required to compile Diamond, you'll also need to download a copy of
Redis and compile it.

3. Clone this repository in your home directory on `SRC_HOST`.
Compile the Diamond platform code and the programs in `apps/benchmarks`,
`apps/tapir-benchmarks`, `apps/baseline-benchmarks/keyvaluestore`, and
`apps/baseline-benchmarks/100game`. For most of the apps, you'll just need to
make a `build` subfolder and run CMake, but for the baseline key-value store,
you'll need to run `mvn package`.

4. Change the constants at the top of `experiment_common.py`
so that `SRC_HOST` contains the right address or hostname, `DATA_REDIS_PORT`
contains a valid port on which to run Redis, `REDIS_DIR` points to a directory
on `SRC_HOST`
containing `redis-cli` and `redis-server` binaries, and `WORKING_DIR` points to
a directory that exists on every client/server machine where binaries will be
copied and results will be stored. These changes should be made on the staging
machine.

5. Set the environment variables `DIAMOND_SRC_DIR`, `DIAMOND_WORKING_DIR`, and
`REDIS_DIR` on `SRC_HOST` to point to the location of the `diamond-src` repo on
`SRC_HOST`, the working directory on the server machines, and a directory
containing Redis binaries on `SRC_HOST`, respectively.

6. Make a text file called `clients.txt` in the `experiments` directory (this
directory) on your staging machine that contains the addresses of each client
machine, one per line.

7. Create your config files and keys file and place them on `SRC_HOST` (since
they will need to be copied to client/server machines). Change the constants at
the top of each experiment script to reflect the config prefix and keys file
you want to use for each experiment. These changes should be made on the
staging machine.
