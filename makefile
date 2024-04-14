# makefile for AVR ATmega328P projects using Arduino Uno
# ECE-231 Spring 2023
# revision history
#	Date		Author			Revision
#	2/14/22		D. McLaughlin	initial release 
# 	2/15/22		D. McLaughlin	updated with corrections (thanks S. Kaza)
#	3/30/22		D. McLaughlin	updated for use with Sparkfun Pocket Programmer
# 	4/3/22		D. McLaughlin	tested on Windows 10 (parallels 17 on MBP Apple Silicon)
#	2/13/23		D. McLaughlin	simplified for Arduino Uno dev board only, ECE231 Spring 2023
#   4/14/24     J. Constable    configured for atmega328p chip

#______________ MODIFY SERIALPORT AND SOURCEFILE_______________________
# Specify the com port (windows) or USB port (macOS)
# Use Device Manager to identify COM port number for Arduino Uno board in Windows
# In Terminal, type ls /dev/tty.usb* to determine USB port number in macOS

# This makefile is used for an atmega328p chip with 16GHz clock. Configuration for Arduino (including atmega328p) is also provided. 
# Use AVR Pocket Programmer

SERIALPORT = comx

# Arduino - check com port in device manager
# SERIALPORT = COM? 

# Specify the name of your source code here:
SOURCEFILE = MegaT.c SSD1306.c i2c.c
#_____________________________________________________________________

CLOCKSPEED = 16000000UL
PROGRAMMER = usbtiny
MCU = atmega328p

# Arduino
# CLOCKSPEED = 16000000
# PROGRAMMER = Arduino


begin:	main.hex

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(MCU) main.elf 

# Arduino
# avr-size --format=avr --mcu=atmega328p main.elf 


main.elf: $(SOURCEFILE)
	avr-gcc -Wall -Os -DF_CPU=$(CLOCKSPEED) -mmcu=$(MCU) -o main.elf $(SOURCEFILE)

# Arduino
# avr-gcc -Wall -Os -DF_CPU=$(CLOCKSPEED) -mmcu=atmega328p -o main.elf $(SOURCEFILE)


flash:	begin

	avrdude -c $(PROGRAMMER) -b 115200 -P $(SERIALPORT) -p $(MCU) -U flash:w:main.hex:i

# Arduino
# avrdude -c $(PROGRAMMER) -b 115200 -P $(SERIALPORT) -p atmega328p -U flash:w:main.hex:i
