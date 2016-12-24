input_dir='/data/wiki/en_articles_part/*'
out_dir='output/wordcount'
reducers_count=5

# Remove previous results
hadoop fs -rm -r -skipTrash $out_dir*
# Run task
hadoop jar /usr/lib/hadoop-mapreduce/hadoop-streaming.jar \
-D mapred.map.tasks=$reducers_count \
-D mapred.reduce.tasks=$reducers_count \
-file mapper_pi.py -mapper mapper_pi.py \
-file reducer_pi.py -reducer reducer_pi.py \
-input /data/random_points/* -output $out_dir
# Check results
for num in `seq 0 $[$reducers_count - 1]`
do
    hadoop fs -cat $out_dir/part-0000$num # | head
done
