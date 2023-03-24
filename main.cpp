#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <exception>
#include <vector>
#include <chrono>
#include <bits/stdc++.h>

#define print(text) std::cout << text << std::endl
#define throwError(text) throw std::invalid_argument(text + std::string(" Error code: ") + std::to_string(GetLastError()))
#define BYTES_PER_CELL 8
#define ADDRESS_LISTING_TRESHOLD 5  // The max number of addresses found that will result in printing them out
#define HEX_ADDRESS_LENGTH 15       // i.e. 0xFF has length 5 (null sign included)

LPVOID getProcessBaseAddress(HANDLE processHandle, DWORD processID){
    DWORD numberOfModules = 200;
    HMODULE processModulesAddresses[numberOfModules];
    DWORD bytesNeeded = numberOfModules * 4;
    WINBOOL errorCode = EnumProcessModulesEx(
        processHandle,
        processModulesAddresses,
        sizeof(processModulesAddresses),
        &bytesNeeded,
        LIST_MODULES_ALL 
    );
    if (errorCode == 0){
        throwError("Cannot list process modules.");
    }

    std::cout << "Searching modules. Their array is at: " << &processModulesAddresses << ". The first module is at: " << processModulesAddresses[0] << std::endl;
    for (DWORD offset = 0; offset < numberOfModules; offset++){
        HMODULE processModuleAddress = processModulesAddresses[offset];
        
        char processModuleName[50];
        char processModulePath[50];

        // Module path
        DWORD filePathLengthBufferUsed = GetModuleFileNameExA(
            processHandle,
            processModuleAddress,
            processModulePath,
            50
        );

        // Module name
        DWORD fileNameLengthBufferUsed = GetModuleBaseName(
            processHandle, 
            processModuleAddress,
            processModuleName,
            50
        );

        std::cout << "--------------------------" << std::endl;
        std::cout << "Found modules's address: " << processModuleAddress << std::endl;
        std::cout << "Name: " << processModuleName << std::endl;
        if (strcmp(processModuleName, "javaw.exe") == 0){
            std::cout << "FOUND THE MAIN MODULE" << std::endl;

            MODULEINFO moduleInfo;
            WINBOOL errorCode = GetModuleInformation(
                processHandle,
                processModuleAddress,
                &moduleInfo,
                1024
            );
            if (errorCode == 0){
                throwError("Could not find the information about the module.");
            }
            LPVOID processBaseAddress = moduleInfo.lpBaseOfDll;
            return processBaseAddress;
        }
    }
    throw std::invalid_argument("The main module was not found.");
}

LPCSTR stringToLPCSTR(std::string& text){
    LPCSTR result = text.c_str();
    return result;
}

std::vector<LPVOID> filterCells(HANDLE processHandle, std::vector<LPVOID> cellsToCheck, BYTE cmpValue){
    auto startTick = clock();
    std::vector<LPVOID> cellsMeetingCriteria;
    for (unsigned int i = 0; i < cellsToCheck.size(); i++){
        LPVOID cellAddress = cellsToCheck[i];
        BYTE cellValue = 0;
        BOOL errorCode = ReadProcessMemory(
            processHandle,
            cellAddress,
            &cellValue,
            1,
            NULL
        );
        if (errorCode == 0){
            continue;
        }
        if (cellsToCheck.size() < 30){
            printf("Cell's value: %d\n", cellValue);
        }
        if (cellValue == cmpValue){
            cellsMeetingCriteria.push_back(cellAddress);
        }
    }
    auto endTick = clock();
    double executionTime = double(endTick - startTick) / double(CLOCKS_PER_SEC);
    printf("Time of execution: %f seconds\n", executionTime);
    printf("Found %d matches\n", cellsMeetingCriteria.size());
    if (cellsMeetingCriteria.size() <= ADDRESS_LISTING_TRESHOLD){

        // Write out the addresses if there are only a few of them
        std::cout << "Less than or 5 potential addresses found. These are:" << std::endl;
        char cellAddressInHex[HEX_ADDRESS_LENGTH];
        for (unsigned int i = 0; i < ADDRESS_LISTING_TRESHOLD; i++)
        {
            std::cout << cellsMeetingCriteria[i] << std::endl;
        }

        BYTE valueToWrite = 63;
        printf(
            "Found the right address: %p. Placing %d in the cell it is pointing at.\n",
            cellsMeetingCriteria[0],
            valueToWrite
        );
        
        DWORD64 errorCode = WriteProcessMemory(
            processHandle,
            cellsMeetingCriteria[0],
            &valueToWrite,
            1,
            NULL
        );
        if (errorCode == 0){
            printf("Error. Code: %d\n", GetLastError());
        }
    }
    return cellsMeetingCriteria;
}

std::string getInput(std::string messageToWrite)
{
    std::string input;
    std::cout << messageToWrite << std::endl;
    std::cin >> input;
    return input;
}

HANDLE getProcessHandle(std::string windowName, DWORD64 accessType){

    std::cout << "Program name: " + windowName << std::endl;

    HWND programWindow = FindWindow(NULL, stringToLPCSTR(windowName));
    if (programWindow == NULL){
        throwError("Failed to find the program.");
    }
    DWORD processID;
    GetWindowThreadProcessId(programWindow, &processID);
    
  	HANDLE processHandle;
    processHandle = OpenProcess(
        accessType,
        FALSE,
        processID
    );
    if (processHandle == INVALID_HANDLE_VALUE || processHandle == NULL){
        throwError("Failed to open the process.");
    }
    return processHandle;
}

DWORD getProcessPageSize(){
    SYSTEM_INFO systemInformation;
    GetSystemInfo(
        &systemInformation
    );
    return systemInformation.dwPageSize;
};

std::vector<MEMORY_BASIC_INFORMATION> getUsefulPages(HANDLE processHandle){
    auto startTick = clock();
    DWORD64 totalSizeInBytes = 0;
    std::vector<MEMORY_BASIC_INFORMATION> usefulPagesAdresses;
    DWORD pageSize = getProcessPageSize();
    DWORD64 currentPageAddress = 0x0;
    DWORD64 maxAddress = 0x7FFFFFFFFFFF; //128 TiB - 1 B
    long long test = 0;
    while (currentPageAddress <= maxAddress){
        MEMORY_BASIC_INFORMATION pageInformation;
        DWORD64 bytesRead = VirtualQueryEx(
            processHandle,
            (LPCVOID)currentPageAddress,
            &pageInformation,
            sizeof(pageInformation)
        );
        if (bytesRead == 0){
            printf("Page error. Code: %d\n", GetLastError());
            currentPageAddress += (DWORD64)pageSize;
        }
        else{
            if ((pageInformation.State == MEM_COMMIT)&&(pageInformation.Protect != PAGE_NOACCESS)){
                usefulPagesAdresses.push_back(pageInformation);
                totalSizeInBytes += pageInformation.RegionSize;
            }
            currentPageAddress = (DWORD64)pageInformation.BaseAddress + pageInformation.RegionSize;
        }
    }
    auto endTick = clock();
    double executionTime = double(endTick - startTick) / double(CLOCKS_PER_SEC);
    printf("Time of execution: %f seconds\n", executionTime);
    printf("Pages found: %d. Total size: %d bytes.\n", usefulPagesAdresses.size(), totalSizeInBytes);
    return usefulPagesAdresses;
};

std::vector<LPVOID> firstCellFilter(HANDLE processHandle, std::vector<MEMORY_BASIC_INFORMATION> pagesToFilter, BYTE cmpValue){
    auto startTick = clock();
    unsigned int maxBufferSize = 1500000;
    BYTE buffer[maxBufferSize];
    printf("Applying the first filter...\n");
    std::vector<LPVOID> addressesWithRightVal;
    DWORD64 counter = 0;
    for (MEMORY_BASIC_INFORMATION pageInfo : pagesToFilter){
        printf("Start of iteration nr %d, page size: %u\n", counter, pageInfo.RegionSize);
        counter++;
        DWORD64 bytesRemaining = pageInfo.RegionSize;
        DWORD64 baseAddress = (DWORD64)(pageInfo.BaseAddress);
        while (bytesRemaining > 0){
            unsigned int currentIterationBufferSize;
            if (bytesRemaining >= maxBufferSize){
                currentIterationBufferSize = maxBufferSize;
            }
            else{
                currentIterationBufferSize = bytesRemaining;
            }
            bytesRemaining -= currentIterationBufferSize;
            DWORD64 bytesRead;
            DWORD64 errorCode = ReadProcessMemory(
                processHandle,
                (LPCVOID)baseAddress,
                &buffer,
                currentIterationBufferSize,
                &bytesRead
            );
            if (errorCode == 0){
                baseAddress += currentIterationBufferSize;
                continue;
            }
            for (DWORD64 offset = 0; offset < currentIterationBufferSize; offset++){
                BYTE byte = buffer[offset];
                if (byte == cmpValue){
                    LPVOID byteAddress = (LPVOID)((DWORD64)baseAddress + offset);
                    addressesWithRightVal.push_back(byteAddress);
                }
            }
            baseAddress += currentIterationBufferSize;
        }
    }
    auto endTick = clock();
    double executionTime = double(endTick - startTick) / double(CLOCKS_PER_SEC);
    printf("Time of execution: %f seconds\n", executionTime);
    printf("Addresses with the right value: %d\n", addressesWithRightVal.size());
    return addressesWithRightVal;
}

void sleep(unsigned int timeInSec){
    printf("Waiting %i seconds for the data to update...\n", timeInSec);
    Sleep(timeInSec * 1000);
    printf("Data updated\n");
    return;
}

DWORD64 getSearchValue(){
    printf("Enter the value to find: ");
    DWORD64 valueToFind = 0;
    scanf("%d", &valueToFind);
    return valueToFind;
}

void scanPointers(std::string windowName){
    HANDLE processHandle = getProcessHandle(windowName, PROCESS_ALL_ACCESS);
    BYTE valueToFind = (BYTE)getSearchValue();
    std::vector<LPVOID> cellsToCheck = firstCellFilter(
        processHandle,
        getUsefulPages(processHandle),
        valueToFind
    );
    while (true){
        BYTE valueToFind = (BYTE)getSearchValue();
        cellsToCheck = filterCells(
            processHandle,
            cellsToCheck,
            valueToFind
        );
    }
}

int main(){
    std::cout << "Enter the program name: ";
    std::string windowName;
    std::getline(std::cin, windowName);
    try{
        scanPointers(windowName);
    }
    catch (std::invalid_argument& exception){
        printf("===== ERROR =====\n");
        printf((exception.what() + std::string("\n")).c_str());
        printf("=================\n");
        return 1;
    }
    return 0;
};
