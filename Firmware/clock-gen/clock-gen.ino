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
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);
//MIDI_CREATE_INSTANCE(SoftwareSerial, SoftSerial, MIDI);

bool running;
bool send_start;
bool send_stop;
bool send_continue;
bool send_tick;
uint32_t tempo_delay;


void play_button_event()
{
  // toggle running state, 
  // send corresponding responses
  running = !running;
    
    if(running)
    {
      send_start = true;
      digitalWrite(PIN_LED_PLAYING, LOW);
    }
    else
    {
      send_stop = true;
      digitalWrite(PIN_LED_PLAYING, HIGH);
    }
}

void cont_button_event()
{
  // ignore continue if running
  if(!running)
  {
    send_continue = true;
    running = true;
    digitalWrite(PIN_LED_PLAYING, LOW);
  }
}

void timer_callback()
{
  send_tick = true;
}

void check_pots()
{
  uint32_t pot_val;
  uint32_t calc;
  
  pot_val = analogRead(PIN_TEMPO_POT);
  
  // Result is 10 bits
  calc = (((0x3ff - pot_val) * 75)/1023) + 8;
  
  tempo_delay = calc  ;//* 5;
}

void check_buttons()
{
  uint8_t val;
  static uint16_t play_debounce = 0;
  static uint16_t cont_debounce = 0;

  // First the PLAY/STOP button  
  val = digitalRead(PIN_PLAY_INPUT);
  
  if(val == LOW)
  {
    play_debounce++;
    
    if(play_debounce == DEBOUNCE_COUNT)
    {
      play_button_event();
    }
  }
  else
  {
    play_debounce = 0;
  }

  // Then the continue button
  val = digitalRead(PIN_CONTINUE_INPUT);
  
  if(val == LOW)
  {
    cont_debounce++;
    
    if(cont_debounce == DEBOUNCE_COUNT)
    {
      cont_button_event();
    }
  }
  else
  {
    cont_debounce = 0;
  }



}


void setup() 
{
  // put your setup code here, to run once:
  
  // LED outputs
  pinMode(PIN_LED_PLAYING, OUTPUT);
  pinMode(PIN_LED_TEMPO, OUTPUT);
  digitalWrite(PIN_LED_PLAYING, HIGH);
  digitalWrite(PIN_LED_TEMPO, HIGH);
  
  // button inputs
  pinMode(PIN_PLAY_INPUT, INPUT_PULLUP);
  pinMode(PIN_CONTINUE_INPUT, INPUT_PULLUP);
  
//  Serial.begin(9600);
//  Serial.println("Setting up");
  
//  SoftSerial.begin(31250);
  
  // do I need to init the soft serial port?
  
#if 1
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
#endif  

  running = false;
  send_start = false;
  send_stop = false;
  send_tick = false;

  // prime the tempo pump
  check_pots();

//  check_timing();

  MsTimer2::set(tempo_delay, timer_callback);
  MsTimer2::start();

}

void loop() 
{
  static uint32_t loops = 0;  
  static uint8_t  ticks = 0;
  static uint8_t  prev_ticks = 0;
  bool reset_timer = false;

  // put your main code here, to run repeatedly:

  // turn the crank...
  MIDI.read();

  // Check buttons
  check_buttons();

  // process inputs
  if(send_start)
  {
    MIDI.sendRealTime(MIDI_NAMESPACE::Start);
    send_start = false;
//    Serial.println("Starting");
    
    ticks = 0;
       
    // Next tick comes immediately...
    // it also resets the timer
    send_tick = true;
    
  }
  if(send_continue)
  {
    MIDI.sendRealTime(MIDI_NAMESPACE::Continue);
    send_continue = false;
//    Serial.println("continuing");

    // Restore the LED blink counter
    ticks = prev_ticks;
    
    // Next tick comes immediately...
    // it also resets the timer
    send_tick = true;
  }
   
  if(send_stop)
  {
    MIDI.sendRealTime(MIDI_NAMESPACE::Stop);
    send_stop = false;
    prev_ticks = ticks ;
//    Serial.println("Stopping");
  }

  if(send_tick)
  {
    MIDI.sendRealTime(MIDI_NAMESPACE::Clock);
    send_tick = false;
    
    ticks++;
    if(ticks < 6)
    {
      digitalWrite(PIN_LED_TEMPO, LOW);
    }
    else if(ticks == 6)
    {
      digitalWrite(PIN_LED_TEMPO, HIGH);
    }
    else if(ticks >= 24)
    {
      ticks = 0;
    }
    
    check_pots();
    
    reset_timer = true;
  }

  if(reset_timer)
  {      
    MsTimer2::stop();
    MsTimer2::set(tempo_delay, timer_callback);
    MsTimer2::start();
    
    reset_timer = false;
  }

  loops++;
}
