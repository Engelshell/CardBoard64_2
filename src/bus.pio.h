// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// --------------- //
// bus_pio_address //
// --------------- //

#define bus_pio_address_wrap_target 0
#define bus_pio_address_wrap 10

static const uint16_t bus_pio_address_program_instructions[] = {
            //     .wrap_target
    0x80a0, //  0: pull   block                      
    0xbb42, //  1: nop                    side 2 [3] 
    0xa342, //  2: nop                           [3] 
    0xbd42, //  3: nop                    side 3 [1] 
    0x6310, //  4: out    pins, 16               [3] 
    0xa142, //  5: nop                           [1] 
    0xb542, //  6: nop                    side 1 [1] 
    0x6310, //  7: out    pins, 16               [3] 
    0xa142, //  8: nop                           [1] 
    0xb042, //  9: nop                    side 0     
    0xa003, // 10: mov    pins, null                 
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program bus_pio_address_program = {
    .instructions = bus_pio_address_program_instructions,
    .length = 11,
    .origin = -1,
};

static inline pio_sm_config bus_pio_address_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + bus_pio_address_wrap_target, offset + bus_pio_address_wrap);
    sm_config_set_sideset(&c, 3, true, false);
    return c;
}
#endif

// ------------ //
// bus_pio_read //
// ------------ //

#define bus_pio_read_wrap_target 6
#define bus_pio_read_wrap 11

static const uint16_t bus_pio_read_program_instructions[] = {
    0xa742, //  0: nop                           [7] 
    0xa742, //  1: nop                           [7] 
    0xa742, //  2: nop                           [7] 
    0xa742, //  3: nop                           [7] 
    0xa742, //  4: nop                           [7] 
    0xa742, //  5: nop                           [7] 
            //     .wrap_target
    0xb742, //  6: nop                    side 0 [7] 
    0xa442, //  7: nop                           [4] 
    0xb842, //  8: nop                    side 1     
    0x4010, //  9: in     pins, 16                   
    0x8020, // 10: push   block                      
    0xa142, // 11: nop                           [1] 
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program bus_pio_read_program = {
    .instructions = bus_pio_read_program_instructions,
    .length = 12,
    .origin = -1,
};

static inline pio_sm_config bus_pio_read_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + bus_pio_read_wrap_target, offset + bus_pio_read_wrap);
    sm_config_set_sideset(&c, 2, true, false);
    return c;
}
#endif
