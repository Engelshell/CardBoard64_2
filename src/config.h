#pragma once
#include <stdint.h>
#include "pico/stdio.h"
#include "hardware/pio.h"


namespace MyGlobals {

    const uint32_t BusPageSize = 0x200;
    
    //Address to start searching for data on cart
    const uint32_t BusBaseAddress = 0x10000000;

    const uint32_t SramBaseAddress = 0x08000000;
    /* Global layout of pins
    * This is not completely exclusive, some PIO programs
    * required having hardcoded pin numbers, if changing MYGPIO
    * ensure you understand how that affects the PIO programs
    */
    const static struct {
        //Start GPIO for the 16 pin communication with cart
        //If BUSGPIO 2, then we use pin 2 through 18
        const uint BUSGPIO  = 2;
        const uint ALEL     = 18; //ALE Low
        const uint ALEH     = 19; //ALE High
        const uint WR       = 20; //Write
        const uint EEPCLOCK = 21; //EEPROM Clock
        const uint RESET    = 22; //Cart Reset
        const uint RD       = 26; //Read
        const uint EEPDATA  = 27; //EEPROM Data
    } MYGPIO;

    const static struct {
        PIO EEPROM = pio1;
        PIO BUS = pio0;
    } MYPIO;
    

    //State Machine number for EEPROM or BUS
    const static struct {
        const uint EEPROM = 0;
        const uint BUSADDRESS = 0;
        const uint BUSREAD = 1;
    } MYSM;


}