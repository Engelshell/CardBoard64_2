#pragma once
//#include <Arduino.h>
#include <stddef.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <cstdint>
#include "hardware/clocks.h"
#include "config.h"
//#include "pioutils.hpp"
#include "bus.pio.h"
#include "bsp/board.h"

using namespace MyGlobals;

class CartBusPIO {
public:
PIO _pio = MYPIO.BUS;
const uint _smAddr = MYSM.BUSADDRESS;
const uint _smRead = MYSM.BUSREAD;
uint _offAddr;
uint _offRead;
pio_sm_config _confAddr;
pio_sm_config _confRead;



int set_bit(int num, int position)
{
	int mask = 1 << position;
	return num | mask;
}

void pu_pio_gpio_init_cons(PIO p, uint8_t start, uint8_t length) {
    for(int i = start ; i < length+start; i++) {
        pio_gpio_init(p, i);
    }
}


void pu_pio_set_pindir(PIO p, uint sm, uint8_t gpio, bool highlow) {
    pio_sm_set_consecutive_pindirs(p, sm, gpio, 1, highlow);
}



void pu_set_pins_cons(PIO p, uint sm, uint8_t gpio, uint8_t length, bool highlow) {
    uint32_t pinValues = 0;
    uint32_t pinMask = 0;
    //put bit value of 1 starting from gpio to length
    for(uint8_t i = gpio; i < gpio+length; i++) {
        pinMask |= 1 << i;
    }
    for(uint8_t i = gpio; i < gpio+length; i++) {
        pinValues |= highlow << i;
    }
    pio_sm_set_pins_with_mask(p, sm, pinValues, pinMask);
    //Serial.print("pinValues: ");Serial.println(pinValues, BIN);
    //Serial.print("pinMask");    Serial.println(pinMask, BIN);
}




void setupAddress() {
    //initialize 16 pin bus for PIO
    pu_pio_gpio_init_cons        (_pio, MYGPIO.BUSGPIO, 16);
    //set bus to output
    pio_sm_set_consecutive_pindirs(_pio, _smAddr, MYGPIO.BUSGPIO, 16, true);
    //set bus level to 0
    pu_set_pins_cons             (_pio, _smAddr, MYGPIO.BUSGPIO, 16, 0);
    pio_gpio_init                (_pio, MYGPIO.ALEL);
    pio_gpio_init                (_pio, MYGPIO.ALEH);
    pio_gpio_init                (_pio, MYGPIO.RD  );
    //MYGPIO.RD, MYGPIO.ALEL, MYGPIO.ALEH output, MYGPIO.RD, 
    //MYGPIO.ALEH default high, MYGPIO.ALEL default low
    pio_sm_set_pindirs_with_mask (_pio, _smAddr, 1u << MYGPIO.RD,   1u << MYGPIO.RD  );
    pio_sm_set_pindirs_with_mask (_pio, _smAddr, 1u << MYGPIO.ALEL, 1u << MYGPIO.ALEL);
    pio_sm_set_pindirs_with_mask (_pio, _smAddr, 1u << MYGPIO.ALEH, 1u << MYGPIO.ALEH);
    pio_sm_set_pins_with_mask    (_pio, _smAddr, 1u << MYGPIO.RD,   1u << MYGPIO.RD  );
    pio_sm_set_pins_with_mask    (_pio, _smAddr, 0u << MYGPIO.ALEL, 1u << MYGPIO.ALEL);
    pio_sm_set_pins_with_mask    (_pio, _smAddr, 1u << MYGPIO.ALEH, 1u << MYGPIO.ALEH);
    //Sideset works with consecutive pins. MYGPIO.ALEH is after MYGPIO.ALEL,
    //so we specify MYGPIO.ALEL and the .pio program specifies 2 sideset pins
    //therefore we can toggle MYGPIO.ALEL and MYGPIO.ALEH using 0b00 sideset in pio
    sm_config_set_sideset_pins   (&_confAddr, MYGPIO.ALEL);
    //NOTE - set_set_pins only allows for 5 pins
    //Pin directions over 5 must be done with pio_sm_set_consecutive_pindirs
    sm_config_set_out_pins       (&_confAddr, MYGPIO.BUSGPIO,  16);
    sm_config_set_out_shift      (&_confAddr, false, false,  32);
    //62.5Mhz
    sm_config_set_clkdiv         (&_confAddr, 4);
    pio_sm_init                  (_pio, _smAddr, _offAddr, &_confAddr);
    //set enabled by default, the pull instruction in the program
    //will prevent it from doing anything until we send data
    pio_sm_set_enabled           (_pio,  _smAddr, true);
}

//must be called after setupAddress
void setupRead() {
    //uses only MYGPIO.RD as sideset
    sm_config_set_sideset_pins   (&_confRead, MYGPIO.RD);
    //16 pin input
    //we start on pin specified by BUSGPIO, so if 2 we read 2 through 18 for the 16 bits
    sm_config_set_in_pins(&_confRead, MYGPIO.BUSGPIO);
    sm_config_set_in_shift(&_confRead, true, false, 8);
    //62.5Mhz
    sm_config_set_clkdiv(&_confRead, 4);
    //Don't enable yet
    //pio_sm_init(_pio, smRead, offsetRead, &cRead);
    //pio_sm_set_enabled(_pio, smRead, true);
}


int32_t setup() {
    //check to ensure we can load program
    //pioasm will allow you to compile programs with more than 32 instructions
    //this will fault if we don't check first
    if(!pio_can_add_program(_pio, &bus_pio_address_program)) { return -1; }
    if(pio_sm_is_claimed   (_pio, _smAddr)) { return -2; }

    if(!pio_can_add_program(_pio, &bus_pio_read_program)) { return -1; }
    if(pio_sm_is_claimed(_pio, _smRead)) { return -2; }

    pio_sm_claim(_pio, _smAddr);
    pio_sm_claim(_pio, _smRead);

    _offAddr = pio_add_program(_pio, &bus_pio_address_program);
    //get default config from the generated bus.h
    _confAddr = bus_pio_address_program_get_default_config(_offAddr);

    _offRead = pio_add_program(_pio, &bus_pio_read_program);
    //get default config from the generated bus.h
    _confRead = bus_pio_read_program_get_default_config(_offRead);

    this->setupAddress();
    this->setupRead();
    return 1;
}



int32_t readSram() {
  uint32_t pageAddress = 0x08000000;
  uint32_t cartSize = 32 * 1024 * 1024; //bytes
  uint32_t endPageAddress = pageAddress + cartSize;
  uint32_t pageSize = 512; //bytes
  uint32_t bufferOffset = 0;
  uint32_t r = 0;
  uint8_t buffer[pageSize] = {0};
  r = readBusPage(pageAddress, pageSize, buffer, bufferOffset, sizeof(buffer));
  
  return r;
}


int32_t readCart() {
  uint32_t pageAddress = 0x10000000;
  uint32_t cartSize = 32 * 1024 * 1024; //bytes
  uint32_t endPageAddress = pageAddress + cartSize;
  uint32_t pageSize = 512; //bytes
  uint32_t bufferOffset = 0;
  uint32_t r = 0;
  uint8_t buffer[pageSize] = {0};
  r = readBusPage(pageAddress, pageSize, buffer, bufferOffset, sizeof(buffer));
  
  return r;
}


int32_t readBusPage(uint32_t pageAddress, uint32_t pageSize, uint8_t * buffer, uint32_t bufferOffset, uint32_t bufferSize) {
  buffer = buffer+bufferOffset;
  uint32_t ret = 0;
  pio_sm_clear_fifos(_pio, _smAddr);
  pio_sm_clear_fifos(_pio, _smRead);
  pio_sm_init(_pio, _smRead, _offRead, &_confRead);
  pio_sm_set_consecutive_pindirs(_pio, _smAddr, 2, 16, true);
  pio_sm_put_blocking(_pio, _smAddr, pageAddress);
  //set to input, may be a race condition if blocking doesn't work
  pio_sm_set_consecutive_pindirs(_pio, _smRead, 2, 16, false);
  io_rw_16 *rxfifo_shift16 = (io_rw_16*)&_pio->rxf[_smRead]+1;
  clock_gpio_init(MYGPIO.EEPCLOCK, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_XOSC_CLKSRC, 6);
  //TODO: Find a better way to trigger reading after set address
  pio_sm_set_enabled(_pio, _smRead, true);

  bool endLoop = false;
  for(ret = 0; ret < pageSize; ret+=2) {
    //pio_sm_is_rx_fifo_empty();
    //wait until rx fifo is not empty
    const uint64_t waittime = time_us_64();
    while ((_pio->fstat & (1u << (PIO_FSTAT_RXEMPTY_LSB + _smRead))) != 0) { 
      //if it takes longer than a second then assume an error has occured
      if(time_us_64() - waittime > 1000000) {
        endLoop = true;
        break;
      }
      tight_loop_contents(); //does nothing
    }
    if(endLoop) break;
    //NOTE: Fifo apparently borrows some ideas from quantum mechanics. 
    //By observing the fifo, the fifo is changed.
    //Therefore we have to copy once
    uint16_t data16 = (uint16_t)*rxfifo_shift16;
    
    //NOTE: Do not have to bit reverse here, for future reference
    //Cortex M0 does not support RBIT, bit reverse can be done in PIO
    //using MOV ISR :: PINS
    
    buffer[ret] = data16 >> 8;
    buffer[ret+1] = data16 & 0xFF;
  }
  
  //disable read SM so it doesn't continue
  pio_sm_set_enabled(_pio, _smRead, false);
  
  pio_sm_set_pins_with_mask    (_pio, _smAddr, 1u << MYGPIO.RD, 1u << MYGPIO.RD);
  return ret;
}


};