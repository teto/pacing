###
###  Plot real and estimated forward OWDs 
######################################
load 'common.plot'

set output "owd.png"

set style line 6 pointsize 3 linecolor "red"
#set style line 2 pointsize 3

#sprintf("Real Forward OWD0 %d",$4)
# "::2" to start at the 2nd line (http://babilonline.blogspot.fr/2010/06/gnuplot-skipping-header-line-from-input.html)
plot  \
	"../../test.csv" every ::2 using 1:4 with linespoints pointtype 2 title "Real forward OWD 1", \
	"../../test.csv" using 1:9 with linespoints pointtype 15 title "Estimated forward OWD 1", \
	"../../test.csv" using 1:12 with linespoints pointtype 10 title "Half RTT on path 1", \
	"../../test.csv" using 1:5 with linespoints pointtype 3 title "Real forward OWD 2", \
	"../../test.csv" using 1:10 with linespoints pointtype 7 title "Estimated forward OWD 2", \
	"../../test.csv" using 1:13 with linespoints pointtype 11 title "Half RTT on path 2"

#with lines

#"../../test.csv" using 1:6 with linespoints pointtype 4 title "Real reverse OWD 1", \
#	"../../test.csv" using 1:15 with linespoints pointtype 8 title "Estimated reverse OWD 1", \
#	"../../test.csv" using 1:7 with linespoints pointtype 5 title "Real reverse OWD 2", \
#	"../../test.csv" using 1:16 with linespoints pointtype 9 title "Estimated reverse OWD 2", \
