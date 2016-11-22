omp time: 39s
OK
n = 1000, m = 1000, tm = 100, th = 1
#!/bin/bash

declare -i th=2

./lifeomp 1000 1000 100 1 0.5 1
while [ $th -le 15 ]
do
    ./lifeomp 1000 1000 100 $th 0.5 0
    let "th = th + 1"
done
echo 'OK'
exit 0

