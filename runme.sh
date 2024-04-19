#!/bin/bash

make

rm -f stats.txt
rm -f myfile.lck

for i in {1..10}; do
    ./locker -f myfile &
done

sleep 300

killall -SIGINT locker

echo "Statistics:"
cat stats.txt