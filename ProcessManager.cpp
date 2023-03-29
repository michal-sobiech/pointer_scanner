#include "ProcessManager.hpp"


ProcessManager::ProcessManager() {}

HANDLE ProcessManager::getProcessHandle(std::string windowName, DWORD64 accessType){

    std::cout << "Program name: " + windowName << std::endl;

    HWND programWindow = FindWindow(NULL, stringToLPCSTR(windowName));

    if (programWindow == NULL){
        throw std::invalid_argument("Failed to find the program.");
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
        throw std::invalid_argument("Failed to open the process.");
    }
    return processHandle;
}

DWORD ProcessManager::getProcessPageSize(){
    SYSTEM_INFO systemInformation;
    GetSystemInfo(
        &systemInformation
    );
    return systemInformation.dwPageSize;
};

LPCSTR ProcessManager::stringToLPCSTR(std::string& text){
    LPCSTR result = text.c_str();
    return result;
}

std::vector<MEMORY_BASIC_INFORMATION> ProcessManager::getUsefulPages(HANDLE processHandle){
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

