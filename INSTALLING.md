AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA


## Compiling the software:

All PC-software to build and flash the embedded software is included,
just run the `make.bat` file.

The system can also be built with Linux, using `wine`.

## Flashing the software to the embedded system:

The software can be flashed either with JTAG or the serial port.

### Flashing with serial port
1. Build the software if not already built.
2. connect a 3.3V serial UART interface to the computer, make sure the drivers
   installs correctly.
3. Check what COM-port number the computer has assigned to the serial interface:
4. Edit the flash.bat file created when compiling, change the part `flash.COM1`
   to the number of the connected UART interface.
5. Connect the embedded system to the UART, make sure it is not powered up.
6. Close the serial programming jumper connector.
7. Power up the board.
8. Run the flash.bat file, wait until it completes.
9. Power down the board.
10. Disconnect the serial programming jumper connector.

### Determining the serial port
#### Windows
The serial port number can be found by looking at
      "Den har datorn" -> egenskaper -> enhetshanteraren -> portar

#### Linux
Run `dmesg | tail` after connecting the serial interface. Look for something
like

`usb 2-2: FTDI USB Serial Device converter now attached to ttyUSB0`

To make it possible for wine to use the device, make a symbolic device link.

`ln -s /dev/ttyUSB0 ~/.wine/dosdevices/com1`

To flash the embedded system from Linux, run `make flash.COM1`

## Communicating with the embedded system:
   Connect to the 3.3V UART with 115200 baud, 8 bit data, 1 stop bit, no parity.
   A prompt shows up when the system is started, press `h` for inline help.


