#pragma once
#include <stdint.h>
#include <string.h>
#include <string>
#include "n64txt.hpp"
#include <math.h>
#include <algorithm>
#include <memory>
#include <functional>
#include <map>
#include "cartbuspio.hpp"
#include "config.h"




class Cart
{
public:
    int32_t setup();
    CartBusPIO cartBusPIO;
    int32_t readRomPage(uint8_t *buffer, int32_t bufsize, uint32_t offset);
    //int32_t readRomPage(uint8_t *buffer, int32_t bufsize, uint32_t page);
    uint32_t cartCRC = 0;
    uint32_t headCRC = 0;

    uint8_t cartSize = 0;
    std::string cartName = "";

    bool isInDatabase();

    // prefer a standard format
    int32_t formatJSON(std::string &bufStr);

    bool CIDExists();
    enum SaveType : uint8_t
    {
        NONE = 0,
        SRAM = 1,
        FRAM1 = 2,
        FRAM2 = 3,
        EEPROM = 4
    };

    enum EepSaveNum : uint32_t
    {
        ENONE = 0,
        EFRAM1 = 64,
        EFRAM2 = 256
    };

    std::map<SaveType, std::string> saveStrMap = {
        {SaveType::NONE, ""},
        {SaveType::SRAM, "SRAM"},
        {SaveType::FRAM1, "FRAM"},
        {SaveType::FRAM2, "FRAM"},
        {SaveType::EEPROM, "EEPROM"}};

    std::map<SaveType, std::string> saveFileNameMap = {
        {SaveType::NONE, ""},
        {SaveType::SRAM, "save.sram"},
        {SaveType::FRAM1, "save.fram"},
        {SaveType::FRAM2, "save.fram"},
        {SaveType::EEPROM, "save.eep"}};

    int32_t getSaveFileName(SaveType type, std::string &filename);

    SaveType getSaveType();
    int32_t setSaveType(SaveType type);
    std::string getSaveTypeStr();
    EepSaveNum getEepSaveNum();
    int32_t setEepSaveNum(EepSaveNum num);
    uint8_t getSaveSize();
 
    static struct
    {
        std::string SRAM = "save.sram";
        std::string FRAM = "save.fram";
        std::string EEPROM = "save.eep";
    } saveTypeFilename;

    uint8_t saveSize = 0;
    SaveType saveType = SaveType::NONE;
    EepSaveNum eepSaveNum = EepSaveNum::ENONE;
    // char _fullName[21] = {0};
    // char _cid[5] = {0};
    uint32_t cidInt = 0;
    uint32_t CRC1 = 0;
    uint32_t CRC2 = 0;
    std::string headerName = "";
    std::string cid = "";
    std::string name = "";
    uint32_t version = 0;
    bool inDatabase = false;
    int32_t createHeaderData();

    int32_t createFromBytes(uint8_t *buffer, int32_t bufsize);
    // pass a buffer of 64 byte size
    void parseInHeaderBytes(uint8_t *buffer, int32_t bufsize);

    // true if match, false is no match
    bool matchCartDatabase();
};