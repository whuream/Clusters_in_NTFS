#include"BootSector.h"
#include"FR.h"

#include<string>
#include<iostream>

using namespace std;

#include<stdio.h>

#define MAX_FILE_COUNT 1000

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

INT32 searchFile(string logicDriverName, string filename, INT64 offset)
{
    offset = 0;
    BYTE data[BYTES_PER_SECTOR * SECTORS_PER_FR];
    INT32 res = 0;
    res = getSectorData(logicDriverName, 0, data);
    if(res != 0)
    {
        return res;
    }
    bootSector bs(data);
    INT64 fileCount = 0;
    string sfsID((char*)bs.fileSystemID, 4);
    if(sfsID != "NTFS")
    {
        return -1;
    }
    FR* fr = NULL;
    INT64 offsetOfSector = bs.startOfMFT * 8;
    while(fileCount < MAX_FILE_COUNT)
    {
        fileCount ++;
        getSectorData(logicDriverName, offsetOfSector, data);
        if(string((char*)data, 4) == "FILE")
        {
            FRHeader FRH(data);
            if(FRH.isExist && !FRH.isDIR)
            {
                fr = new FR(data);
                if(fr->aName != NULL)
                    {
                    string fnameUNICODE((char *)fr->aName->content->fileName, fr->aName->content->nameLenth * 2);
                    string fnameASCII;
                    int index = 0;
                    for(; index < fnameUNICODE.length(); index ++)
                    {
                        if(index %2 == 0)
                        {
                            fnameASCII.push_back(fnameUNICODE[index]);
                        }
                    }
                    fprintf(stdout, "\n%s", fnameASCII.c_str());
                    if(filename == fnameASCII)
                    {
                        break;
                    }
                }
            }
        }
        offsetOfSector += SECTORS_PER_FR;
    }
    if(fileCount == MAX_FILE_COUNT)
    {
        return -2;
    }
    offset = offsetOfSector - SECTORS_PER_FR;
    SAFE_RELEASE_SINGLE_POINTER(fr);
    return 0;
}

int main()
{
    INT64 out = 0;
    char c[100];
    char name[1000];
    fprintf(stdout, "输入文件所在盘符（例如：C）：");
    fscanf(stdin, "%s", &c);
    fprintf(stdout, "输入文件名（只支持英文）（例如：small.txt）：");
    fscanf(stdin, "%s", name);
    INT32 res = 0;
    res = searchFile(string(c), string(name), out);
    if(res == 0)
    {
        printf("\nfile found");
    }
    else
    {
        printf("\nfile not found");
    }
    printf("\n");
    std::system("pause");

    return 0;
}