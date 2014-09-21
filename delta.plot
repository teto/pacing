###
###  Plot real and estimated deltas 
######################################


# Among available (pdfcairo, png, X11 )
# if using png, set output eg set output "/tmp/myGraph.png"
set terminal x11

# Places of the legend
set key right top



#set title ' Flux en fonction du temps: '
set xlabel 'Round number'
set ylabel 'Time (ms)'

# allow to distinguish between data points and plot borders
# set offsets 30
set offset graph 0.1, graph 0.1, graph 0.1, graph 0.1
#set bmargin 20
#set lmargin {<margin>}
#set rmargin {<margin>}
#set tmargin 20
set autoscale xy
#set xdata time
#set timefmt "%d%m%H%M"
#set format x "%d/%m\n%H/%M"

set pointintervalbox 3
set grid

set datafile separator "," 

#sprintf("Real Forward OWD0 %d",$4)
# "::2" to start at the 2nd line (http://babilonline.blogspot.fr/2010/06/gnuplot-skipping-header-line-from-input.html)
plot  \
	"../../test.csv" every ::2 using 1:(abs($5-$4)) with linespoints pointtype 1 title "Real Forward DeltaOWD", \
	"../../test.csv" using 1:(abs($7-$6)) with linespoints pointtype 2 title "Real Reverse DeltaOWD", \
	"../../test.csv" using 1:11 with linespoints pointtype 6 title "Estimated Forward DeltaOWD", \
	"../../test.csv" using 1:17 with linespoints pointtype 3 title "Estimated Reverse DeltaOWD"

#with lines