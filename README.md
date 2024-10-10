# Arduino_GIGA-stuff

This github project contains some of my experiments playing with the Arduino GIGA board. 

There are three main sub-directories

- Documents: Things like excel pin documents
- libraries: WIP libraries.  I typically keep a link to each of these in my Sketches/libraries directory.
- sketches: Some different GIGA test sketches, some of interest many just for the understanding. I keep a link (windows mklink /J to this directory in my Arduino directory)

## Warning: always a work in progress!

Everything up in this project is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement. in no event shall the
authors or copyright holders be liable for any claim, damages or other
liability, whether in an action of contract, tort or otherwise, arising from,
out of or in connection with the software or the use or other dealings in the
software.


Some of my own scratchpad Teenys documents such as XLS documents showing pin assignments and the like

Probably the most updated one is the file:

## Documents

### Arduino_GIGA_R1_pins.xlsx

This document has a few pages where I try to show the mapping of the Arduino pins to the
underlying STM hardware pin and shows the functionality of each of these pins.

## Libraries

### GIGA_digitalWriteFast

This library is a header file that provides faster versions of some of the digital pin APIs such as digitalWrite.  It is setup to both
handle pin numbers as well as Pin Names, that are defined in the variant definitions, like LED_RED.  To use these, you should first
call the standard pinMode api to the proper state.

- Include file: GIGA_digitalWriteFast.h
- digitalWriteFast(pin, state) - sets the pin to HIGH or LOW
- digitalToggleFast(pin) - Toggles the state of the pin 
- digitalReadFast(pin) - Reads the current state of the pin.


### SoftwareSerial

This library is a GIGA (maybe Portenta H7 as well but not tested yet).  
It allows you to create a logical Serial object on almost any two digital pins. 
It is different than the AVR version of SoftwareSerial, as it uses interrupts to
do the RX processing versus having to tell the library when to poll for input.

Still WIP: lots more timing work, currently I think only one instance can be defined. ...

- include file: SoftwareSerial.h
- depends on the libraries:
  - GIGA_digitalWriteFast - if debug is enabled in the source file
  - Portenta_H7_TimerInterrupt.h - for Hardware Timer.  This requires my version of the library that you can download from https://github.com/KurtE/Portenta_H7_TimerInterrupt  Note: I may make this optional later.
- Constructor:

```
SoftwareSerial(uint8_t rxPin, uint8_t txPin, bool inverse_logic = false, TIM_TypeDef* timer=TIM15);
```
Note the timer defaults to TIM15.  Will document more soon on which timers work
- Example Sketch: SoftwareSerialTest.ino 

Note: I have done most of my playing with this on the M7 processor, but have done
some testing and fixes on the M4, where I uploaded the sketch mentioned to the
M4 processor and use the sketch GIGA_RPC_Print_M7 which is in the sketchs directory of this project.

>>> More needed.... I know ...

## Sketches

>>> coming soon ... maybe 


