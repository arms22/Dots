#! /usr/bin/env bash

AVRROOT=/Applications/Arduino1.0.6.app/Contents/Resources/Java/hardware/tools/avr
SKETCH=dotsduino_demo.ino.hex
BOOTLOADER=/Applications/Arduino1.0.6.app/Contents/Resources/Java/hardware/arduino/bootloaders/atmega/ATmegaBOOT_168_atmega328_pro_8MHz.hex

echo "######## Fuse set ########"
sleep 1
$AVRROOT/bin/avrdude -C$AVRROOT/etc/avrdude.conf -v -v -v -v -patmega328p -cstk500v2 -Pusb -e -Ulock:w:0x3F:m -Uefuse:w:0x05:m -Uhfuse:w:0xDA:m -Ulfuse:w:0xFF:m 

echo "######## Write Bootloader ########"
sleep 1
$AVRROOT/bin/avrdude -C$AVRROOT/etc/avrdude.conf -v -v -v -v -patmega328p -cstk500v2 -Pusb -Uflash:w:$BOOTLOADER:i -Ulock:w:0x0F:m 

echo "######## Write Demo sketch ########"
sleep 1
$AVRROOT/bin/avrdude -C$AVRROOT/etc/avrdude.conf -v -patmega328p -carduino -P/dev/cu.usbserial-A800csyY -b57600 -D -Uflash:w:$SKETCH:i

echo $'\a'
