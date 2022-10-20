# rtl_868
Receiver software for FM modulated temperature stations.

compile:
```
make
```
optionally with cmake


run requires `rtl_fm`, e.g. from https://github.com/librtlsdr/librtlsdr
```
rtl_fm -f 868.26e6 -M fm -s 500k -r 75k -g 42 -A fast | ./rtl_868 > dump-file.txt
```
