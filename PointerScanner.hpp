#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <exception>
#include <vector>
#include <chrono>
#include <bits/stdc++.h>

#include "ProcessManager.hpp"

#define MAX_PROCESSED_ADDRS 2000000
#define FIRST_FILTER_BUFFER_SIZE 2000000
#define CELL_SIZE 4
#define SMALL_CELL_AMOUNT 5


class PointerScanner {
    public:
        PointerScanner();
        void scanPointers(std::string windowName);
        std::vector<DWORD64> filterCells(HANDLE processHandle, std::vector<DWORD64> cellsToCheck, BYTE cmpValue);
        std::vector<DWORD64> firstCellFilter(HANDLE processHandle, std::vector<MEMORY_BASIC_INFORMATION> usefulPages, BYTE cmpValue);
        HANDLE getProcessHandle(std::string windowName);
        std::vector<MEMORY_BASIC_INFORMATION> getUsefulPages(HANDLE processHandle);
        void changeCellValue(HANDLE processHandle, DWORD64 cellAddr, BYTE valueToWrite, unsigned int bytesAmount);
    private:
        ProcessManager procMan;
};