#include"BootSector.h"
#include"FR.h"

#include<string>

using namespace std;

INT32 getSectorData( string logicDriveName,UINT64 sectorNumber, BYTE *dest)
{
    string add = "\\\\.\\" + logicDriveName + ":";
    HANDLE hDisk = CreateFile(add.c_str(), GENERIC_READ, FILE_SHARE_READ,\
        NULL, OPEN_EXISTING, NULL, NULL);
    if(hDisk == INVALID_HANDLE_VALUE)
    {
        return -1;
    }
    UINT64 offset = BYTES_PER_SECTOR * sectorNumber;
    void* p = (void*)&offset;

    SetFilePointer(hDisk, *(LONG*)p, (PLONG)((BYTE*)p + 4), FILE_BEGIN);
    DWORD dwReadLenth = 0;
    ReadFile(hDisk, dest, BYTES_PER_SECTOR, &dwReadLenth, NULL);
    if(dwReadLenth != BYTES_PER_SECTOR)
    {
        return -2;
    }
    return 0;
}

int main()
{
    BYTE data[BYTES_PER_SECTOR];
    if(!getSectorData("G", 0, data))
    {
        bootSector bs(data);
        bs.sectorsPerCluster++;
    }
    UINT64 num = 6291536;
    if(!getSectorData("G", num, data))
    {
        FRHeader FRH(data);
        FRH.flags ++;
    }

    BYTE data2[1024];
    if(!getSectorData("G", num, data2) && !getSectorData("G", num + 1, data2 + 512))
    {
        FR fr(data);
        fr.FRH->flags ++;
    }
    system("pause");

    return 0;
}