#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <exception>
#include <vector>
#include <chrono>
#include <bits/stdc++.h>

#include "ProcessManager.hpp"


class PointerScanner {
    public:
        PointerScanner();
        void scanPointers(std::string windowName);
        std::vector<DWORD64> filterCells(HANDLE processHandle, std::vector<DWORD64> cellsToCheck, BYTE cmpValue);
        std::vector<DWORD64> firstCellFilter(HANDLE processHandle, std::vector<MEMORY_BASIC_INFORMATION> usefulPages, BYTE cmpValue);
    private:
        ProcessManager procMan;
        void changeCellValue(HANDLE processHandle, DWORD64 cellAddr, BYTE valueToWrite);
};