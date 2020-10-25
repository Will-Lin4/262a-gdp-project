#!/bin/bash

## Prints the duplicate logs (and number of records in the duplicates), also stores the list of logs in /tmp/gdp-0{1,2,3,4}.logs
## Assumes that password-less SSH is setup for gdp-0{1,2,3,4}
## Not very thoroughly tested, works on ubuntu at least

for (( i=1; i<5; i++ )) ; do
    ssh gdp-0$i.eecs.berkeley.edu 'find /var/swarm/gdp/gcls/_* -name "*.gdplog"' | cut -d '/' -f 7 | cut -c -43 | sort > /tmp/gdp-0$i.logs
done

cat /tmp/gdp-01.logs /tmp/gdp-02.logs /tmp/gdp-03.logs /tmp/gdp-04.logs | sort > /tmp/all.logs
cat /tmp/all.logs | uniq > /tmp/uniq.logs

diff /tmp/all.logs /tmp/uniq.logs | grep "<" | sort | uniq | cut -c 3- > /tmp/dup.logs

for log in `cat /tmp/dup.logs` ; do
    for (( i=1; i<5; i++ )) ; do
        if grep -q -- $log /tmp/gdp-0$i ; then
            echo $log gdp-0$i
            ssh gdp-0$i.eecs.berkeley.edu "log-view -vv $log" | grep 'recs'
        fi
    done
done
