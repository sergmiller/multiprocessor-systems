out_dir='output/wordcount'
reducers_count=3

# Remove previous results
hadoop fs -rm -r -skipTrash $out_dir*
# Run task
hadoop jar /usr/lib/hadoop-mapreduce/hadoop-streaming.jar \
-D mapred.map.tasks=$reducers_count \
-D mapred.reduce.tasks=$reducers_count \
-file mapper.py -mapper mapper.py \
-file reducer.py -reducer reducer.py \
-input /data/griboedov/* -output $out_dir
# Check results
for num in `seq 0 $[$reducers_count - 1]`
do
    hadoop fs -cat $out_dir/part-0000$num | head
done
