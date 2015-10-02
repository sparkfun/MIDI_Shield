
#include "notemap.h"

// Default constructor: initialize members to safe defaults.
notemap::notemap()
{
  // init private data members

  mode = NORMAL;
  staccato = false;

  for( uint8_t i = 0; i < 16; i++)
  {
    voice_map[i] = 0;
  }
  num_keys_held = 0;
  last_key = 0;
}

void notemap::setMode(notemap::map_mode m)
{
  mode = m;
}

notemap::map_mode notemap::getMode()
{
  return mode;
}

void notemap::setShort(bool short_on)
{
  staccato = short_on;
}

bool notemap::getShort()
{
  return staccato;
}



/////////////////////////////////////////////////////////////////////////
// Note bitmap routines.
// In a more ambitious version of this, the notemap would be a
// class, and these would be the member functions.
/////////////////////////////////////////////////////////////////////////

// Print the bitmap
void notemap::debug()
{
  for (uint8_t i = 0; i < 16; i++)
  {
    Serial.print(voice_map[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

// Set a bit in the map
void notemap::setKey(uint8_t note)
{
  uint8_t idx = note / 8;
  uint8_t pos = (note % 8) ;

  voice_map[idx] |= (0x01 << pos);

  num_keys_held ++;
}

// clear a bit in the map
void notemap::clearKey(uint8_t note)
{
  uint8_t idx = note / 8;
  uint8_t pos = (note % 8) ;

  voice_map[idx] &= ~(0x01 << pos);

  num_keys_held --;
}

bool notemap::isBitSet(uint8_t note)
{
  uint8_t idx = note / 8;
  uint8_t pos = (note % 8) ;

  return( voice_map[idx] & (0x01 << pos));
}

uint8_t notemap::getNumKeysHeld()
{
  return num_keys_held ;
}

uint8_t notemap::getLastKey()
{
  return last_key;
}

// Are any keys held?
// If so return true, and return param insidacing which one.
uint8_t notemap::getLowest()
{
  //Serial.print("Checkmap:");
  //mapDebug();

  uint8_t keynum;

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

          return keynum;
        }
      }
    }
  }

  return 0;
}

uint8_t notemap::getNext(uint8_t start)
{
  //debug();

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
    //Serial.print("key: ");
    //Serial.println(key);
    
    if(isBitSet(key))
    {
      return key;
    }
  }
  return start;
}


uint8_t notemap::whichKey()
{
  if(num_keys_held == 0)
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
      if(num_keys_held == 1)
      {
        last_key = getLowest();
      }
    }        
    break;
    case ARP_DN:
    {
      if(num_keys_held == 1)
      {
        last_key = getLowest();
      }
    }        
    break;
    case NORMAL:
    default:
    {
       last_key = getLowest();
    }        
    break;
  }

  return last_key;
}

void notemap::tickArp(bool rising)
{
  if(rising)
  {  
    clk_on = true;
    
    if(num_keys_held > 1)
    {
      last_key = getNext(last_key);
    }
  }
  else // falling
  {
    clk_on = false;
  }
}

bool notemap::getGate()
{
  if(staccato)
  {
    return clk_on & (num_keys_held > 0);
  }
  else
  {
    return (num_keys_held > 0);
  }
}

