set terminal png enhanced size 800,600 


set style line 1 lt 1 lw 3 pt 3 lc rgb "red"
set style line 2 lt 3 lw 3 pt 3 lc rgb "red"
set style line 3 lt 1 lw 3 pt 3 lc rgb "blue"
set style line 4 lt 3 lw 3 pt 3 lc rgb "blue"

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