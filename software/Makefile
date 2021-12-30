#
TARGET = MyPlayer

#
ALT_DEVICE_FAMILY ?= soc_cv_av
CROSS_COMPILE = arm-linux-gnueabihf-
CFLAGS = -D$(ALT_DEVICE_FAMILY) -static -g -Wall  -I${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include -I${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include/$(ALT_DEVICE_FAMILY)
LDFLAGS =  -g -Wall
CC = $(CROSS_COMPILE)gcc
ARCH = arm

build: $(TARGET)

$(TARGET): main.o pcm.o audio_control.o
	$(CC) $(LDFLAGS)   $^ -o $@  -static
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.a *.o *~ *.bmp
