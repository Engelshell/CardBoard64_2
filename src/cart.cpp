#include "cart.h"


int32_t Cart::setup()
{
    int32_t ret = 1;
    cartBusPIO.setup();

    ret = this->Cart::createHeaderData();
    if (ret != 1)
        return ret;


    return ret;
}

int32_t Cart::readRomPage(uint8_t *buffer, int32_t bufsize, uint32_t offset)
{
    const uint32_t address = MyGlobals::BusBaseAddress + offset;
    //TODO: check offsets
    uint32_t bufferOffset = 0;
    uint32_t r = 0;

    r = cartBusPIO.readBusPage(address, MyGlobals::BusPageSize, buffer, bufferOffset, bufsize);
    return r;
}


bool Cart::isInDatabase()
{
    return this->inDatabase;
}

// prefer a standard format
int32_t Cart::formatJSON(std::string &bufStr)
{

    int32_t ret = 0;
    uint32_t printSize = 0x200;

    const char * jformat1 = 
      R"({)" "\n"
      R"(  "header": {)" "\n"
      R"(    "name": "%s",)" "\n"
      R"(    "cid":    "%s",)" "\n"
      R"(    "CRC1":   "%x",)" "\n"
      R"(    "CRC2":   "%x",)" "\n"
      R"(    "version": %d)" "\n"
      R"(  },)" "\n"
      R"(  "database": {)" "\n"
      R"(    "fullname": "%s",)" "\n"
      R"(    "savetype": "%s",)" "\n"
      R"(    "gamecrc":  "%x",)" "\n"
      R"(    "headcrc":  "%x)" "\n"
      R"(  })" "\n"
      R"(})" "\n"
      ;


    
    char buffer[printSize];
    ret = snprintf(buffer, printSize, jformat1, name.c_str(), cid.c_str(), CRC1,
                   CRC2, cartSize, version, name.c_str(), this->getSaveTypeStr().c_str(), cartCRC, headCRC);
    bufStr.assign(buffer, printSize);
    return ret;
};

bool Cart::CIDExists()
{
    return this->cidInt != 0;
}

int32_t Cart::getSaveFileName(SaveType type, std::string &filename)
{
    filename.assign(saveFileNameMap.at(type));
    return 1;
}

Cart::SaveType Cart::getSaveType()
{
    return this->saveType;
}
int32_t Cart::setSaveType(SaveType type)
{
    this->saveType = type;
    return 1;
}
std::string Cart::getSaveTypeStr()
{
    return saveStrMap.at(this->saveType);
}

uint8_t Cart::getSaveSize()
{
    return this->saveSize;
}

int32_t Cart::createHeaderData()
{
    int32_t ret = 1;
    uint8_t buffer[0x200];
    ret = this->readRomPage(buffer, 0x200, 0);
    if (ret < 1)
    {
        return ret;
    }

    ret = this->createFromBytes(buffer, 0x200);
    if (ret != 1)
    {
        return ret;
    }

    return ret;
}

int32_t Cart::createFromBytes(uint8_t *buffer, int32_t bufsize)
{
    if (bufsize < 64)
    {
        return -1;
    }
    parseInHeaderBytes(buffer, bufsize);
    this->inDatabase = matchCartDatabase();
    return 1;
}
// pass a buffer of 64 byte size
void Cart::parseInHeaderBytes(uint8_t *buffer, int32_t bufsize)
{
    // need data as char type
    char *cbuff = reinterpret_cast<char *>(buffer);
    // extract 20 chars starting at index 0x20
    this->name = std::string(cbuff + 0x20, 20);
    // extract 4 chars starting at index 0x3B
    this->cid = std::string(cbuff + 0x3B, 4);
    // convert 4 chars to uint32_t value
    this->cidInt = (this->cid.at(0) << 24) | (this->cid.at(1) << 16) | (this->cid.at(2) << 8) | (this->cid.at(3));
    // conver 4 chars to uin32_t value
    this->CRC1 = (buffer[0x10] << 24) | (buffer[0x11] << 16) | (buffer[0x12] << 8) | (buffer[0x13]);
    // conver 4 chars to uin32_t value
    this->CRC2 = (buffer[0x14] << 24) | (buffer[0x15] << 16) | (buffer[0x16] << 8) | (buffer[0x17]);
    this->version = buffer[0x3F];

    return;
};

// true if match, false is no match
bool Cart::matchCartDatabase()
{
    for (auto &rows : headerEntries)
    {
        if (this->CRC1 == rows.headcrc)
        {
            this->cartCRC = rows.gamecrc;
            this->headCRC = rows.headcrc;
            SaveType stype = SaveType(rows.savetype);
            this->setSaveType(stype);
            this->cartSize = rows.size;
            this->name = rows.name;

            /*if (stype == SaveType::FRAM1)
            {
                this->setEepSaveNum(EepSaveNum::EFRAM1);
            }
            if (stype == SaveType::FRAM2)
            {
                this->setEepSaveNum(EepSaveNum::EFRAM2);
            }*/

            return true;
        }
    }

    return false;
}