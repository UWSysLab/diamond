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
* An instance of Redis will run on this machine, and clients will store their
data in this Redis instance when they finish executing.
* Scripts to parse the data in the above-mentioned Redis instance and compute
statistics will run on this machine.

You should enter the hostnames or IP addresses of the machines into the following
locations:

* `SRC_HOST` and `DATA_REDIS_PORT` in experiment\_common.py and client\_common.py
should be set to the address of the privileged machine and the port for the Redis
instance running on it, respectively.
* The addresses of the client machines should be entered in a file called "clients.txt"
in this directory.
* The addresses of the server machines should be included in the config files used
with the experiments (see the README in the directory above for more information
about the config files).
