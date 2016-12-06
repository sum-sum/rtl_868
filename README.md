# rtl_868
Reciever software for FM modulated temperature stations.

compile: 'make'
run: rtl_fm -f 868.26e6 -M fm -s 500k -r 75k -g 42 -A fast | ./rtl_868 > dump-file.txt

Win32 Cross compilation (build on Linuw, run on Windows)
compile: './wmake.sh rtl_868.exe'

