#set terminal png enhanced size 800,600 

# TODO rather use LPs than lt cf http://kunak.phsx.ku.edu/~sergei/Gnuplot/line_point_types.html
set style line 1 lt 1 lw 3 pt 3 lc rgb "red"
set style line 2 lt 3 lw 3 pt 3 lc rgb "red"
set style line 3 lt 1 lw 3 pt 3 lc rgb "blue"
set style line 4 lt 3 lw 3 pt 3 lc rgb "blue"

# Among available (pdfcairo, png, X11 )
# if using png, set output eg set output "/tmp/myGraph.png"

if (!exists("mattTerminal")) mattTerminal='x11'

set terminal mattTerminal

# Places of the legend
set key right top

#set xdata time
#set timefmt "%d%m%H%M"
#set format x "%d/%m\n%H/%M"

# Left/Right/Top/Bottom
set offset graph 0.0, graph 0.1, graph 0.1, graph 0.
#set bmargin 20
#set lmargin {<margin>}
#set rmargin {<margin>}
#set tmargin 20
set autoscale xy

set pointintervalbox 3
set grid

set xlabel 'Round number'
set ylabel 'Time (ms)'


# set font on axis labels
set xtics font ",20" 
set ytics font ",20" 

# set key font
set key font ",14"


set datafile separator "," 