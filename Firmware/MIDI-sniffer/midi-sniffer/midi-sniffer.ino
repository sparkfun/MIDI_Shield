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

void setup()
{
  // put your setup code here, to run once:

  // LED outputs
  Serial.begin(19200);
  Serial.println("Setting up");

  // do I need to init the soft serial port?
  // No - MIDI will do it.

#if 1
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
#endif

  pinMode(PIN_RAW_INPUT, INPUT_PULLUP);

}

void loop()
{
  static uint8_t  ticks = 0;

  // put your main code here, to run repeatedly:

  // turn the crank...
  
  if(digitalRead(PIN_RAW_INPUT) == LOW)
  {
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

            Serial.print('Clock ');
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
            Serial.println("continuing");
          }
          break;
        case midi::Stop :
          {
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



