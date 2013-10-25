#include"BootSector.h"
#include"FR.h"

#include<string>
#include<iostream>

using namespace std;

#include<stdio.h>

#define MAX_FILE_COUNT 100000
#define MAX_OPEN_COUNT 1

INT32 openDisk(string logicDriverName, HANDLE &hDisk)
{
    string add = "\\\\.\\" + logicDriverName + ":";
    UINT32 index = 0;
    for(;index < MAX_OPEN_COUNT; index ++)
    {
        hDisk = CreateFile(add.c_str(), GENERIC_READ, FILE_SHARE_READ,\
        NULL, OPEN_EXISTING, NULL, NULL);
        if(hDisk != INVALID_HANDLE_VALUE)
        {
            break;
        }
        //cerr<<GetLastError()<<endl;
    }
    if(index == MAX_OPEN_COUNT)
    {
        fprintf(stderr, "\nopen disk filed in openDisk(), have opened %lld times\nError ID: %lu", (INT64)MAX_OPEN_COUNT, GetLastError());
            //cerr<<"\nopen disk filed in openDisk()";
            return -1;
    }
    return 0;
}

INT32 getData(HANDLE hDisk, UINT64 offset, BYTE *dest, DWORD lenth)
{
    //UINT64 offset = BYTES_PER_SECTOR * sectorNumber;
    void* p = (void*)&offset;

    SetFilePointer(hDisk, *(LONG*)p, (PLONG)((BYTE*)p + 4), FILE_BEGIN);
    DWORD dwReadLenth = 0;
    INT32 res = 0;
    res = ReadFile(hDisk, dest, lenth, &dwReadLenth, NULL);
    if(res == 0)
    {
        fprintf(stderr, "\nread data failed in getData(), read %lu/%lu byte(s)", dwReadLenth, lenth);
        //cerr<<"\nread data failed in getData(), read "<<lenth<<"//"<<dwReadLenth<<" byte(s)";
        return -2;
    }
    return 0;
}

INT32 searchFile(HANDLE hDisk, string filename, INT64 &offset)
{
    offset = 0;
    BYTE data[BYTES_PER_SECTOR * SECTORS_PER_FR];
    INT32 res = 0;
    res = getData(hDisk, 0, data, BYTES_PER_SECTOR);
    if(res != 0)
    {
        fprintf(stderr, "\nget boot sector data filed in searchFile()");
        //cerr<<"\nget data filed in searchFile()";
        return res;
    }
    bootSector bs(data);
    INT64 fileCount = 0;
    string sfsID((char*)bs.fileSystemID, 4);
    if(sfsID != "NTFS")
    {
        fprintf(stderr, "\nthis disk is not ntfs formate");
        //cerr<<"\nthis disk is not ntfs formate";
        return -1;
    }
    FR* fr = NULL;
    INT64 offsetOfSector = bs.startOfMFT * 8;
    while(fileCount < MAX_FILE_COUNT)
    {
        fileCount ++;
        res = getData(hDisk, offsetOfSector * BYTES_PER_SECTOR, data, BYTES_PER_SECTOR * SECTORS_PER_FR);
        if(res !=0 )
        {
            fprintf(stderr, "\nget FR data filed in searchFile()");
            return res;
        }
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
                    string::size_type index = 0;
                    for(; index < fnameUNICODE.length(); index ++)
                    {
                        if(index %2 == 0)
                        {
                            fnameASCII.push_back(fnameUNICODE[index]);
                        }
                    }
                    fprintf(stdout, "\nfound file %s", fnameASCII.c_str());
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
        fprintf(stderr, "\nfile not found have scan %lld file", (UINT64)MAX_FILE_COUNT);
        //cerr<<"\nfile not found have scan #MAX_FILE_COUNT file";
        return -2;
    }
    offset = offsetOfSector;
    SAFE_RELEASE_SINGLE_POINTER(fr);
    return 0;
}

INT32 main()
{
    char logicDriverName[100];
    char name[1000];
    fprintf(stdout, "输入文件所在盘符（例如：C）：");
    fscanf_s(stdin, "%s", &logicDriverName, 2);
    fprintf(stdout, "输入文件名（只支持英文）（例如：small.txt）：");
    fscanf_s(stdin, "%s", name, sizeof(name));
    
    INT32 res = 0;
    INT64 sectorNum = 0;

    HANDLE hDisk = NULL;
    res = openDisk(string(logicDriverName), hDisk);
    if(res != 0)
    {
        fprintf(stderr, "\nopen disk failed");
        std::system("pause");
        return res;
    }

    res = searchFile(hDisk, string(name), sectorNum);
    if(res == 0)
    {
        BYTE data[BYTES_PER_SECTOR * SECTORS_PER_FR];
        printf("\nfile record found in sector %lld", sectorNum);
        
        getData(hDisk, sectorNum * BYTES_PER_SECTOR, data, BYTES_PER_SECTOR * SECTORS_PER_FR);
        FR fr(data);
        if(fr.aData->header->isResident)
        {
            fprintf(stdout, "\n$FILE_NAME attribute in this file isResident\nso the data of this file is in sector %lld", sectorNum);
        }
        else
        {
            fprintf(stdout,"\ncluster(s):");
            vector<clasterFragments>* pcf = &fr.aData->header->run->cf;
            vector<clasterFragments>::iterator it = pcf->begin();
            for(; it != pcf->end(); it ++)
            {
                fprintf(stdout, "\nfragment %lld: begin in cluster %llu, lenth %llu", (INT64)(it - pcf->begin()), it->begin, it->lenth);
            }
        }
    }
    else
    {
        printf("\nfile not found");
    }
    printf("\n");
    std::system("pause");

    return 0;
}