#!/bin/bash

declare -i count=500

while [ $count -le 5000 ]
do
    ./parallel_life $count 1 0 -1 5
    let "count = count + 500"
done
echo 'OK'
exit 0

