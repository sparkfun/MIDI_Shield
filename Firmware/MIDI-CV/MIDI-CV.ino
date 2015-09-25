/* Werkstatt MIDI-CV interface
   by: Byron Jacquot
   SparkFun Electronics
   date: October 9, 2014
   license: Public Domain - please use this code however you'd like.
   It's provided as a learning tool.

   This code is dependent on the FortySevenEffects MIDI library for Arduino.
   https://github.com/FortySevenEffects/arduino_midi_library
   This was done using version 4.2, hash 352e3d10fb

   You'll need to install that library into the Arduino IDE before compiling.

   Please be aware that the FortySevenEffects MIDI library is licensed under
   GPL version 3.0.
*/

#include <SoftwareSerial.h>
#include <MIDI.h>
#include <Wire.h>

// Instantiate the MIDI interface using the macro
// - HardwareSerial is the type of serial port to be used underneath
// the MIDI routines.
// - Serial1 is the name of that portto be used.  On a pro-micro, "Serial"
// is the USB serial port, and "Serial1" is the hardware UART.
// - "MIDI" parameter is the resulting object name.
SoftwareSerial SoftSerial(8, 9);
MIDI_CREATE_INSTANCE(SoftwareSerial, SoftSerial, MIDI);

// A pin to use for the gate output.
static const int GATEPIN = 10;

// global variables
//
// A bitmap to track all of the midi keys - 7-bits worth = 128 bits.
// When we see a note-on, we'll set the corresponding bit.  When we see a note
// off, we'll clear it.  When we need to update the CV, we'll find the
// lowest bit set in the map.
static uint8_t voice_map[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// The last key records the LUT index of the last key value we used.
// The last bend records the most recently seen bend message.
// We need to keep track so we can update note CV when we get new notes,
// or new bend messages - we need the other half in order to put them together.
static uint8_t last_key = 0;
static int16_t last_bend = 0;//bend is signed?

// constants to describe the MIDI input.
// NUM_KEYS is the number of keys we're interpreting
// BASE_KEY is the offset of the lowest key number
static const int8_t NUM_KEYS = 49;
static const int8_t BASE_KEY = 36;


// LUT for the key # to DAC value conversion.
// This was calculated by the spreadsheet in the theory directory, and just
// pasted in.
//
// The thinking here was that it would do 49 keys worth - a 4 octave keyboard.
// It would start at about 0.5V, and go up to 4.5V.  The bender would then
// add or subtract 0.5V to the key value, so the overall span is 5 octaves,
// from 0V to 5V.
static const uint16_t note_map[NUM_KEYS] =
#if 0
{
  384,  448,  512,  576,  640,  704,  768,  832,  896,  960,  1024, 1088,
  1152, 1216, 1280, 1344, 1408, 1472, 1536, 1600, 1664, 1728, 1792, 1856,
  1920, 1984, 2048, 2112, 2176, 2240, 2304, 2368, 2432, 2496, 2560, 2624,
  2688, 2752, 2816, 2880, 2944, 3008, 3072, 3136, 3200, 3264, 3328, 3392,
  3456
};
  { //c    c#    d     d#    e     f     f#    g     g#    a     a#    b
    410,  478,  546,  614,  683,  751,  819,  887,  956,  1024, 1092, 1161,
    1229, 1297, 1365, 1434, 1502, 1570, 1638, 1707, 1775, 1843, 1911, 1980,
    2048, 2116, 2185, 2253, 2321, 2389, 2458, 2526, 2594, 2662, 2731, 2799,
    2867, 2935, 3004, 3072, 3140, 3209, 3277, 3345, 3413, 3482, 3550, 3618,
    3686
  };
#else
  {
410	,
478	,
546	,
614	,
683	,
751	,
819	,
887	,
956	,
1024	,
1092	,
1161	,
1229	,
1297	,
1365	,
1434	,
1502	,
1570	,
1638	,
1707	,
1775	,
1843	,
1911	,
1980	,
2048	,
2116	,
2185	,
2253	,
2321	,
2389	,
2458	,
2526	,
2594	,
2662	,
2731	,
2799	,
2867	,
2935	,
3004	,
3072	,
3140	,
3209	,
3277	,
3345	,
3413	,
3482	,
3550	,
3618	,
3686	
};
#endif

/////////////////////////////////////////////////////////////////////////
// Note bitmap routines.
// In a more ambitious version of this, the notemap would be a
// class, and these would be the member functions.
/////////////////////////////////////////////////////////////////////////

// Print the bitmap
void mapDebug()
{
  for (uint8_t i = 0; i < 16; i++)
  {
    Serial.print(voice_map[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

// Set a bit in the map
void mapSet(uint8_t note)
{
  uint8_t idx = note / 8;
  uint8_t pos = (note % 8) ;

  voice_map[idx] |= (0x01 << pos);
}

// clear a bit in the map
void mapClear(uint8_t note)
{
  uint8_t idx = note / 8;
  uint8_t pos = (note % 8) ;

  voice_map[idx] &= ~(0x01 << pos);
}

// Are any keys held?
// If so return true, and return param insidacing which one.
bool mapCheck(uint8_t & keynum)
{
  //Serial.print("Checkmap:");
  //mapDebug();

  // starting at bottom gives us low note priority.
  // could alternately start from the top...
  for (uint8_t i = 0; i < 16; i++)
  {
    if (voice_map[i] != 0x0)
    {
      // find the lowest bit set
      uint8_t j, k;
      for (j = 0x1, k = 0; k < 8; j <<= 1, k++)
      {
#if 0
        Serial.print("j: ");
        Serial.print(j);
        Serial.print("k: ");
        Serial.println(k);
#endif
        if (voice_map[i] & j)
        {
          keynum = (i * 8) + k ;

          return true;
        }
      }
    }
  }

  keynum = 0;
  return false;
}


// CV can be updated by both pitch bend and note-on messages
void updateCV()
{
//  uint16_t val = note_map[last_key];

#if 0
  Serial.print("KEY: ");
  Serial.print(last_key);
#endif
  uint32_t val = 400ul + ((last_key * 6826ul)/100ul);

#if 0  
  val = last_key * 6826ul;
  Serial.print(" VALa: ");
  Serial.print(val, HEX);
  
  val /= 100;
  Serial.print(" VALb: ");
  Serial.print(val, HEX);

  val += 400 ;
  Serial.print(" VALc: ");
  Serial.print(val, HEX);
#endif
  val += last_bend;

//  Serial.print(" VAL2: ");
//  Serial.println(val, HEX);


  Wire.beginTransmission(0x60);
  Wire.write(byte((val & 0x0f00) >> 8));
  Wire.write(byte(val & 0xff));
  Wire.endTransmission();

}

// Update otputs sets the outputs to the current conditions.
// Called from note on & note off.
void updateOutputs()
{
  uint8_t key;

  if (mapCheck(key))
  {
    Serial.print("key: ");
    Serial.println(key, HEX);

    // key is in terms of MIDI note number.
    // Constraining the key number to the range of valid indices for the LUT.
    if (key < BASE_KEY)
    {
      last_key = 0;
    }
    else if ( key > BASE_KEY + NUM_KEYS)
    {
      last_key = NUM_KEYS;
    }
    else
    {
      last_key = key - BASE_KEY;
    }

    updateCV();

    digitalWrite(GATEPIN, HIGH);
  }
  else
  {
    digitalWrite(GATEPIN, LOW);
  }
}

/////////////////////////////////////////////////////////////////////////
// Callbacks for the MIDI parser
/////////////////////////////////////////////////////////////////////////

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  // Do whatever you want when a note is pressed.
  // Try to keep your callbacks short (no delays ect)
  // otherwise it would slow down the loop() and have a bad impact
  // on real-time performance.

  Serial.print("on: ");
  Serial.println(pitch , HEX);

  mapSet(pitch);

  updateOutputs();

}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  // Do something when the note is released.
  // Note that NoteOn messages with 0 velocity are interpreted as NoteOffs.
  Serial.print("off: ");
  Serial.println(pitch , HEX);

  mapClear(pitch);

  updateOutputs();

}

void handlePitchBend(byte channel, int bend)
{
#if  1
  Serial.print("bend: ");
  Serial.println(bend , HEX);
#endif
  // Bend is 14 bits, signed
  // dual-7-bit thwacking already handled by midi parser

  last_bend = bend >> 5;

#if 0
  Serial.print("newbend: ");
  Serial.println(last_bend, HEX);
#endif

  updateCV();
}

void handleCC(byte channel, byte number, byte value)
{
  Serial.print("cc: ");
  Serial.print(number);
  Serial.print(" chan: ");
  Serial.print(channel, HEX);
  Serial.print("val: ");
  Serial.println(value, HEX);

  switch (number)
  {
    case 1:
      {

        Wire.beginTransmission(0x61);
        // Turn 7 bits into 12
        Wire.write(byte((value & 0x70) >> 3));
        Wire.write(byte((value & 0x0f) << 4));
        Wire.endTransmission();
      };
      break;

    // Other CC's would line up here...

    default:
      break;
  }

}

/////////////////////////////////////////////////////////////////////////
// Arduino boilerplate - setup() & loop()
/////////////////////////////////////////////////////////////////////////

void setup()
{
  pinMode(GATEPIN, OUTPUT);  // Set RX LED as an output
  // TX LED is set as an output behind the scenes

  Serial.begin(9600); //This pipes to the serial monitor
  // Serial1.begin(31250, SERIAL_8N1); //This is the UART, pipes to sensors attached to board

  Wire.begin();
#define TWI_FREQ_NUNCHUCK 800000L
  TWBR = ((F_CPU / TWI_FREQ_NUNCHUCK) - 16) / 2;


  // Initiate MIDI communications, listen to all channels
  // .begin sets the thru mode to on, so we'll have to turn it off if we don't want echo
  MIDI.begin(MIDI_CHANNEL_OMNI);

  MIDI.turnThruOff();

  // so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(handleNoteOn);  // Put only the name of the function
  // Do the same for NoteOffs
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleCC);
  MIDI.setHandlePitchBend(handlePitchBend);


  Serial.println("setup complete");
}

void loop()
{
  // Pump the MIDI parser as quickly as we can.
  // This will invoke the callbacks when messages are parsed.
  MIDI.read();

}

