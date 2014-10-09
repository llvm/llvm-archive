#!/bin/bash

log_dir=$1

if [ -z "$log_dir" ]; then
	exit 1
fi

stage1_file=/tmp/$0.$$.stage1
stage2_file=/tmp/$0.$$.stage2
stage3_file=/tmp/$0.$$.stage3

rm -f "$stage1_file" "$stage2_file" "$stage3_file"

# Filter successful results and print them in the format: benchmark,duration
grep '^===== DaCapo [A-Za-z]\+ PASSED' $log_dir/*.log | sed "s/^.*:===== DaCapo //g ; s/ msec =====//g ; s/ PASSED in /,/g" | sort > $stage1_file

# Calculate minimum, maximum and average durations

count=0
while read -r line ; do
	cur_bench=$(echo $line | cut -d ',' -f 1)
	cur_duration=$(echo $line | cut -d ',' -f 2)
	
	if [ -z "$last_bench" ]; then
		last_bench=$cur_bench
		count=1
		average_duration=$cur_duration
		min_duration=$cur_duration
		max_duration=$cur_duration
	else
		if [ "$last_bench" == "$cur_bench" ]; then
			count=$(($count + 1))
			average_duration=$(($average_duration + $cur_duration))
			if [ "$cur_duration" -lt "$min_duration" ]; then
				min_duration=$cur_duration
			fi
			if [ "$cur_duration" -gt "$max_duration" ]; then
				max_duration=$cur_duration
			fi
		else
			average_duration=$(($average_duration / $count))
			echo "$last_bench,$min_duration,$average_duration,$max_duration" >> $stage2_file
			
			last_bench=$cur_bench
			count=1
			average_duration=$cur_duration
			min_duration=$cur_duration
			max_duration=$cur_duration
		fi
	fi
done < $stage1_file

if [ "$count" -gt 0 ]; then
	average_duration=$(($average_duration / $count))
	echo "$last_bench,$min_duration,$average_duration,$max_duration" >> $stage2_file
fi

echo '#benchmark,min_duration_ms,average_duration_ms,max_duration_ms,std_dev_duration_ms'

# Calculate the standard deviation
while read -r line ; do
	cur_bench=$(echo $line | cut -d ',' -f 1)
	cur_average=$(echo $line | cut -d ',' -f 3)
	
	grep "^$cur_bench," "$stage1_file" > "$stage3_file"
	
	count=0
	std_dev='sqrt((0'
	
	while read -r line2 ; do
		cur_duration=$(echo $line2 | cut -d ',' -f 2)
		
		std_dev+="+(($cur_duration - $cur_average)^2)"
		count=$(($count+1))
	done < "$stage3_file"
	
	std_dev+=")/$count)"
	std_dev=$(echo "scale=0; $std_dev" | bc -q 2>/dev/null)
	echo "$line,$std_dev"
done < $stage2_file

rm -f "$stage1_file" "$stage2_file" "$stage3_file"
