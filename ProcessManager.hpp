#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <exception>
#include <vector>
#include <chrono>
#include <bits/stdc++.h>


class ProcessManager {
    public:
        LPVOID getProcessBaseAddress(HANDLE processHandle, DWORD processID);
        LPCSTR stringToLPCSTR(std::string& text);
        std::vector<MEMORY_BASIC_INFORMATION> getUsefulPages(HANDLE processHandle);
        HANDLE getProcessHandle(std::string windowName, DWORD64 accessType);
        std::vector<MEMORY_BASIC_INFORMATION> getUsefulPages(HANDLE processHandle);
        std::vector<LPVOID> firstCellFilter(HANDLE processHandle, std::vector<MEMORY_BASIC_INFORMATION> pagesToFilter, BYTE cmpValue);
        std::vector<LPVOID> filterCells(HANDLE processHandle, std::vector<LPVOID> cellsToCheck, BYTE cmpValue);
}