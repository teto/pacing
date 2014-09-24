#!/bin/sh
# TODO pass terminal and optionnaly output
# if []
terminal="png"
gnuplot -e "mattTerminal='$terminal'"  owd.plot
gnuplot -e "mattTerminal='$terminal'" delta.plot
