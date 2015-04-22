# scam2srec
Print S-records from an Seeed-Studio/Grove_Serial_Camera_Kit (serial cam -> S-records)

A small Arduino sketch derived from
https://github.com/Seeed-Studio/Grove_Serial_Camera_Kit

(I did not clone it because I made a lot of
unnecessary changes just to please my working habits...)

The sketch does exactly what the Seeed Studio sketch does
(as I could not find any documentation, it would have been
difficult to do something else).  Except that it does not
use an SD card; it sends the data via 'Serial'.

In part for the fun of it, it prints S-Records.
https://en.wikipedia.org/wiki/SREC_%28file_format%29
(It gives an old engineering touch)

Binary (as come out of the kit) or base64 would have been
more compact.

To get the .JPEG file from the .HEX file, I use :

$ srec_cat p3.hex -o p3.jpg -binary

I capture the data with 'minicom' (^A-L)


