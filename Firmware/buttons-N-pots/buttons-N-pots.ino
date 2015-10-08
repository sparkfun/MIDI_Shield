/******************************************************************************
buttons-N-pots.ino
Test the physical controls on SparkFun MIDI Shield.

Byron Jacquot, SparkFun Electronics
October 8, 2015
https://github.com/sparkfun/MIDI_Shield/tree/V_1.5/Firmware/buttons-N-pots

A quick test to show that the bottons and pots on the MIDI Shield are functional.
Reads their values, and prints out reports when they change.

It doesn't exercise any of the MIDI functionality.  See the other 
sketches for MIDI examples:
https://github.com/sparkfun/MIDI_Shield/tree/V_1.5/Firmware

Development environment specifics:
  It was developed for the Arduino Uno compatible SparkFun RedBoard, with a  SparkFun
  MIDI Shield.
    
  Written, compiled and loaded with Arduino 1.6.5

This code is released under the [MIT License](http://opensource.org/licenses/MIT).

Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.

Distributed as-is; no warranty is given.
******************************************************************************/


static const uint8_t PIN_BTN0 = 2;
static const uint8_t PIN_BTN1 = 3;
static const uint8_t PIN_BTN2 = 4;

static const uint8_t PIN_POT0 = 0;
static const uint8_t PIN_POT1 = 1;

static const uint8_t PIN_LED_GRN = 6;
static const uint8_t PIN_LED_RED = 7;

typedef struct inputStates
{
  uint8_t button[3];
  uint8_t pot[2];
}inputStates;

inputStates lastdata;
inputStates newdata;

void readinputs(struct inputStates * data)
{
  // A bit of a funny statement below.
  // digitalRead return HIGH or LOW.
  // We want boolean (true/false) indicators of whether the button are
  // pushed.
  // And button inputs are active low - when a button is pushed, it'll read "LOW"
  // The right side of the expression checks if the input is equal to LOW, converting
  // that into a boolean indicator, stored in the array.  
  data->button[0] = (digitalRead(PIN_BTN0) == LOW);
  data->button[1] = (digitalRead(PIN_BTN1) == LOW);
  data->button[2] = (digitalRead(PIN_BTN2) == LOW);

  // Analog inputs have an LSB (out of 10 bits) or so noise, 
  // leading to "chatter" in the change detector logic.
  // Shifting off the 2 LSBs to remove it
  data->pot[0] = analogRead(PIN_POT0) >> 2;
  data->pot[1] = analogRead(PIN_POT1) >> 2;
}

void compareinputs(inputStates * old_p, inputStates * new_p)
{
  uint8_t idx;
  
  for(idx = 0; idx < 3; idx++)
  {
    if(old_p->button[idx] != new_p->button[idx])
    {
      old_p->button[idx] = new_p->button[idx];
      Serial.print("Button #");
      Serial.print(idx);
      Serial.print(" changed to ");
      Serial.println(old_p->button[idx]);
    }
  }

  for(idx = 0; idx < 2; idx++)
  {
    if(old_p->pot[idx] != new_p->pot[idx])
    {
      old_p->pot[idx] = new_p->pot[idx];
      Serial.print("Pot #");
      Serial.print(idx);
      Serial.print(" changed to ");
      Serial.println(old_p->pot[idx]);
    }
  }
}

void updateLEDs(inputStates * data)
{
  digitalWrite(PIN_LED_GRN, !data->button[0]);
  digitalWrite(PIN_LED_RED, !data->button[1]);
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(19200);    
  Serial.println("Setting up.");
  
  pinMode(PIN_BTN0, INPUT_PULLUP);
  pinMode(PIN_BTN1, INPUT_PULLUP);
  pinMode(PIN_BTN2, INPUT_PULLUP);

  pinMode(PIN_LED_GRN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);

  readinputs(&lastdata);
}

void loop() {
  // put your main code here, to run repeatedly:

  readinputs(& newdata);
  compareinputs(& lastdata, & newdata);
  updateLEDs(&newdata);
}
