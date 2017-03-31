# Dramco-Volume-Knob

## Instructions Dramco Volume Knob

Download firmware (bootloader): https://github.com/micronucleus/micronucleus (in firmware/releases/t85_default.hex)
Program with ArduinoISP: 
"C:\Program Files (x86)\Arduino\hardware\tools\avr/bin/avrdude" -CC:"\Program Files (x86)\Arduino\hardware\tools\avr/etc/avrdude.conf" -v -pattiny85 -cstk500v1 -PCOM4 -b19200 -Uflash:w:t85_default.hex:i -U lfuse:w:0xe1:m -U hfuse:w:0x5d:m -U efuse:w:0xfe:m

Upload arduino sketch:
Install micronucleus drivers (in micronucleus driver installation folder)
Install digistum in Arduino IDE: add to board url http://digistump.com/package_digistump_index.json
Install "Digistump AVR boards" under the Tools... Board... Board Manager...
Choose Digispark default 16.5MHz.

You must insert / reinsert the ATTiny85's USB plug after the sketch has compiled and is ready to upload.