#!/bin/bash

declare -i count=500
declare -i th=1

while [ $count -le 5000 ]
do
    ./life $count $count 100 15 0.5
    let "count = count + 500"
done

while [ $th -le 15 ]
do 
    ./life 500 500 1000 $th 0.5
    let "th = th + 1"
done
echo 'OK'
exit 0

