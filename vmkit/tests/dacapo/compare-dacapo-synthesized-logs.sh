#!/bin/bash

synth_file_1=$1
synth_file_2=$2

if [ \( -z "$synth_file_1" \) -o \( -z "$synth_file_2" \) ]; then
	exit 1
fi

echo '#benchmark,min_duration_overhead,average_duration_overhead,max_duration_overhead,std_dev_duration_overhead'

while read -r line ; do
	if ! echo $line | grep '^#' >/dev/null 2>&1; then
		cur_bench=$(echo $line | cut -d ',' -f 1)
		min_duration_1=$(echo $line | cut -d ',' -f 2)
		average_duration_1=$(echo $line | cut -d ',' -f 3)
		max_duration_1=$(echo $line | cut -d ',' -f 4)
		std_dev_duration_1=$(echo $line | cut -d ',' -f 5)
		
		line_2=$(grep "^$cur_bench," "$synth_file_2")
		min_duration_2=$(echo $line_2 | cut -d ',' -f 2)
		average_duration_2=$(echo $line_2 | cut -d ',' -f 3)
		max_duration_2=$(echo $line_2 | cut -d ',' -f 4)
		std_dev_duration_2=$(echo $line_2 | cut -d ',' -f 5)
		
		min_duration_overhead=$(echo "scale=4; 100.0 * (($min_duration_2 / $min_duration_1) - 1.0)" | bc -q 2>/dev/null)
		average_duration_overhead=$(echo "scale=4; 100.0 * (($average_duration_2 / $average_duration_1) - 1.0)" | bc -q 2>/dev/null)
		max_duration_overhead=$(echo "scale=4; 100.0 * (($max_duration_2 / $max_duration_1) - 1.0)" | bc -q 2>/dev/null)
		std_dev_duration_overhead=$(echo "scale=4; 100.0 * (($std_dev_duration_2 / $std_dev_duration_1) - 1.0)" | bc -q 2>/dev/null)
		
		printf "%s,%.2f,%.2f,%.2f,%.2f\n" "$cur_bench" "$min_duration_overhead" "$average_duration_overhead" "$max_duration_overhead" "$std_dev_duration_overhead"
	fi
done < "$synth_file_1"
