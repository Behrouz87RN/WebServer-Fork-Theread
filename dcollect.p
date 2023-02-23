set terminal png size 1024,1024
set output 'statisticsfork.png'
set xlabel 'Concurrency Level'
set ylabel 'Requests/s'
plot 'statistics_fork.log' using 1:3 title 'Average' with lines,  'statistics_fork.log' using 1:4 title 'std.dev' with lines

set terminal png size 1024,1024
set output 'statisticsthread.png'
set xlabel 'Concurrency Level'
set ylabel 'Requests/s'
plot 'statistics_thread.log' using 1:3 title 'Average' with lines,  'statistics_thread.log' using 1:4 title 'std.dev' with lines

