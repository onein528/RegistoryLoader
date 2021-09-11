// Copyright (c) 2021 onein528
// Licensed under the MIT License.

#include "RegistryLoader.h"


BOOL ParseFileHeader(PBASE_BLOCK pBaseBlock, HANDLE hFile);
BOOL ParseHiveBinHeader(PHBIN pHBin, HANDLE hFile);
DWORD ParseCell(HANDLE hFile, LPDWORD pCellSize);


int wmain(void) {

    BASE_BLOCK BaseBlock = { 0 };
    HBIN HBin = { 0 };
    KEY_NODE KeyNode = { 0 };

    DWORD dwResult = SUCCESS;

    HANDLE hFile = CreateFileW(L"C:\\Users\\T31068068\\Desktop\\BCD_Backup", GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    ParseFileHeader(&BaseBlock, hFile);

    ParseHiveBinHeader(&HBin, hFile);

    DWORD dwCellType = 0;
    DWORD dwCellSize = 0;

    while (dwResult != FAILURE) {

        dwResult = dwCellType = ParseCell(hFile, &dwCellSize);

        switch (dwCellType) {

        case CELL_KEY_NODE:

            ParseKeyNodeCell(&KeyNode, hFile, dwCellSize);
        }

        break; // temp
    }

    return SUCCESS;
}


BOOL ParseFileHeader(PBASE_BLOCK pBaseBlock, HANDLE hFile) {

    // Base block size is 4096
    BYTE byReadData[4096] = { 0 };
    PBYTE pReadedData = NULL;
    DWORD nBaseBlockSize = 4096;
    DWORD nReadedSize = 0;

    BYTE byDwordArray[4] = { 0 };
    FILETIME ftBaseBlockTimeStamp = { 0 };
    SYSTEMTIME stBaseBlockTimeStamp = { 0 };
    CHAR szTempSigneture[5] = "";

    // read file until 4096 bytes
    if (ReadFile(hFile, &byReadData, nBaseBlockSize, &nReadedSize, NULL) == FAILURE) {

        wprintf(L"ReadFile failed with %d", GetLastError());
        return FAILURE;
    }

    pReadedData = byReadData;

    wprintf(L"File Header:\n");

    // Regf signeture
    for (int i = 0; i < 4; i++) szTempSigneture[i] = pReadedData[i];
    CharToWchar(pBaseBlock->szRegfSigneture, szTempSigneture, 5);
    pReadedData += 4;

    wprintf(L"    Signeture:  %s\n", pBaseBlock->szRegfSigneture);

    // Primary sequence number
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->dwPrimarySequenceNumber = *(DWORD*)byDwordArray;
    pReadedData += 4;

    // Secondary sequence number
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->dwSecondarySequenceNumber = *(DWORD*)byDwordArray;
    pReadedData += 4;

    wprintf(L"    SequenceNo. { %d, %d }\n", pBaseBlock->dwPrimarySequenceNumber, pBaseBlock->dwSecondarySequenceNumber);

    // Last written timestamp
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->ftLastWrittenTimeStamp.dwLowDateTime = *(DWORD*)byDwordArray;
    pReadedData += 4;
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->ftLastWrittenTimeStamp.dwHighDateTime = *(DWORD*)byDwordArray;
    pReadedData += 4;

    SYSTEMTIME stLastWrittenTimeStamp = { 0 };
    FileTimeToSystemTime(&pBaseBlock->ftLastWrittenTimeStamp, &stLastWrittenTimeStamp);
    wprintf(L"    Last written time: %04d/%02d/%02d %02d:%02d\n"
        , stLastWrittenTimeStamp.wYear, stLastWrittenTimeStamp.wMonth, stLastWrittenTimeStamp.wDay
        , stLastWrittenTimeStamp.wHour, stLastWrittenTimeStamp.wMinute);

    // Major version
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->dwWriterMajorVersion = *(DWORD*)byDwordArray;
    pReadedData += 4;

    // Minor version
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->dwWriterMinorVersion = *(DWORD*)byDwordArray;
    pReadedData += 4;

    wprintf(L"    Maj.Min  %d.%d\n", pBaseBlock->dwWriterMajorVersion, pBaseBlock->dwWriterMinorVersion);

    // File type
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->dwFileType = *(DWORD*)byDwordArray;
    pReadedData += 4;

    wprintf(L"    File type:  %d\n", pBaseBlock->dwFileType);

    // File format
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->dwFileFormat = *(DWORD*)byDwordArray;
    pReadedData += 4;

    wprintf(L"    File format:  %d\n", pBaseBlock->dwFileFormat);

    // Root cell offset
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->dwRootCellOffset = *(DWORD*)byDwordArray;
    pReadedData += 4;

    wprintf(L"    Root cell offset:  %d\n", pBaseBlock->dwRootCellOffset);

    // Hive bins data size
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->dwHiveBinsDataSize = *(DWORD*)byDwordArray;
    pReadedData += 4;

    wprintf(L"    Hive bins data:  %d\n", pBaseBlock->dwHiveBinsDataSize);

    // Clustering factor
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->dwClusteringFactor = *(DWORD*)byDwordArray;
    pReadedData += 4;

    wprintf(L"    Clustering factor:  %d\n", pBaseBlock->dwClusteringFactor);

    // File name
    CHAR szTempFileName[64];
    for (int i = 0; i < 64; i++) szTempFileName[i] = (CHAR)pReadedData[i];
    pReadedData += 64;

    ByteToWchar(pBaseBlock->szFileName, szTempFileName, 64);
    wprintf(L"    File name:  %s\n", pBaseBlock->szFileName);

    // Resource manager guid
    ByteToGuid(pReadedData, &pBaseBlock->guidRmId);
    WCHAR szRmGuidString[40] = L"";
    GuidToWchar(szRmGuidString, &pBaseBlock->guidRmId);
    pReadedData += 16;

    wprintf(L"    RmId:  %s\n", szRmGuidString);


    // Log file guid
    ByteToGuid(pReadedData, &pBaseBlock->guidLogId);
    WCHAR szLogGuidString[40] = L"";
    GuidToWchar(szLogGuidString, &pBaseBlock->guidLogId);
    pReadedData += 16;

    wprintf(L"    LogId:  %s\n", szLogGuidString);

    // Guid flags
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->dwGuidFlags = *(DWORD*)byDwordArray;
    pReadedData += 4;

    wprintf(L"    Guid flags:  %x\n", pBaseBlock->dwGuidFlags);

    // Transaction manager guid
    ByteToGuid(pReadedData, &pBaseBlock->guidTmId);
    WCHAR szTmGuidString[40] = L"";
    GuidToWchar(szTmGuidString, &pBaseBlock->guidTmId);
    pReadedData += 16;

    wprintf(L"    TmId:  %s\n", szTmGuidString);

    // Guid signeture
    for (int i = 0; i < 4; i++) szTempSigneture[i] = pReadedData[i];
    CharToWchar(pBaseBlock->szGuidSigneture, szTempSigneture, 5);
    pReadedData += 4;

    wprintf(L"    Guid signeture:  %s\n", pBaseBlock->szGuidSigneture);

    // Last reorganized time

    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->ftLastReorganizedTime.dwLowDateTime = *(DWORD*)byDwordArray;
    pReadedData += 4;
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pBaseBlock->ftLastReorganizedTime.dwHighDateTime = *(DWORD*)byDwordArray;
    pReadedData += 4;

    SYSTEMTIME stLastReorganizedTime = { 0 };
    FileTimeToSystemTime(&pBaseBlock->ftLastReorganizedTime, &stLastReorganizedTime);
    wprintf(L"    Last reorganized time: %04d/%02d/%02d %02d:%02d\n"
        , stLastReorganizedTime.wYear, stLastReorganizedTime.wMonth, stLastReorganizedTime.wDay
        , stLastReorganizedTime.wHour, stLastReorganizedTime.wMinute);

    // Offline registry signeture

    pReadedData += 4;

    // Offline registry flags
    pReadedData += 4;

    // Reserved from (offset)184 to 184 + (size)324
    pReadedData += 324;

    // Check sum
    pReadedData += 4;

    // Serialization time stamp
    pReadedData += 8;

    // Reserved size
    pReadedData += 3520;

    // ThawTmId
    pReadedData += 16;

    // ThawRmId
    pReadedData += 16;

    // ThawLogId
    pReadedData += 16;

    // Boot type
    pReadedData += 4;

    // Boot recover
    pReadedData += 4;

    // Rewind
    // pReadedData -= 4096;

    wprintf(L"\n");

    // Move 4096 bytes
    SetFilePointer(hFile, 4096, NULL, FILE_BEGIN);

    return SUCCESS;
}


BOOL ParseHiveBinHeader(PHBIN pHBin, HANDLE hFile) {

    // Hive bin header size is 32
    BYTE byReadData[32] = { 0 };
    PBYTE pReadedData = NULL;
    DWORD nBaseBlockSize = 32;
    DWORD nReadedSize = 0;

    BYTE byDwordArray[4] = { 0 };
    CHAR szTempSigneture[5] = "";

    // read file until 4096 bytes
    if (ReadFile(hFile, &byReadData, nBaseBlockSize, &nReadedSize, NULL) == FAILURE) {

        wprintf(L"ReadFile failed with %d", GetLastError());
        return FAILURE;
    }

    pReadedData = byReadData;

    wprintf(L"Hive bin Header:\n");

    // Regf signeture
    for (int i = 0; i < 4; i++) szTempSigneture[i] = pReadedData[i];
    CharToWchar(pHBin->szHBinSigneture, szTempSigneture, 5);
    pReadedData += 4;

    wprintf(L"    Signeture:  %s\n", pHBin->szHBinSigneture);

    // Relative offset
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pHBin->dwRelativeOffset = *(DWORD*)byDwordArray;
    pReadedData += 4;

    wprintf(L"    Relative offset:  %d\n", pHBin->dwRelativeOffset);

    // Size
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pHBin->dwSize = *(DWORD*)byDwordArray;
    pReadedData += 4;

    wprintf(L"    Size:  %d\n", pHBin->dwSize);

    // Reserved
    pReadedData += 8;

    // Time stamp
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pHBin->TimeStamp.dwLowDateTime = *(DWORD*)byDwordArray;
    pReadedData += 4;
    for (int i = 0; i < 4; i++) byDwordArray[i] = pReadedData[i];
    pHBin->TimeStamp.dwHighDateTime = *(DWORD*)byDwordArray;
    pReadedData += 4;

    SYSTEMTIME stTimeStamp;
    FileTimeToSystemTime(&pHBin->TimeStamp, &stTimeStamp);
    wprintf(L"    Time stamp: %04d/%02d/%02d %02d:%02d\n"
        , stTimeStamp.wYear, stTimeStamp.wMonth, stTimeStamp.wDay
        , stTimeStamp.wHour, stTimeStamp.wMinute);

    // Spara (or MemAlloc)
    pReadedData += 4;

    wprintf(L"\n");

    // Move 32 bytes
    SetFilePointer(hFile, 4096 + 32, NULL, FILE_BEGIN);

    return SUCCESS;
}


DWORD ParseCell(HANDLE hFile, LPDWORD pCellSize) {

    BYTE byReadData[6] = { 0 };
    PBYTE pReadedData = NULL;
    DWORD nBaseBlockSize = 6;
    DWORD nReadedSize = 0;

    BYTE dwDwordArray[4] = { 0 };
    CHAR szTempSigneture[3] = "";

    // read 6 bytes
    if (ReadFile(hFile, &byReadData, nBaseBlockSize, &nReadedSize, NULL) == FAILURE) {

        wprintf(L"ReadFile failed with %d", GetLastError());
        return FAILURE;
    }

    pReadedData = byReadData;

    wprintf(L"Cell:\n");

    WCHAR szCellSigneture[3] = L"";
    DWORD dwSize = 0;

    // Cell size
    for (int i = 0; i < 4; i++) dwDwordArray[i] = pReadedData[i];
    dwSize = abs(*(DWORD*)dwDwordArray);
    *pCellSize = dwSize;
    pReadedData += 4;

    wprintf(L"    Size:  %d\n", dwSize);

    // Cell signeture
    for (int i = 0; i < 2; i++) szTempSigneture[i] = pReadedData[i];
    CharToWchar(szCellSigneture, szTempSigneture, 3);
    pReadedData += 2;

    wprintf(L"    Signeture:  %s\n", szCellSigneture);

    if (CHECK_RETURN(wcscmp(szCellSigneture, L"nk"))) {

        return CELL_KEY_NODE;
    }

    // temporary return
    return FAILURE;

    return SUCCESS;
}
