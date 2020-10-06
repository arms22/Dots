#! /usr/bin/env bash

AVRROOT=/Applications/Arduino.app/Contents/Java/hardware/tools/avr
SKETCH=dotsduino_demo.ino.hex
BOOTLOADER=/Applications/Arduino.app/Contents/Java/hardware/arduino/avr/bootloaders/atmega/ATmegaBOOT_168_atmega328_pro_8MHz.hex
PROGRAMPORT=/dev/cu.usbmodem00297912
ARDUINOPORT=/dev/cu.usbmodem00297914

echo "######## Fuse set ########"
sleep 1
$AVRROOT/bin/avrdude -C$AVRROOT/etc/avrdude.conf -v -patmega328p -cstk500 -P$PROGRAMPORT -e -Ulock:w:0x3F:m -Uefuse:w:0xFD:m -Uhfuse:w:0xDA:m -Ulfuse:w:0xFF:m

echo "######## Write Bootloader ########"
sleep 1
$AVRROOT/bin/avrdude -C$AVRROOT/etc/avrdude.conf -v -patmega328p -cstk500 -P$PROGRAMPORT -Uflash:w:$BOOTLOADER:i -Ulock:w:0x0F:m

echo "######## Write Demo sketch ########"
sleep 1
$AVRROOT/bin/avrdude -C$AVRROOT/etc/avrdude.conf -v -patmega328p -carduino -P$ARDUINOPORT -b57600 -D -Uflash:w:dotsduino_demo.ino.hex:i

echo $'\a'
