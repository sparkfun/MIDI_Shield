#pragma once

#include <Arduino.h>
#include <stdint.h>

class notemap
{
  public:

         notemap();

    void debug();

    void setBit(uint8_t note);
    void clearBit(uint8_t note);
    void clearAll();

    bool isBitSet(uint8_t note);

    uint8_t getNumBits();
    uint8_t getLowest();

  private:

    uint8_t keys[16];
    uint8_t numKeys;
};

class notetracker
{
  public:
    notetracker();
  
    enum tracker_mode {NORMAL, ARP_UP, ARP_DN};

    void                  noteOn(uint8_t key);
    void                  noteOff(uint8_t key);
    
    void                  setMode(notetracker::tracker_mode m);
    notetracker::tracker_mode getMode();

    void                  setShort(bool short_on);
    bool                  getShort();

    //uint8_t               getLowest();
    uint8_t               getNext(uint8_t start);

    void                  setSustain(bool);

    void                  tickArp(bool);

    uint8_t               whichKey();

    uint8_t               getLastKey();

    bool                  getGate();
    
  private:

    tracker_mode mode;
    
    bool     staccato;
    bool     sustaining;
    bool     clk_hi;

    notemap voice_map;
    notemap sustain_map;
    notemap * active_map_p;
    
    uint8_t last_key;
};


