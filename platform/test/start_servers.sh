#!/bin/bash

../build/server -c local0.config -i 0 &
../build/server -c local0.config -i 1 &
../build/server -c local0.config -i 2 &

../build/tss -c local.tss.config -i 0 &
../build/tss -c local.tss.config -i 1 &
../build/tss -c local.tss.config -i 2 &
