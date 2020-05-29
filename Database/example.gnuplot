
set datafile separator ','

set key autotitle columnhead
plot 'plot.csv' using 1:2 with lines, '' using 1:3 with lines

pause 1
reread
