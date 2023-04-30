#include "virtfat16.h"
#include "bsp/board.h"
#include "tusb.h"

// Reserved characters for FAT short 8.3 names.
inline bool StaticFat::sfnReservedChar(uint8_t c) {
  if (c == '"' || c == '|' || c == '[' || c == '\\' || c == ']') {
    return true;
  }
  //  *+,./ or :;<=>?
  if ((0X2A <= c && c <= 0X2F && c != 0X2D) || (0X3A <= c && c <= 0X3F)) {
    return true;
  }
  // Reserved if not in range (0X20, 0X7F).
  return !(0X20 < c && c < 0X7F);
}
// Reserved characters for exFAT names and FAT LFN.
inline bool StaticFat::lfnReservedChar(uint8_t c) {
  return c < 0X20 || c == '"' || c == '*' || c == '/' || c == ':'
    || c == '<' || c == '>' || c == '?' || c == '\\'|| c == '|';
}

uint8_t StaticFat::lfnChecksum(uint8_t* name) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < 11; i++) {
        sum = (((sum & 1) << 7) | (sum >> 1)) + name[i];
    }
    return sum;
}



uint32_t StaticFat::readStaticFatDrive(uint32_t chunk, uint32_t offset, uint8_t * buffer, uint32_t bufferSize) {
    uint32_t addr = (chunk * 0x200) + offset;
    uint32_t offset1 = 0;
    uint8_t * tPtr = nullptr;

    for(File file : StaticFat::filesVector) {
        if(addr >= file.startAddr && addr < file.endAddr) {
            return file.cBackPtr(chunk, offset, buffer, bufferSize, &file);
        }
    }
    if(addr >= mbrStartAddr && addr < mbrEndAddr) {
        tPtr = (uint8_t *)&mbrBlock;
        memcpy(buffer, tPtr, bufferSize);
        return bufferSize;
    }
    else if(addr >= fatBStartAddr && addr < fatBEndAddr) {
        tPtr = (uint8_t *)&fatBlock;
        memcpy(buffer, tPtr, bufferSize);
        return bufferSize;
    }
    else if(addr >= table1StartAddr && addr < table1EndAddr) {
        offset1 = addr - table1StartAddr;
        tPtr = (uint8_t*)&fatTableBlock;
        memcpy(buffer, tPtr+offset1, bufferSize);
        return bufferSize;
    }
    else if(addr >= table2StartAddr && addr < table2EndAddr) {
        offset1 = addr - table2StartAddr;
        tPtr = (uint8_t*)&fatTableBlock;
        memcpy(buffer, tPtr+offset1, bufferSize);
        return bufferSize;
    }
    else if(addr >= rootDirStartAddr && addr < rootDirEndAddr) {
        offset1 = addr - rootDirStartAddr;
        tPtr = (uint8_t*)&rootDirBlock;
        memcpy(buffer, tPtr+offset1, bufferSize);
        return bufferSize;
    }
    
    memset(buffer, 0, bufferSize);
    //return 0's if no match
    return bufferSize;
}



//max name size 255
//max sectors below 2GB
uint32_t StaticFat::addFileCallback(const char * name, uint32_t size,
int32_t (*cBackPtr) (uint32_t, uint32_t, uint8_t*, uint32_t, File*)) {
 
    uint8_t nameLen = strlen(name);
    char shortName[12] = {0};
    bool needLdir = makeShortName(name, shortName);

    uint8_t chksum = lfnChecksum((uint8_t *)shortName);
    //only do long dirs if name is more than 8 main characters excluding extension
    if(needLdir) {
        addLongDir(name, chksum);
    }
    uint16_t clusterPos = addToTable(size);
    addShortDir(shortName, ATTR::ATTR_ARCHIVE, clusterPos, size);
    
    uint32_t RootDirSectors = ((fatBlock.BPB_RootEntCnt * 32) + (fatBlock.BPB_BytsPerSec - 1)) / fatBlock.BPB_BytsPerSec;
    uint32_t FirstDataSector = fatBlock.BPB_ResvdSecCnt + (fatBlock.BPB_NumFATs * fatBlock.BPB_FATSz16) + RootDirSectors;
    uint32_t FirstSectorofCluster = ((clusterPos - 2) * fatBlock.BPB_SecPerClus) + FirstDataSector;
    uint32_t firstBytes = fatBStartAddr + (FirstSectorofCluster * 0x200);
    uint32_t sizeBytes = size;

    File file;
    file.cBackPtr = cBackPtr;
    file.startAddr = firstBytes;
    file.endAddr = firstBytes + sizeBytes;
    file.fileSize = size;
    filesVector.push_back(file);
    
    return true;
}


//outShortName 11 chars]
//return true if long directory needed, false if not
bool StaticFat::makeShortName(const char * name, char * outShortName) {
    uint8_t nameLen = strlen(name);
    uint32_t nameShortLen = 11 < nameLen ? 11 : nameLen;
    char shortName[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',0};

    char * ext = strrchr(name, '.');
    int8_t z = ext != NULL ? ext - name: nameLen;
    uint8_t x1 = 0;
    for(uint8_t i = 0; i < z && x1 <= 9; i++) {
        char c = toupper(name[i]);
        bool isR = sfnReservedChar((uint8_t)c);
        if(!isR) {
            //if more than 8 characters replace with ~1
            if(x1+1 == 9) {
                shortName[6] = '~';
                shortName[7] = '1';
                x1++;
                break;
            }
            shortName[x1++] = c;
        }
    }
    
    if(ext != NULL) {
        uint8_t extLen = strlen(ext);
        for(uint8_t i = 0, x = 8; i < extLen && x < 11; i++) {
            char c = toupper(ext[i]);
            bool isR = sfnReservedChar((uint8_t)c);
            if(!isR) {
                shortName[x++] = c;
            }
        }
    }
    strncpy(outShortName, shortName, 12);

    if(x1 > 8) {
        return true;
    } 
    return false;
}


void StaticFat::addShortDir(const char * name, ATTR attribute, uint16_t cluster, uint32_t size) {
    uint32_t nameSize = 11 < strlen(name) ? 11 : strlen(name);

    DIR dir1;
    for(uint8_t i = 0; i < nameSize; i++) {
        dir1.DIR_Name[i] = name[i];
    }
    dir1.DIR_Attr = attribute; //Archive
    dir1.DIR_FstClusLO = cluster;
    dir1.DIR_FileSize = size;
    dir1.DIR_CrtDate[0] = 0xED;
    dir1.DIR_CrtDate[1] = 0x54;
    dir1.DIR_CrtTime[0] = 0xF8;
    dir1.DIR_CrtTime[1] = 0x0E;
    dir1.DIR_CrtTimeTenth = 0x67;
    dir1.DIR_LstAccDate[0] = 0xED;
    dir1.DIR_LstAccDate[1] = 0x54;
    dir1.DIR_WrtDate[0] = 0xED;
    dir1.DIR_WrtDate[1] = 0x54;
    dir1.DIR_WrtTime[0] = 0xF8;
    dir1.DIR_WrtTime[1] = 0x0E;
    uint8_t * bPtr = (uint8_t *)&rootDirBlock;
    uint8_t * ptr = (uint8_t *)&dir1;
    memcpy(bPtr+rootDirPos, ptr, 32);
    rootDirPos+=32;
};

void StaticFat::addLongDir(const char * name, uint8_t chksum) {
    //NOTE: wchar_t on pico for some reason takes 4 bytes with memcpy
    uint8_t nameSize = strlen(name)+1;
    //uint16_t nameSize = std::char_traits<char16_t>::length(name);
    uint8_t  nameOrd = (nameSize + 12) / 13;

    uint16_t wName[nameSize] = {0};
    for(uint8_t i = 0; i < nameSize; i++) {
        wName[i] = (uint16_t)name[i];
    }

    //255/13 = 19
    std::list<LDIR> dirList;
    uint16_t c = 0;

    for (uint8_t order = 1; order <= nameOrd; order++) {
        LDIR ldir;
        ldir.LDIR_Ord = order == nameOrd ? (uint8_t)0x40 | order : order;
        ldir.LDIR_Chksum = chksum; //TODO

        uint16_t cRem = nameSize - c;
        //count of 16 bit values
        uint16_t n1 = 5, n2 = 6, n3 = 2;
        //calculate remainders
        if(cRem < 13) {
            if(cRem - 11 > 0) {
                n3 = cRem - 11;
            } else {
                n3 = 0;
            }
            if(cRem < 11) {
                if(cRem - 5 > 0) {
                    n2 = cRem - 5;
                } else {
                    n2 = 0;
                }
                if(cRem < 5) {
                    n1 = cRem;
                }
            }
        }
        for(uint8_t i = 0; i < n1; i++, c++) {
            ldir.LDIR_Name1[i] = wName[c];
        }
        for(uint8_t i = 0; i < n2; i++, c++) {
            ldir.LDIR_Name2[i] = wName[c];
        }
        for(uint8_t i = 0; i < n3; i++, c++) {
            ldir.LDIR_Name3[i] = wName[c];
        }

        dirList.push_front(ldir);
    }

    for(LDIR ldir : dirList) {
        uint8_t * bPtr = (uint8_t *)&rootDirBlock;
        uint8_t * ptr = (uint8_t *)&ldir;
        memcpy(bPtr+rootDirPos, ptr, 32);
        rootDirPos+=32;
    }
}

uint16_t StaticFat::addToTable(uint32_t bytes) {
    uint32_t sectors = ((bytes +(0x200-1)) / 0x200);
    uint16_t numClusters = ((sectors + (fatBlock.BPB_SecPerClus - 1)) / fatBlock.BPB_SecPerClus);
    uint16_t s = fatTableBlockPos;
    uint16_t h = s + numClusters;
    for(;fatTableBlockPos < h;) {
        if(!(fatTableBlockPos+1 < h)) {
            fatTableBlock[fatTableBlockPos] = 0xFFFF;
        } else {
            fatTableBlock[fatTableBlockPos] = fatTableBlockPos+1;
        }
        fatTableBlockPos++;
    }
    //return index of first cluster added
    return s;
}

