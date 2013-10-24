#include"BootSector.h"

#include<Windows.h>

#include<string>

using namespace std;

#define FILE_NAME_ID 48
#define COUNT_OFFSET_OFFSET 10


class logicDisk
{
#define ROOT_DIC_OFFSET 5
#define SECTOR_PER_FR 2

public:
    string name;

    bootSector bs;

    UINT64 rootDIC;
};

class FRInfo
{


#define ROOT_OFFSET 5

public:

    BYTE FR[1024];
    

    string name;

    WORD dataOffset;
    bool isDirect;

    
};

class FileInfo
{
public:
    string logicDriveName;
    string filePath;


    FileInfo(string _filePath);
    
    // false return < 0
    // succeed return 0
    INT32 getSectorData(DWORD sectorNumber, BYTE *dest);

    // succeed return = 0
    // failed return < 0
    INT32 getFileInfo();


    INT32 process();
};
