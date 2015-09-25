/* Werkstatt MIDI-CV interface DAC integration test
   by: Byron Jacquot
   SparkFun Electronics
   date: October 9, 2014
   license: Public Domain - please use this code however you'd like.
   It's provided as a learning tool.

   Generates opposing sawtooth waves on two I2C DACs so we can verify that
   they've got the proper address configuration.
 */

#include <Arduino.h>
#include <Wire.h>

#include <utility\twi.h>

void setup()
{
  Serial.begin(9600);
  
  Wire.begin();
  
  analogReference(INTERNAL);
  
  // Scope loop
  pinMode(8, OUTPUT);
  
#define TWI_FREQ_NUNCHUCK 800000L
  TWBR = ((F_CPU / TWI_FREQ_NUNCHUCK) - 16) / 2;
}

uint16_t val = 0;

void loop()
{
  
  digitalWrite(8, HIGH);
#if 1
  Wire.beginTransmission(0x60);
  Wire.write(byte(val >> 8));
  Wire.write(byte(val & 0xff));
  Wire.endTransmission();
#endif  
#if 1
  uint16_t val2 = 0xfff - val;
  Wire.beginTransmission(0x61);
  Wire.write(byte(val2 >> 8));
  Wire.write(byte(val2 & 0xff));
  Wire.endTransmission();
#endif  
  val++;
  val &= 0x0fff;
  
  digitalWrite(8, LOW);
}

