
#include "notemap.h"


notemap::notemap()
{
    for(uint8_t i = 0; i < 16; i++)
    {
      keys[i] = 0;
    }
    numKeys = 0;
}  

void notemap::debug()
{
  Serial.print("Notemap: numKeys: ");  
  Serial.print(numKeys);
  Serial.print(" Array: ");
  for (uint8_t i = 0; i < 16; i++)
  {
    Serial.print(keys[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

void notemap::setBit(uint8_t note)
{
  uint8_t idx = note / 8;
  uint8_t pos = (note % 8) ;

  keys[idx] |= (0x01 << pos);

  numKeys ++;
}

void notemap::clearBit(uint8_t note)
{
  uint8_t idx = note / 8;
  uint8_t pos = (note % 8) ;

  keys[idx] &= ~(0x01 << pos);

  numKeys --;
}

void notemap::clearAll()
{
    for(uint8_t i = 0; i < 16; i++)
    {
      keys[i] = 0;
    }
    numKeys = 0;
}
  

bool notemap::isBitSet(uint8_t note)
{  
  uint8_t idx = note / 8;
  uint8_t pos = (note % 8) ;

  return( keys[idx] & (0x01 << pos));
}

uint8_t notemap::getLowest()
{
 //Serial.print("Checkmap:");
  //mapDebug();

  uint8_t keynum;

  // starting at bottom gives us low note priority.
  // could alternately start from the top...
  for (uint8_t i = 0; i < 16; i++)
  {
    if (keys[i] != 0x0)
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
        if (keys[i] & j)
        {
          keynum = (i * 8) + k ;

          return keynum;
        }
      }
    }
  }

  return 0;
}


uint8_t notemap::getNumBits()
{
  return numKeys;  
}


/////////////////////////////////////////////////////////////////////////
////////
/////////////////////////////////////////////////////////////////////////

notetracker::notetracker()
{
    mode = NORMAL;
    
    staccato   = false;
    sustaining = false;
    clk_hi     = false;

    //notemap sustain_map;
    active_map_p = &voice_map;
    
    last_key = 0;
}

void notetracker::noteOn(uint8_t key)
{
  voice_map.setBit(key);

  if(sustaining)
  {
    sustain_map.setBit(key);
  }
}

void notetracker::noteOff(uint8_t key)
{
  voice_map.clearBit(key);
}

void notetracker::setMode(notetracker::tracker_mode m)
{
  mode = m;
}

notetracker::tracker_mode notetracker::getMode()
{
  return mode;
}

void notetracker::setShort(bool short_on)
{
  staccato = short_on;
}

bool notetracker::getShort()
{
  return staccato;
}

void notetracker::setSustain(bool on)
{
  
  Serial.print("Sustain: ");
  Serial.println(sustaining);

  if(on)
  {
    sustaining = true;

    // Should give us a member-by-member copy of object??
    sustain_map = voice_map;

    active_map_p = &sustain_map;
    
  }
  else
  {
    sustaining = false;

    sustain_map.clearAll();

    active_map_p = &voice_map;
  }

}

#if 0
uint8_t notetracker::getLowest()
{
  //Serial.print("Checkmap:");
  //mapDebug();

  uint8_t keynum;

  // starting at bottom gives us low note priority.
  // could alternately start from the top...
  for (uint8_t i = 0; i < 128; i++)
  {
    if (active_map_p->isBitSet(i))
    {
      return i;
    }
  }

  return 0;
}
#endif

uint8_t notetracker::getNext(uint8_t start)
{
  int8_t step;

  if(mode == ARP_UP)
  {
    step = 1;
  }
  else if(mode == ARP_DN)
  {
    step = -1;
  }

  for(uint8_t key = start+step;  key != start; key = (key+step)%128)
  {
    if(active_map_p->isBitSet(key))
    {
      return key;
    }
  }
  return start;  
}


void notetracker::tickArp(bool rising)
{
  if(rising)
  {  
    clk_hi = true;
    
    if(active_map_p->getNumBits() > 1)
    {
      last_key = getNext(last_key);
    }
  }
  else // falling
  {
    clk_hi = false;
  }
}

uint8_t notetracker::whichKey()
{
  uint8_t num_keys = active_map_p->getNumBits();
  
  if(num_keys == 0)
  {
    return last_key;
  }
  
  switch(mode)
  {
    case ARP_UP:
    {
      // When there's only one key held, we need to respond to it 
      // immediately.
      // If multiple keys are held, advanceArp() sets last_key
      // based on timer. 
      if(num_keys == 1)
      {
        last_key = active_map_p->getLowest();
      }
    }        
    break;
    case ARP_DN:
    {
      if(num_keys == 1)
      {
        last_key = active_map_p->getLowest();
      }
    }        
    break;
    case NORMAL:
    default:
    {
       last_key = active_map_p->getLowest();
    }        
    break;
  }

  return last_key;
  
}

uint8_t notetracker::getLastKey()
{
  return last_key;  
}

bool notetracker::getGate()
{
  if(staccato)
  {
    return clk_hi & ((active_map_p->getNumBits())> 0);
  }
  else
  {
    return ((active_map_p->getNumBits()) > 0);
  }
}
    

