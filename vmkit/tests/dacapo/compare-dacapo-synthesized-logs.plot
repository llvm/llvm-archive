#!/usr/bin/gnuplot -p

set datafile separator ","
set datafile missing '_'
set datafile commentschars "#"

set ylabel "Average relative overhead (lower is better)"
set ytics border nomirror
set grid ytics
set format y "%.0f %%"

set style data histogram
set style histogram clustered gap 1
set bars fullwidth
set style fill solid border 0
set boxwidth 1

# set title "Monitoring overhead in Dacapo benchmarks"

set xtics axis nomirror
set border 3

set term png size 900, 600
set output "overhead.png"
plot	\
	'/home/koutheir/dacapo_benchmarks/overhead-0-1.csv' using 3:xtic(1) linecolor rgb "#e5e5e5" title "0 vs 1",	 \
	'/home/koutheir/dacapo_benchmarks/overhead-0-6.csv' using 3:xtic(1) linecolor rgb "#b2b2b2" title "0 vs 6",	 \
	'/home/koutheir/dacapo_benchmarks/overhead-0-7.csv' using 3:xtic(1) linecolor rgb "#7f7f7f" title "0 vs 7"

set output "overhead-0-1.png"
plot	\
	'/home/koutheir/dacapo_benchmarks/overhead-0-1.csv' using 3:xtic(1) linecolor rgb "#808080" title "0 vs 1"

set output "overhead-0-6.png"
plot	\
	'/home/koutheir/dacapo_benchmarks/overhead-0-6.csv' using 3:xtic(1) linecolor rgb "#808080" title "0 vs 6"

set output "overhead-0-7.png"
plot	\
	'/home/koutheir/dacapo_benchmarks/overhead-0-7.csv' using 3:xtic(1) linecolor rgb "#808080" title "0 vs 7"
