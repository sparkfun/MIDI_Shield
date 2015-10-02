#pragma once

#include <Arduino.h>
#include <stdint.h>

class notemap
{
  public:
    notemap();
  
    enum map_mode {NORMAL, ARP_UP, ARP_DN};
    void setMode(notemap::map_mode m);
    notemap::map_mode getMode();

    void setShort(bool short_on);
    bool getShort();

    void debug();
    void setKey(uint8_t note);
    void clearKey(uint8_t note);
    bool isBitSet(uint8_t note);
    uint8_t getLowest();
    uint8_t getNext(uint8_t start);

    void tickArp(bool);

    uint8_t whichKey();

    uint8_t getNumKeysHeld();
    uint8_t getLastKey();

    bool    getGate();
    
  private:

    map_mode mode;
    bool     staccato;
    bool     clk_on;

    uint8_t voice_map[16];
    uint8_t num_keys_held;
    uint8_t last_key;
};


