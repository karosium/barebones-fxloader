all:
        gcc fxloader.c dumbloader.c -lusb-1.0 -I/usr/include/libusb-1.0 -o dumbloader
