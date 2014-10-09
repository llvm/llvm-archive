#!/usr/bin/gnuplot -p

set datafile separator ","
set datafile missing '_'
set datafile commentschars "#"

set ylabel "Duration (lower is better)"
set ytics border nomirror
set grid ytics
set yrange [0:*]
set ydata time
set timefmt "%S"
set format y "%M:%S"

set xtics axis nomirror
set border 3

set style data histogram
set style histogram errorbars gap 1 lw 1
set bars fullwidth
set style fill solid border 0
set boxwidth 1

# set title "Monitoring overhead in Dacapo benchmarks"

set term png size 900, 600
set output "dacapo.png"
plot	\
	'/home/koutheir/dacapo_benchmarks/0.csv' using (column(3)/1000.0):(column(5)/1000.0):xtic(1) linecolor rgb "#e5e5e5" title "0",	\
	'/home/koutheir/dacapo_benchmarks/1.csv' using (column(3)/1000.0):(column(5)/1000.0):xtic(1) linecolor rgb "#b2b2b2" title "1",	\
	'/home/koutheir/dacapo_benchmarks/6.csv' using (column(3)/1000.0):(column(5)/1000.0):xtic(1) linecolor rgb "#7f7f7f" title "6",	\
	'/home/koutheir/dacapo_benchmarks/7.csv' using (column(3)/1000.0):(column(5)/1000.0):xtic(1) linecolor rgb "#4c4c4c" title "7"
