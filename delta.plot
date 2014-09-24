###
###  Plot real and estimated deltas 
######################################
load 'common.plot'

set output "delta.png"

#sprintf("Real Forward OWD0 %d",$4)
# "::2" to start at the 2nd line (http://babilonline.blogspot.fr/2010/06/gnuplot-skipping-header-line-from-input.html)
plot  \
	"../../test.csv" every ::2 using 1:(abs($5-$4)) with linespoints pointtype 1 title "Real Forward DeltaOWD", \
	"../../test.csv" using 1:(abs($7-$6)) with linespoints pointtype 2 title "Real Reverse DeltaOWD", \
	"../../test.csv" using 1:11 with linespoints pointtype 6 title "Estimated Forward DeltaOWD", \
	"../../test.csv" using 1:17 with linespoints pointtype 3 title "Estimated Reverse DeltaOWD"

#with lines