
#include <algorithm>
#include "PointerScanner.hpp"


PointerScanner::PointerScanner() {
    this->procMan = ProcessManager(); 
}

HANDLE PointerScanner::getProcessHandle(std::string windowName) {
    return procMan.getProcessHandle(windowName, PROCESS_ALL_ACCESS);
}

std::vector<MEMORY_BASIC_INFORMATION> PointerScanner::getUsefulPages(HANDLE processHandle) {
    return procMan.getUsefulPages(processHandle);
}

std::vector<DWORD64> PointerScanner::filterCells(HANDLE processHandle, std::vector<DWORD64> cellsToCheck, BYTE cmpValue){
    std::vector<DWORD64> cellsMeetingCriteria;
    auto startTick = clock();
    for (DWORD64 cellAddr : cellsToCheck) {
        BYTE cellValue;
        BOOL errorCode = ReadProcessMemory(
            processHandle,
            (LPCVOID)cellAddr,
            &cellValue,
            1,
            NULL
        );
        if (errorCode == 0){
            // printf("Error\n");
            continue;
        }
        if (cellValue == cmpValue){
            cellsMeetingCriteria.push_back(cellAddr);
        }
    }
    auto endTick = clock();
    double executionTime = double(endTick - startTick) / double(CLOCKS_PER_SEC);
    printf("Time of execution: %f seconds\n", executionTime);
    return cellsMeetingCriteria;
}

std::vector<DWORD64> PointerScanner::firstCellFilter(HANDLE processHandle, std::vector<MEMORY_BASIC_INFORMATION> usefulPages, BYTE cmpValue) {
    std::vector<DWORD64> cellsMeetingCriteria;
    // unsigned int pageCounter = 0;

    // A buffer for reading the page data
    BYTE addrContentsBuffer[FIRST_FILTER_BUFFER_SIZE];

    for (MEMORY_BASIC_INFORMATION pageInfo : usefulPages) {

        // DWORD64 bytesRemaining = pageInfo.RegionSize;
        // DWORD64 baseAddress = (DWORD64)(pageInfo.BaseAddress);
        // while (bytesRemaining > 0){
        //     unsigned int currentIterationBufferSize;
        //     if (bytesRemaining >= FIRST_FILTER_BUFFER_SIZE){
        //         currentIterationBufferSize = FIRST_FILTER_BUFFER_SIZE;
        //     }
        //     else{
        //         currentIterationBufferSize = bytesRemaining;
        //     }
        //     bytesRemaining -= currentIterationBufferSize;
        //     DWORD64 bytesRead;
        //     DWORD64 errorCode = ReadProcessMemory(
        //         processHandle,
        //         (LPCVOID)baseAddress,
        //         &addrContentsBuffer,
        //         currentIterationBufferSize,
        //         &bytesRead
        //     );
        //     if (errorCode == 0){
        //         baseAddress += currentIterationBufferSize;
        //         continue;
        //     }
        //     for (DWORD64 offset = 0; offset < currentIterationBufferSize; offset++){
        //         BYTE byte = addrContentsBuffer[offset];
        //         if (byte == cmpValue){
        //             DWORD64 byteAddress = ((DWORD64)baseAddress + offset);
        //             cellsMeetingCriteria.push_back(byteAddress);
        //         }
        //     }
        //     baseAddress += currentIterationBufferSize;
        // }


        // printf("Start of iteration nr %d, page size: %u\n", pageCounter, pageInfo.RegionSize);
        // pageCounter++;
        
        DWORD64 pageAddrsToProcess = pageInfo.RegionSize;
        DWORD64 iterAddr = (DWORD64)(pageInfo.BaseAddress);

        while(pageAddrsToProcess > 0) {
            DWORD64 addrsProcessedThisIter = std::min(
                (int)pageAddrsToProcess, FIRST_FILTER_BUFFER_SIZE); // TODO uint???
            DWORD64 bytesRead;  // TODO
            DWORD64 errorCode = ReadProcessMemory(
                processHandle,
                (LPCVOID)iterAddr,
                &addrContentsBuffer,
                addrsProcessedThisIter,
                &bytesRead
            );
            // printf("%d, %d\n", errorCode, bytesRead);
            if (errorCode == 0){
                // Could not get addresses from the page, skip to the next one
                // printf("Error\n");
                // printf("%d, %d\n", errorCode, bytesRead);
                ;
            }
            for (DWORD64 offset = 0; offset < bytesRead; offset++){
                BYTE addrContents = addrContentsBuffer[offset];
                if (addrContents == cmpValue){
                    DWORD64 byteAddress = (DWORD64)iterAddr + offset;
                    cellsMeetingCriteria.push_back(byteAddress);
                }
            }
            iterAddr += addrsProcessedThisIter;
            pageAddrsToProcess -= addrsProcessedThisIter;
        }
    }
    return cellsMeetingCriteria;
}

void PointerScanner::changeCellValue(HANDLE processHandle, DWORD64 cellAddr, BYTE valueToWrite, unsigned int bytesAmount) {
    printf(
        "Placing %d in the cell %x.\n",
        valueToWrite,
        cellAddr
    );
    
    DWORD64 errorCode = WriteProcessMemory(
        processHandle,
        (LPVOID)cellAddr,
        &valueToWrite,
        1,
        NULL
    );
    
    if (errorCode == 0){
        printf("Error. Code: %d\n", GetLastError());
    }
}

