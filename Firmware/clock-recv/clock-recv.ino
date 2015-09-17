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


