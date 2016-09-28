#!/bin/bash

declare -i count=5000

while [ $count -le 50000 ]
do
    ./parallel_life $count 1 0 -1 5
    let "count = count + 5000"
done
echo 'OK'
exit 0

