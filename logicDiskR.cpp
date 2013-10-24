#include"logicDiskR.h"

FileInfo::FileInfo(string _filePath)
{
    logicDriveName = _filePath[0];
    filePath = _filePath;
    bytePerSector = BYTES_PER_SECTOR;
}



INT32 FileInfo::getFileInfo()
{
    BYTE *buffer = new BYTE[bytePerSector];
    INT32 res = 0;
    res = getSectorData(0, buffer);
    if(res < 0)
    {
        return res;
    }

    void *off = (void *)(buffer + 11);
    bytePerSector = *(WORD*)(off);
    
    off = (void *)(buffer + 13);
    sectorPerCluster = *(BYTE*)(off);

    off = (void*)(buffer + 48);
    startOfMFT = (UINT64)(*(DWORD*)(off) * sectorPerCluster);

    // fix me: assume that size of every FR is 2 sector
    startOfBoot = startOfMFT + ROOT_OFFSET * 2;


    delete[] buffer;
    return 0;
}

INT32 FileInfo::process()
{
    INT32 res = 0;
    res = getFileInfo();
    if(res < 0)
    {
        return res;
    }
    return 0;
}



INT32 FRInfo::getFRInfo()
{
    attributeOffset = (INT64)(*(WORD *)(FR + ATRRIBUTE_OFFSET_OFFSET));
    WORD flag = *(WORD *)(FR + 22);

    sizeOfFR = *(WORD *)(FR + 24);
    mainFR = *(INT64 *)(FR + 28);
    if(mainFR != 0)
    {
        return -1;
    }

    INT64 poff = attributeOffset;
    DWORD ID = 0;
    DWORD len = 0;
    INT64 countoff = 0;
    while(1)
    {
        if(poff = sizeOfFR - 1)
        {
            break;
        }
        ID = *(DWORD *)(FR + ATRRIBUTE_TYPE_OFFSET);
        len = *(DWORD *)(FR + ATRRIBUTE_LENTH_OFFSET);
        countoff = *(INT64*)(FR + COUNT_OFFSET_OFFSET);

        if(ID == FILE_NAME_ID)
        {

        }
        else
        {
            poff += len;
        }
    }

    return 0;
}
