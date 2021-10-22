#!/bin/bash

sleep 1 && pkill -SIGSTOP da_proc && pkill -SIGCONT da_proc && echo OK &
./tools/stress.py -r template_cpp/run.sh -t perfect -l logs -p $1 -m $2