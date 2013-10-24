#include"FR.h"

FRHeader::FRHeader(BYTE* _data)
{
    data = _data;

    offsetToTheSequencceOfAttributesPart = *(WORD *)(data + 0x14);
    flags = *(WORD*)(data + 0x16);
    realSizeOfFR = *(DWORD *)(data + 0x18);
    fileReferenceToTheBaseFileRecord = *(UINT64*)(data + 0x20);

    isDIR = ((flags & 10) != 0) ? true: false;
    isExist = ((flags & 1) != 0) ? true: false;

    isMainFR = fileReferenceToTheBaseFileRecord == 0 ? true: false;
}

runList::runList(BYTE* _data)
{
    data = _data;
    BYTE *off = data;
    while(*off != 0 || (off - data) <= 0xFFFF)
    {
        BYTE sizeOflenth = *off & 0x0F;
        BYTE sizeOfOffset = (*off & 0xF0)>>4;
        clasterFragments tmp;
        off ++;

        int index = 0;
        for(index = 0; index < sizeOflenth; index ++)
        {
            tmp.lenth += (UINT64)(*(off + index)) << index * 8;
        }
        off += sizeOflenth;
        for(index = 0; index < sizeOfOffset; index ++)
        {
            tmp.begin += (UINT64)(*(off + index)) << index * 8;
        }
        off += sizeOfOffset;
        cf.push_back(tmp);
    }
}

attributeHeader::attributeHeader(BYTE* _data)
{
    lenthOfTheStream = 0;
    startingVCN = 0;
    lastVCN = 0;
    offsetToTheRunlist = 0;
    realSizeOfTheStream = 0;
    run = NULL;

    data = _data;
    type = *(DWORD*)(data + 0x0);
    lenth = *(DWORD*)(data + 0x4);
    nonResidentFlag = *(data + 0x8);

    offsetToTheContentPart = *(WORD*)(data + 0xA);
    isResident = nonResidentFlag == 0;
    
    if(isResident)
    {
        lenthOfTheStream = *(DWORD*)(data + 0x10);
    }
    else
    {
        startingVCN = *(UINT64*)(data + 0x10);
        lastVCN = *(UINT64*)(data + 0x18);
        offsetToTheRunlist = *(WORD*)(data + 0x20);
        realSizeOfTheStream = *(UINT*)(data + 0x30);
        run = new runList(data + offsetToTheRunlist);
    }
}

attributeHeader::~attributeHeader()
{
    if(!isResident)
    {
        SAFE_RELEASE_SINGLE_POINTER(run);
    }
}

attributeNAMEContent::attributeNAMEContent(BYTE* _data)
{
    data = _data;
    nameLenth = *(BYTE*)(data + 40);
    fileName = new BYTE[nameLenth * 2];
    memcpy(fileName, data + 42, nameLenth * 2);
}

attributeNAMEContent::~attributeNAMEContent()
{
    SAFE_RELEASE_VECTOR_POINTER(fileName);
}

attributeDATAContent::attributeDATAContent(BYTE* _data)
{
    data = _data;
}

attributeIndexRootContent::attributeIndexRootContent(BYTE* _data)
{
    data = _data;
}

attributeIndexAllocationContent::attributeIndexAllocationContent(BYTE* _data)
{
    data = _data;
}

attributeBitmapContent::attributeBitmapContent(BYTE* _data)
{
    data = _data;
}

template<typename T>
attribute<T>::attribute(BYTE* _data)
{
    content = NULL;
    data = _data;
    header = new attributeHeader(data);
    if(header->isResident)
    {
        content = new T(data + header->offsetToTheContentPart);
    }
    else
    {
        ;
    }
}

template<typename T>
attribute<T>::~attribute()
{
    SAFE_RELEASE_SINGLE_POINTER(header);
    if(header->isResident)
    {
        SAFE_RELEASE_SINGLE_POINTER(content);
    }
}

FR::FR(BYTE* _data)
{
    aName = NULL;
    aData = NULL;
    aIndexRoot = NULL;
    aIndexAllocationAttribute = NULL;
    aBitmap = NULL;

    data = _data;
    FRH = new FRHeader(_data);
    DWORD offset = FRH->offsetToTheSequencceOfAttributesPart;
    while(offset <= FRH->realSizeOfFR)
    {
        attributeHeader tmp(data + offset);
        if(tmp.type == 0x30)
        {
            aName = new attribute<attributeNAMEContent>(data + offset);
            //offset += aName->header->
        }
        else if(tmp.type == 0x80)
        {
            aData = new attribute<attributeDATAContent>(data + offset);
        }
        else if(tmp.type == 0x90)
        {
            aIndexRoot = new attribute<attributeIndexRootContent>(data + offset);
        }
        else if(tmp.type == 0xA0)
        {
            aIndexAllocationAttribute = new attribute<attributeIndexAllocationContent>(data + offset);
        }
        else if(tmp.type == 0xB0)
        {
            aBitmap = new attribute<attributeBitmapContent>(data + offset);
        }
    }
}

FR::~FR()
{
    SAFE_RELEASE_SINGLE_POINTER(aName);
    SAFE_RELEASE_SINGLE_POINTER(aData);
    SAFE_RELEASE_SINGLE_POINTER(aIndexRoot);
    SAFE_RELEASE_SINGLE_POINTER(aIndexAllocationAttribute);
    SAFE_RELEASE_SINGLE_POINTER(aBitmap);
}
