#!/usr/bin/gnuplot -p

set datafile separator ","
set datafile missing '_'
set datafile commentschars "#"

set ylabel "Duration in microseconds (lower is better)"
set ytics border nomirror
set grid ytics
set yrange [0:*]
# set ydata time
# set timefmt "%S"
# set format y "%M:%S"

set xtics axis nomirror
set border 3

set style data histogram
set style histogram errorbars gap 1 lw 1
set bars fullwidth
set style fill solid border 0
set boxwidth 1

# set title "Monitoring overhead in micro-benchmarks"

# set term wxt 0
set term png size 800, 600

set output "micro-benchmarks-MethodCallBenchmark.png"
plot	\
	'/home/koutheir/vmkit_monitor_benchmarks/MethodCallBenchmark.csv' using (column(3)):(column(5)):xtic(1) linecolor rgb "orange" title "MethodCallBenchmark"

set output "micro-benchmarks-SmallObjectsBenchmark.png"
plot	\
	'/home/koutheir/vmkit_monitor_benchmarks/SmallObjectsBenchmark.csv' using (column(3)):(column(5)):xtic(1) linecolor rgb "blue" title "SmallObjectsBenchmark"
