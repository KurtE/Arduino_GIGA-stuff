/* Blink inbuilt LED example */

#include <Arduino.h>
#include <LibPrintf.h>
#include "Wire.h"

void setup()
{
	printf("\n\nSetup begins\n");
	Wire1.begin();
  Wire1.setClock(	100000 );
	// initialize the LED pin as an output:

	// initialize the pushbutton pin as an input:
	// pinMode(buttonPin, INPUT);
	// pinMode(ledPin, OUTPUT);
	Wire1.beginTransmission(0x53);
	Wire1.write(0x2C);
	Wire1.write(0x08);
	Wire1.endTransmission();

	Wire1.beginTransmission(0x53);
	Wire1.write(0x31);
	Wire1.write(0x08);
	Wire1.endTransmission();

	Wire1.beginTransmission(0x53);
	Wire1.write(0x2D);
	Wire1.write(0x08);
	Wire1.endTransmission();
	printf("\n\nSetup COMPLETE\n\n\n");
}

void loop()
{
	Wire1.beginTransmission(0x53);
Wire1.write(0x32); 
Wire1.endTransmission();
// printf("\n\nrequesting from 53\n\n\n");
Wire1.requestFrom(0x53, 1);
byte x0 = Wire1.read();

Wire1.beginTransmission(0x53);
Wire1.write(0x33); 
Wire1.endTransmission();
Wire1.requestFrom(0x53, 1);
byte x1 = Wire1.read();
x1 = x1 & 0x03;

uint16_t x = (x1 << 8) + x0;
int16_t xf = x;
if(xf > 511)
{
xf = xf - 1024;
}
float xa = xf * 0.004;
printf("\n\nX = %f\n",xa);
// Serial.print("X = "); 
// Serial.print(xa);
// Serial.print(" g"); 
// Serial.println(); 


Wire1.beginTransmission(0x53);
Wire1.write(0x34); 
Wire1.endTransmission();
Wire1.requestFrom(0x53, 1);
byte y0 = Wire1.read();

Wire1.beginTransmission(0x53);
Wire1.write(0x35); 
Wire1.endTransmission();
Wire1.requestFrom(0x53, 1);
byte y1 = Wire1.read();
y1 = y1 & 0x03;

uint16_t y = (y1 << 8) + y0;
int16_t yf = y;
if(yf > 511)
{
yf = yf - 1024;
}
float ya = yf * 0.004;
// printk("Y = %f\n",ya);
printf("Y = %f\n",ya);
// printf("\n\nYa = %f\n\n\n",ya);
// Serial.print("Y = "); 
// Serial.print(ya);
// Serial.print(" g"); 
// Serial.println(); 

Wire1.beginTransmission(0x53);
Wire1.write(0x36); 
Wire1.endTransmission();
Wire1.requestFrom(0x53, 1);
byte z0 = Wire1.read();

Wire1.beginTransmission(0x53);
Wire1.write(0x37); 
Wire1.endTransmission();
Wire1.requestFrom(0x53, 1);
byte z1 = Wire1.read();
z1 = z1 & 0x03;

uint16_t z = (z1 << 8) + z0;
int16_t zf = z;
if(zf > 511)
{
zf = zf - 1024;
}
float za = zf * 0.004;
printf("Z = %f\n\n",za);
// Serial.print("Z = "); 
// Serial.print(za);
// Serial.print(" g"); 
// Serial.println(); 
// Serial.println(); 
delay(2);

}