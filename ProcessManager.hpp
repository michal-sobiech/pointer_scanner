#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <exception>
#include <vector>
#include <chrono>
#include <bits/stdc++.h>


class ProcessManager {
    public:
        ProcessManager();
        LPVOID getProcessBaseAddress(HANDLE processHandle, DWORD processID);
        LPCSTR stringToLPCSTR(std::string& text);
        std::vector<MEMORY_BASIC_INFORMATION> getUsefulPages(HANDLE processHandle);
        HANDLE getProcessHandle(std::string windowName, DWORD64 accessType);
        DWORD getProcessPageSize();
};