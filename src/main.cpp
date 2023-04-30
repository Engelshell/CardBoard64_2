#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "config.h"
#include "virtfat16.h"
#include "usbdisk.h"
#include "bsp/board.h"
#include "tusb.h"
#include "cart.h"

uint64_t runOneShotTime = 0;
bool runOneShot = true;
uint32_t store = 0;

Cart cart;
std::string fileErrorStr = "";
std::string ccartName = "";
bool isClkSet = false;
char headerJSON[512] = {0};

static int32_t cartDataCallback(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufferSize, StaticFat::File *file)
{
    int32_t r = 0;
    const uint32_t offx = ((lba * 0x200) - file->startAddr) + offset;
    r = cart.readRomPage(buffer, bufferSize, offx);
    return bufferSize;
}

static int32_t headerFileCallback(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufferSize, StaticFat::File *file)
{
    const int32_t r = snprintf((char *)buffer, strlen(headerJSON), "%s", headerJSON);
    //memcpy(buffer, headerJSON, bufferSize);
    return bufferSize;
}

static int32_t errorFileCallback(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufferSize, StaticFat::File *file)
{
    const int32_t r = snprintf((char *)buffer, fileErrorStr.size(), "%s", fileErrorStr.c_str());
    return bufferSize;
}

bool errorHandle() {
    if(!isClkSet) {
        fileErrorStr = "Could not set clock speed\n";
    }
    if(!fileErrorStr.empty()) {
        staticf.addFileCallback("error.txt", fileErrorStr.size(), errorFileCallback);
        return true;
    }
    return false;
}

void runCart()
{  
    int32_t r = 1;
    if(errorHandle()) {
        return;
    }
    
    staticf.addFileCallback("header.txt", 512, headerFileCallback);
    r = cart.setup();
    std::string buf1 = "";
    cart.formatJSON(buf1);
    snprintf(headerJSON, buf1.size(), "%s", buf1.c_str());

    const uint32_t s = cart.cartSize * 1024 * 1024;
    std::string n = std::string(cart.name);
    staticf.addFileCallback(n.c_str(), s, cartDataCallback);


    //tud_cdc_write_str(buf1.c_str());
    //tud_cdc_write_flush(); 
}

int main()
{
    
    isClkSet = set_sys_clock_khz(250000, false);

    board_init();
    tud_init(BOARD_TUD_RHPORT);
    runOneShotTime = time_us_64();
    uint64_t looper = time_us_64();
    while (1)
    {
        tud_task();
        led_blinking_task();
        cdc_task();
        if (runOneShot)
        {
            // wait 100ms
            if (time_us_64() - runOneShotTime > 100000)
            {
                runOneShot = false;
                runCart();
            }
        }
    }

    return 0;
}
