/******************************************************************************
clock-recv.ino
Use SparkFun MIDI Shield as a MIDI clock receiver.

Byron Jacquot, SparkFun Electronics
October 8, 2015
https://github.com/sparkfun/MIDI_Shield/tree/V_1.5/Firmware/clock-recv

Listenn for clock/start/stop/continue messages on the MIDI input

Resources:

  This sketch has a clock generating counterpart in clock-gen.ino

  This code is dependent on the FortySevenEffects MIDI library for Arduino.
  https://github.com/FortySevenEffects/arduino_midi_library
  This was done using version 4.2, hash fb693e724508cb8a473fa0bf1915101134206c34
  This library is now under the MIT license, as well.
  You'll need to install that library into the Arduino IDE before compiling.
  
    
Development environment specifics:
  It was developed for the Arduino Uno compatible SparkFun RedBoard, with a  SparkFun
  MIDI Shield.
    
  Written, compiled and loaded with Arduino 1.6.5

This code is released under the [MIT License](http://opensource.org/licenses/MIT).

Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.

Distributed as-is; no warranty is given.
******************************************************************************/
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <MIDI.h>

#define PIN_LED_PLAYING 6
#define PIN_LED_TEMPO 7
#define PIN_PLAY_INPUT 2
#define PIN_CONTINUE_INPUT 3

#define PIN_TEMPO_POT 1

static const uint16_t DEBOUNCE_COUNT = 50;

//SoftwareSerial SoftSerial(8,9);

/* Args: 
   - type of port to use (hard/soft)
   - port object name
   - name for this midi instance
*/
//MIDI_CREATE_INSTANCE(SoftwareSerial, SoftSerial, MIDI);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

void setup() 
{
  // put your setup code here, to run once:
  
  // LED outputs
  pinMode(PIN_LED_PLAYING, OUTPUT);
  pinMode(PIN_LED_TEMPO, OUTPUT);
  digitalWrite(PIN_LED_PLAYING, HIGH);
  digitalWrite(PIN_LED_TEMPO, HIGH);
  
  // button inputs
//  Serial.begin(9600);
//  Serial.println("Setting up");
  
  //  SoftSerial.begin(31250);
  // do I need to init the soft serial port?
  // No - MIDI will do it.
  
#if 1
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
#endif  

}

void loop() 
{
  static uint32_t loops = 0;  
  static uint8_t  ticks = 0;
  static uint8_t  prev_ticks = 0;

  // put your main code here, to run repeatedly:

  // turn the crank...
  if(  MIDI.read())
  {
    switch(MIDI.getType())
    {
      case midi::Clock :
      { 
        ticks++;

        //Serial.print('.');
//        Serial.println(ticks);        
        
        if(ticks < 6)
        {
          digitalWrite(PIN_LED_TEMPO, LOW);
          //Serial.print('#');       
        }
        else if(ticks == 6)
        {
          digitalWrite(PIN_LED_TEMPO, HIGH);
        }
        else if(ticks >= 24)
        {
          ticks = 0;
//          Serial.print('\n');
        }
      }
      break;
      
      case midi::Start :
      {
        digitalWrite(PIN_LED_PLAYING, LOW);
        ticks = 0;
//        Serial.println("Starting");
      }
      break;

      case midi::Stop :
      {
        digitalWrite(PIN_LED_PLAYING, HIGH);
        prev_ticks = ticks;
//        Serial.println("Stopping");
      }
      break;
      case midi::Continue :
      {

        digitalWrite(PIN_LED_PLAYING, LOW);

        // Restore the LED blink counter
        ticks = prev_ticks;
//        Serial.println("continuing");
      }
      break;
      
      default:
      break;
    }
  }

  loops++;
}


