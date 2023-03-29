
#include <algorithm>
#include "PointerScanner.hpp"

#define MAX_PROCESSED_ADDRS 2000000
#define FIRST_FILTER_BUFFER_SIZE 2000000
#define CELL_SIZE 4
#define SMALL_CELL_AMOUNT 5


PointerScanner::PointerScanner() {
    this->procMan = ProcessManager(); 
}

void PointerScanner::scanPointers(std::string windowName){
    HANDLE processHandle = procMan.getProcessHandle(windowName, PROCESS_ALL_ACCESS);
    
    printf("Enter the value to find: \n");
    DWORD64 valueToFind = 0;
    scanf("%d", &valueToFind);

    std::vector<DWORD64> cellsToCheck = firstCellFilter(
        processHandle,
        procMan.getUsefulPages(processHandle),
        valueToFind
    );

    printf("Cells with value %d: %d\n", valueToFind, cellsToCheck.size());

    while (true){
        
        printf("Enter the value to find: \n");
        DWORD64 valueToFind = 0;
        scanf("%d", &valueToFind);

        cellsToCheck = filterCells(
            processHandle,
            cellsToCheck,
            valueToFind
        );

        printf("Cells with value %d: %d\n", valueToFind, cellsToCheck.size());

        if (cellsToCheck.size() == 0) {
            return;
        }
        if (cellsToCheck.size() < SMALL_CELL_AMOUNT) {
            printf("Small number of cells, printing out the addresses (in dec):\n");
            for (DWORD64 cellAddr : cellsToCheck) {
                printf("%x\n", cellAddr);
                printf("%d\n", cellAddr);
            }
            printf("What do you want to do now?\n"
                   "1 - keep searching\n"
                   "2 - change the value\n");
            unsigned int input;
            std::cin >> input;
            if (input == 1) {
                continue;
            }
            else {
                printf("What do you want to do now?\n"
                       "1 - change singular byte\n"
                       "2 - change the value of 2 bytes (current and next)\n"
                       "3 - change the value of 2 bytes (previous and next)\n");
                std::cin >> input;

                printf("Choose the address:\n");
                for (unsigned int i = 0; i < cellsToCheck.size(); i++) {
                    printf("%d - %x\n", i + 1, cellsToCheck.at(i));
                }
                std::cin >> input;
                DWORD64 cellAddr = cellsToCheck.at(input - 1);

                printf("Choose the value to write:\n");
                unsigned int valueToWrite;
                std::cin >> valueToWrite;

                switch (input) {
                    case 1:
                        changeSingleCellValue(
                            processHandle,
                            cellAddr,
                            (BYTE)valueToWrite
                        );
                        break;
                    case 2:
                        changeTwoCellValuesBE(
                            processHandle,
                            cellAddr,
                            (WORD)valueToWrite
                        );
                        break;
                    case 3:
                        changeTwoCellValuesLE(
                            processHandle,
                            cellAddr,
                            (WORD)valueToWrite
                        );
                        break;
                    default:
                        printf("invalid option");
                }    
                return;
            }
        }
    }
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

void PointerScanner::changeCellValue(HANDLE processHandle, DWORD64 cellAddr, DWORD64 valueToWrite, unsigned int bytesAmount) {
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

void PointerScanner::changeSingleCellValue(HANDLE processHandle, DWORD64 cellAddr, BYTE valueToWrite) {
    changeCellValue(processHandle, cellAddr, (DWORD64)valueToWrite, 1);
}

void PointerScanner::changeTwoCellValuesBE(HANDLE processHandle, DWORD64 cellAddr, WORD valueToWrite) {
    changeCellValue(processHandle, cellAddr, (DWORD64)valueToWrite, 2);
}

void PointerScanner::changeTwoCellValuesLE(HANDLE processHandle, DWORD64 cellAddr, WORD valueToWrite) {
    changeCellValue(processHandle, cellAddr-1, (DWORD64)valueToWrite, 2);
}