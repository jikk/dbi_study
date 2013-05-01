#set terminal latex
#set output "bzip_perf.tex"
set term postscript monochrome enhanced
set term postscript eps font "LucidaGrande,13"

set output "bzip_perf.eps"

set title "Performance Breakdown of Bzip2"

#set auto x
set size 0.6, 0.75
set yrange[0:35]
set ylabel "Slowdown"
set ylabel rotate by 90
set xtics rotate by -45
set style histogram rowstacked
set style data histogram
set style fill solid border -1
#set style histogram cluster gap 1
#set bmargin 3
#set key top right

plot 'bzip_perf.dat' using 2:xtic(1) title columnheader fs solid lc rgb '#000000' lt 1\
				  ,'' using 3 title columnheader fs solid lc rgb '#ffffff' lt 1\
				  ,'' using 4 title columnheader fs solid lc rgb '#414141' lt 1\
				  ,'' using 5 title columnheader fs solid lc rgb '#bebebe' lt 1\
				  ,'' using 6 title columnheader fs solid lc rgb '#808080' lt 1

set output "tar_perf.eps"
set size 0.6, 0.75
set yrange[0:5]
set ylabel "Slowdown"
set ylabel rotate by 90
set xtics rotate by -45
set style histogram rowstacked
set style data histogram
set style fill solid border -1
set title "Performance Breakdown of tar"

plot 'tar_perf.dat' using 2:xtic(1) title columnheader fs solid lc rgb '#000000' lt 1\
				  ,'' using 3 title columnheader fs solid lc rgb '#ffffff' lt 1\
				  ,'' using 4 title columnheader fs solid lc rgb '#414141' lt 1\
				  ,'' using 5 title columnheader fs solid lc rgb '#bebebe' lt 1\
				  ,'' using 6 title columnheader fs solid lc rgb '#808080' lt 1

#				  ,'' using 7:xtic(6) title columnheader fs solid lc rgb '#000000' lt 1
#				  ,'' using 8 title columnheader fs solid lc rgb '#ffffff' lt 1\
#				  ,'' using 9 title columnheader fs solid lc rgb '#414141' lt 1\
#				  ,'' using 10 title columnheader fs solid lc rgb '#bebebe' lt 1\
#				  ,'' using 12:xtic(11) title columnheader fs solid lc rgb '#000000' lt 1\
#				  ,'' using 13 title columnheader fs solid lc rgb '#ffffff' lt 1\
#				  ,'' using 14 title columnheader fs solid lc rgb '#414141' lt 1\
#				  ,'' using 15 title columnheader fs solid lc rgb '#bebebe' lt 1\
#				  ,'' using 17:xtic(16) title columnheader fs solid lc rgb '#000000' lt 1\
#				  ,'' using 18 title columnheader fs solid lc rgb '#ffffff' lt 1\
#				  ,'' using 19 title columnheader fs solid lc rgb '#414141' lt 1\
#				  ,'' using 20 title columnheader fs solid lc rgb '#bebebe' lt 1\
