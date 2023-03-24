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


std::string getInput(std::string messageToWrite)
{
    std::string input;
    std::cout << messageToWrite << std::endl;
    std::cin >> input;
    return input;
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
