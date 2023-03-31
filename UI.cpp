#include "UI.hpp"

#define ZIGZAG "=============================\n"


UI::UI() {
    this->ptrScanner = PointerScanner();
}

std::string UI::intToHexString(unsigned int value) {
    char hexStr[100];
    sprintf(hexStr, "%x", value);
    return std::string(hexStr);
}

void UI::start() {
    std::string windowName = getStringInput("Enter the window name: ");
    HANDLE processHandle = ptrScanner.getProcessHandle(windowName);
    std::vector<MEMORY_BASIC_INFORMATION> usefulPages = ptrScanner.getUsefulPages(processHandle);

    unsigned int valueToFind = getUIntInput("Enter the value to find: ");
    std::vector<DWORD64> cellsToCheck = ptrScanner.firstCellFilter(
        processHandle,
        usefulPages,
        valueToFind
    );

    printf("Cells with value %d: %d\n", valueToFind, cellsToCheck.size());

    while (true) {
        
        unsigned int valueToFind = getUIntInput("Enter the value to find: \n");

        cellsToCheck = ptrScanner.filterCells(
            processHandle,
            cellsToCheck,
            valueToFind
        );

        printf("Cells with value %d: %d\n", valueToFind, cellsToCheck.size());

        if (cellsToCheck.size() == 0) {
            printf("No cells found.");
            return;
        }
        if (cellsToCheck.size() < SMALL_CELL_AMOUNT) {
            printf("Small number of cells, printing out the addresses (in dec):\n");
            for (DWORD64 cellAddr : cellsToCheck) {
                printf("%x\n", cellAddr);
            }

            unsigned int input = getUIntInput(
                ZIGZAG
                "1 - keep searching\n"
                "2 - write to process memory\n"
                "Choose: "
            );
            if (input == 1) {
                continue;
            }
            while (true) {
                std::string message = ZIGZAG;
                for (unsigned int i = 0; i < cellsToCheck.size(); i++) {
                    message += (
                        std::to_string(i + 1)
                        + " - 0x"
                        + intToHexString((unsigned int)cellsToCheck.at(i))
                        + "\n"
                    );
                }
                message += "Choose: ";
                unsigned int input = getUIntInput(message);
                DWORD64 chosenCellAddr = DWORD64(cellsToCheck.at(input - 1));

                cellWritingProcedure(processHandle, chosenCellAddr);
            }
        }
    }
}

void UI::cellWritingProcedure(HANDLE processHandle, DWORD64 chosenCellAddr) {
    while (true) {
        std:: string message = (
            ZIGZAG
            "1 - Increase current address by 1 byte\n"
            "2 - Decrease current address by 1 byte\n"
            "3 - Write to the currently chosen cell ("
            + intToHexString(chosenCellAddr)
            + ")\n"
            + "Choose: "
        );
        unsigned int input = getUIntInput(message);
        unsigned int valueToWrite;
        switch(input) {
            case 1:
                chosenCellAddr++;
                break;
            case 2:
                chosenCellAddr--;
                break;
            case 3:
                valueToWrite = getUIntInput("Enter the value to write (0-255): ");
                ptrScanner.changeCellValue(
                    processHandle,
                    chosenCellAddr,
                    (BYTE)valueToWrite,
                    1
                );
                return;
            default:
                break;
        }
    }
}


unsigned int UI::getUIntInput(std::string text) {
    std::cout << text;
    unsigned int input;
    std::cin >> input;
    return input;
}

std::string UI::getStringInput(std::string text) {
    std::cout << text;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

