/******************************************************************************
MIDI-sniffer.ino
Use SparkFun MIDI Shield as a MIDI data analyzer.

Byron Jacquot, SparkFun Electronics
October 8, 2015
https://github.com/sparkfun/MIDI_Shield/tree/V_1.5/Firmware/MIDI-sniffer

Reads all events arriving over MIDI, and turns them into descriptive text.
If you hold the button on D2, it will switch to display the raw hex values arriving,
which can be useful for viewing incomplete messages and running status.

Resources:

  Requires that the MIDI Sheild be configured to use soft serial on pins 8 & 9, 
  so that debug text can be printed to the hardware serial port.

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

#define PIN_RAW_INPUT 2

#define PIN_POT_A0 0
#define PIN_POT_A1 1

static const uint16_t DEBOUNCE_COUNT = 50;

// Need to use soft serial, so we can report what's happening
// via messages on hard serial.
SoftwareSerial SoftSerial(8, 9);

/* Args:
   - type of port to use (hard/soft)
   - port object name
   - name for this midi instance
*/
MIDI_CREATE_INSTANCE(SoftwareSerial, SoftSerial, MIDI);
// This doesn't make much sense to use with hardware serial, as it needs 
// hard serial to report what it's seeing...

void setup()
{
  // put your setup code here, to run
  once:

  // LED outputs
  Serial.begin(19200);
  Serial.println("Setting up");

  // do I need to init the soft serial port?
  // No - MIDI Lib will do it.

  // We want to receive messages on all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);
  
  // We also want to echo the input to the output, 
  // so the sniffer can be dropped inline when things misbehave.
  MIDI.turnThruOn();

  pinMode(PIN_RAW_INPUT, INPUT_PULLUP);

}

void loop()
{
  static uint8_t  ticks = 0;
  static uint8_t  old_ticks = 0;


  // put your main code here, to run repeatedly:

  if(digitalRead(PIN_RAW_INPUT) == LOW)
  {
    // If you hold button D2 on the shield, we'll print
    // the raw hex values from the MIDI input.
    //
    // This can be useful if you need to troubleshoot issues with
    // running status

    byte input;
    if(SoftSerial.available() != 0)
    {
      input = SoftSerial.read();
    
      if(input & 0x80)
      {
        Serial.println();
      }
      Serial.print(input, HEX);
      Serial.print(' ');
    }
  }
  else
  {
    // turn the crank...
    if (  MIDI.read())
    {
      switch (MIDI.getType())
      {
        case midi::NoteOff :
          {
            Serial.print("NoteOff, chan: ");
            Serial.print(MIDI.getChannel());
            Serial.print(" Note#: ");
            Serial.print(MIDI.getData1());
            Serial.print(" Vel#: ");
            Serial.println(MIDI.getData2());
          }
          break;
        case midi::NoteOn :
          {
            uint8_t vel;

            Serial.print("NoteOn, chan: ");
            Serial.print(MIDI.getChannel());
            Serial.print(" Note#: ");
            Serial.print(MIDI.getData1());
            Serial.print(" Vel#: ");
            vel = MIDI.getData2();
            Serial.print(vel);
            if (vel == 0)
            {
              Serial.print(" *Implied off*");
            }
            Serial.println();
          }
          break;
        case midi::AfterTouchPoly :
          {
            Serial.print("PolyAT, chan: ");
            Serial.print(MIDI.getChannel());
            Serial.print(" Note#: ");
            Serial.print(MIDI.getData1());
            Serial.print(" AT: ");
            Serial.println(MIDI.getData2());
          }
          break;
        case midi::ControlChange :
          {
            Serial.print("Controller, chan: ");
            Serial.print(MIDI.getChannel());
            Serial.print(" Controller#: ");
            Serial.print(MIDI.getData1());
            Serial.print(" Value: ");
            Serial.println(MIDI.getData2());
          }
          break;
        case midi::ProgramChange :
          {
            Serial.print("PropChange, chan: ");
            Serial.print(MIDI.getChannel());
            Serial.print(" program: ");
            Serial.println(MIDI.getData1());
          }
          break;
        case midi::AfterTouchChannel :
          {
            Serial.print("ChanAT, chan: ");
            Serial.print(MIDI.getChannel());
            Serial.print(" program: ");
            Serial.println(MIDI.getData1());

          }
          break;
        case midi::PitchBend :
          {
            uint16_t val;

            Serial.print("Bend, chan: ");
            Serial.print(MIDI.getChannel());

            // concatenate MSB,LSB
            // LSB is Data1
            val = MIDI.getData2() << 7 | MIDI.getData1();

            Serial.print(" value: 0x");
            Serial.println(val, HEX);


          }
          break;
        case midi::SystemExclusive :
          {
            // Sysex is special.
            // could contain very long data...
            // the data bytes form the length of the message,
            // with data contained in array member
            uint16_t length;
            const uint8_t  * data_p;

            Serial.print("SysEx, chan: ");
            Serial.print(MIDI.getChannel());
            length = MIDI.getSysExArrayLength();

            Serial.print(" Data: 0x");
            data_p = MIDI.getSysExArray();
            for (uint16_t idx = 0; idx < length; idx++)
            {
              Serial.print(data_p[idx], HEX);
              Serial.print(" 0x");
            }
            Serial.println();
          }
          break;
        case midi::TimeCodeQuarterFrame :
          {
            // MTC is also special...
            // 1 byte of data carries 3 bits of field info 
            //      and 4 bits of data (sent as MS and LS nybbles)
            // It takes 2 messages to send each TC field,
          
            Serial.print("TC 1/4Frame, type: ");
            Serial.print(MIDI.getData1() >> 4);
            Serial.print("Data nybble: ");
            Serial.println(MIDI.getData1() & 0x0f);
          }
          break;
        case midi::SongPosition :
          {
            // Data is the number of elapsed sixteenth notes into the song, set as 
            // 7 seven-bit values, LSB, then MSB.
          
            Serial.print("SongPosition ");
            Serial.println(MIDI.getData2() << 7 | MIDI.getData1());
          }
          break;
        case midi::SongSelect :
          {
            Serial.print("SongSelect ");
            Serial.println(MIDI.getData1());
          }
          break;
        case midi::TuneRequest :
          {
            Serial.println("Tune Request");
          }
          break;
        case midi::Clock :
          {
            ticks++;

            Serial.print("Clock ");
            Serial.println(ticks);
          }
          break;
        case midi::Start :
          {
            ticks = 0;
            Serial.println("Starting");
          }
          break;
        case midi::Continue :
          {
            ticks = old_ticks;
            Serial.println("continuing");
          }
          break;
        case midi::Stop :
          {
            old_ticks = ticks;
            Serial.println("Stopping");
          }
          break;
        case midi::ActiveSensing :
          {
            Serial.println("ActiveSense");
          }
          break;
        case midi::SystemReset :
          {
            Serial.println("Stopping");
          }
          break;
        case midi::InvalidType :
          {
            Serial.println("Invalid Type");
          }
          break;
        default:
          {
            Serial.println();
          }
          break;
      }
    }
  }
}



