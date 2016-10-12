#!/bin/bash

declare -i count=100
declare -i th=1
declare -i cyc=1

 while [ $count -le 500 ]
do
    ./life $count 100 10 4 0.2 20
    let "count = count + 100"
done

while [ $cyc -le 20 ]
do
    ./life 1000 1000 10 4 0.2 $cyc
    let "cyc = cyc + 1"
done

#while [ $th -le 15 ]
#do
 #   ./life 1000 100 100 $th 0.2 20
  #  let "th = th + 1"
#done
#echo 'OK'
exit 0
