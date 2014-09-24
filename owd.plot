###
###  Plot real and estimated deltas 
######################################
load 'common.plot'

set output "owd.png"

#sprintf("Real Forward OWD0 %d",$4)
# "::2" to start at the 2nd line (http://babilonline.blogspot.fr/2010/06/gnuplot-skipping-header-line-from-input.html)
plot  \
	"../../test.csv" every ::2 using 1:4 with linespoints pointtype 2 title "Real forward OWD0", \
	"../../test.csv" using 1:9 with linespoints pointtype 6 title "Estimated forward OWD0", \
	"../../test.csv" using 1:5 with linespoints pointtype 3 title "Real forward OWD1", \
	"../../test.csv" using 1:10 with linespoints pointtype 7 title "Estimated forward OWD1", \
	"../../test.csv" using 1:6 with linespoints pointtype 4 title "Real reverse OWD0", \
	"../../test.csv" using 1:15 with linespoints pointtype 8 title "Estimated reverse OWD0", \
	"../../test.csv" using 1:7 with linespoints pointtype 5 title "Real reverse OWD1", \
	"../../test.csv" using 1:16 with linespoints pointtype 9 title "Estimated reverse OWD1", \
	"../../test.csv" using 1:12 with linespoints pointtype 10 title "Half RTT on path 0", \
	"../../test.csv" using 1:13 with linespoints pointtype 11 title "Half RTT on path 1", \

#with lines