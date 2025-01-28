/*
  Analog Input

  Demonstrates analog input by reading an analog sensor on analog pin 0 and
  turning on and off a light emitting diode(LED) connected to digital pin 13.
  The amount of time the LED will be on and off depends on the value obtained
  by analogRead().

  The circuit:
  - potentiometer
    center pin of the potentiometer to the analog input 0
    one side pin (either one) to ground
    the other side pin to +5V
  - LED
    anode (long leg) attached to digital output 13 through 220 ohm resistor
    cathode (short leg) attached to ground

  - Note: because most Arduinos have a built-in LED attached to pin 13 on the
    board, the LED is optional.

  created by David Cuartielles
  modified 30 Aug 2011
  By Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/AnalogInput
*/

int sensorPin = A0;        // select the input pin for the potentiometer
int ledPin = LED_BUILTIN;  // select the pin for the LED
int sensorValue = 0;       // variable to store the value coming from the sensor
int sensorValuePrev = -1;
const int analog_pins[] = { A0, A1, A2, A3, A4, A5, A6, A7, 0, 1, 2, 3 };
#ifndef ARDUINO_ARDUINO_NANO33BLE
const PureAnalogPin *pure_pins[] = { &A8, &A9, &A10, &A11 };
#endif

void setup() {
    // declare the ledPin as an OUTPUT:
    analogReadResolution(12);
    pinMode(ledPin, OUTPUT);
    Serial.begin(115200);
}


void loop() {
    // read the value from the sensor:
#ifndef ARDUINO_ARDUINO_NANO33BLE
    sensorValue = (sensorPin >= A0) ? analogRead(sensorPin) : analogRead(*pure_pins[sensorPin]);
#else
    sensorValue = analogRead(sensorPin);
#endif
    if (sensorValue != sensorValuePrev) {
        Serial.println(sensorValue);
        sensorValuePrev = sensorValue;
    }
    // turn the ledPin on
    digitalWrite(ledPin, HIGH);
    // stop the program for <sensorValue> milliseconds:
    delay((sensorValue < 2000) ? sensorValue : 2000);
    // turn the ledPin off:
    digitalWrite(ledPin, LOW);
    // stop the program for <sensorValue> milliseconds:
    delay((sensorValue < 2000) ? sensorValue : 2000);

    if (Serial.available()) {
        int pin_num = Serial.parseInt();
        while (Serial.available()) Serial.read();
        if (pin_num < 0) {
            // use for resolution
            pin_num = -pin_num;
            Serial.print("Setting analogReadResolution:");
            Serial.println(pin_num);
            analogReadResolution(pin_num);
        } else {
            sensorPin = analog_pins[pin_num];
            Serial.print("New pin A");
            Serial.print(pin_num);
            Serial.print(" : ");
            Serial.println(sensorPin);
        }
        sensorValuePrev = -1;
    }
}
