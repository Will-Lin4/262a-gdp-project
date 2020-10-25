#!/bin/bash

for i in `seq 1 10`;
do
    gnome-terminal -x bash -c "./rw-bench -t 1 -n 2000 -m 100 -M 101; sleep 10" &
done
