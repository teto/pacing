
# Among available (pdfcairo, png, X11 )
# if using png, set output eg set output "/tmp/myGraph.png"
set terminal x11

# Places of the legend
set key left bottom

#set xdata time
#set timefmt "%d%m%H%M"
#set format x "%d/%m\n%H/%M"

set offset graph 0.1, graph 0.1, graph 0.1, graph 0.1
#set bmargin 20
#set lmargin {<margin>}
#set rmargin {<margin>}
#set tmargin 20
set autoscale xy

set pointintervalbox 3
set grid

set datafile separator "," 

#sprintf("Real Forward OWD0 %d",$4)
# "::2" to start at the 2nd line (http://babilonline.blogspot.fr/2010/06/gnuplot-skipping-header-line-from-input.html)
plot  \
	"../../test.csv" every ::2 using 1:4 with linespoints pointtype 2 title "Real forward OWD0", \
	"../../test.csv" using 1:5 with linespoints pointtype 3 title "Real forward OWD1", \
	"../../test.csv" using 1:6 with linespoints pointtype 4 title "Real reverse OWD0", \
	"../../test.csv" using 1:7 with linespoints pointtype 5 title "Real reverse OWD1", \
	"../../test.csv" using 1:6 with linespoints pointtype 6 title "Estimated forward OWD0", \
	"../../test.csv" using 1:6 with linespoints pointtype 6 title "Estimated forward OWD1", \
	"../../test.csv" using 1:6 with linespoints pointtype 6 title "Estimated reverse OWD0", \
	"../../test.csv" using 1:6 with linespoints pointtype 6 title "Estimated reverse OWD1"

#with lines