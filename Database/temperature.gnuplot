
set datafile separator ','

set key autotitle columnhead
plot 'data.csv' using 1:2 with lines

pause 1
reread
