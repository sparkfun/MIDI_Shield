
#define KNOB1  0
#define KNOB2  1

#define BUTTON1  2
#define BUTTON2  3
#define BUTTON3  4



#define STAT1  7
#define STAT2  6

byte incomingByte;
byte note;
byte velocity;
int pot;

byte byte1;
byte byte2;
byte byte3;


int action=2; //0 =note off ; 1=note on ; 2= nada

//setup: declaring iputs and outputs and begin serial
void setup() {

  pinMode(STAT1,OUTPUT);   // declare the LED's pin as output
  pinMode(STAT2,OUTPUT);

  pinMode(BUTTON1,INPUT);
  pinMode(BUTTON2,INPUT);
  pinMode(BUTTON3,INPUT);


  digitalWrite(BUTTON1,HIGH);
  digitalWrite(BUTTON2,HIGH);
  digitalWrite(BUTTON3,HIGH);


  for(int i = 0;i < 10;i++)
  {
    digitalWrite(STAT1,HIGH);  
    digitalWrite(STAT2,LOW);
    delay(30);
    digitalWrite(STAT1,LOW);  
    digitalWrite(STAT2,HIGH);
    delay(30);
  }
  digitalWrite(STAT1,HIGH);   
  digitalWrite(STAT2,HIGH);
 
 //start serial with midi baudrate 31250 or 38400 for debugging
 Serial.begin(31250);     
  //Serial.begin(38400); 
  //Serial.println("MIDI Board");  
}

//loop: wait for serial data, and interpret the message
void loop () {


  /* 
   // Button and knob test functions
   if(analogRead(KNOB1) > 512){ digitalWrite(STAT1,HIGH); }
   else{ digitalWrite(STAT1,LOW); }
   if(analogRead(KNOB2) > 512){ digitalWrite(STAT2,HIGH); }
   else{ digitalWrite(STAT2,LOW); }
   
   
   if(!digitalRead(BUTTON1)){ digitalWrite(STAT1,LOW); digitalWrite(STAT2,LOW); }
   else if(!digitalRead(BUTTON2)){ digitalWrite(STAT1,HIGH); digitalWrite(STAT2,LOW); }
   else if(!digitalRead(BUTTON3)){ digitalWrite(STAT1,LOW); digitalWrite(STAT2,HIGH); }
   else if(!digitalRead(BUTTON4)){ digitalWrite(STAT1,HIGH); digitalWrite(STAT2,HIGH); }
   */
  
  //*************** MIDI OUT ***************//

  pot = analogRead(0);
  note = pot/8;  // convert value to value 0-127
  if(button(BUTTON1) || button(BUTTON2) || button(BUTTON3))
  {
    
    noteOn(0x95,0x3C,0x45);
    digitalWrite(STAT2,LOW);
    while(button(BUTTON1) || button(BUTTON2) || button(BUTTON3));
  }
  /*while(1);
  else
  {
    noteOn(0x90,0x3C,0x00);
    digitalWrite(STAT2,HIGH);
  }*/
  
  //*************** MIDI LOOPBACK ******************//
  /*
  if(Serial.available() > 0)
  {
    byte1 = Serial.read();
    byte2 = Serial.read();
    byte3 = Serial.read();
  
    noteOn(byte1, byte2, byte3);
    
    //digitalWrite(STAT1,HIGH);  
    //digitalWrite(STAT2,HIGH);
  }*/
  //*************** MIDI IN ***************//
  /*
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    // wait for as status-byte, channel 1, note on or off
    if (incomingByte== 144){ // note on message starting starting
      action=1;
    }
    else if (incomingByte== 128){ // note off message starting
      action=0;
    }
    else if (incomingByte== 208){ // aftertouch message starting
      //not implemented yet
    }
    else if (incomingByte== 160){ // polypressure message starting
      //not implemented yet
    }
    else if ( (action==0)&&(note==0) ){ // if we received a "note off", we wait for which note (databyte)
      note=incomingByte;
      playNote(note, 0);
      note=0;
      velocity=0;
      action=2;
    }
    else if ( (action==1)&&(note==0) ){ // if we received a "note on", we wait for the note (databyte)
      note=incomingByte;
    }
    else if ( (action==1)&&(note!=0) ){ // ...and then the velocity
      velocity=incomingByte;
      playNote(note, velocity);
      note=0;
      velocity=0;
      action=0;
    }
    else{
      //nada
    }
  }*/
  
}

void noteOn(byte cmd, byte data1, byte data2) {
   Serial.print(cmd, BYTE);
   Serial.print(data1, BYTE);
   Serial.print(data2, BYTE);
 }

void blink(){
  digitalWrite(STAT1, HIGH);
  delay(100);
  digitalWrite(STAT1, LOW);
  delay(100);
}


void playNote(byte note, byte velocity)
{
  int value=LOW;
  if (velocity >10){ 
    value=HIGH; 
  }
  else{ 
    value=LOW; 
  }

  //since we don't want to "play" all notes we wait for a note between 36 & 44
  if(note>=36 && note<44)
  { 
    byte myPin=note-34; // to get a pinnumber between 2 and 9
    digitalWrite(myPin, value);
  }

}

char button(char button_num)
{
  return (!(digitalRead(button_num)));
}
