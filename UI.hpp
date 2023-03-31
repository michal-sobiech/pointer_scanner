#include "PointerScanner.hpp"


class UI {
    public:
        UI();
        void start();
        unsigned int getUIntInput(std::string text);
        std::string getStringInput(std::string text);
    private:
        PointerScanner ptrScanner;
        std::string intToHexString(unsigned int value);
        void cellWritingProcedure(HANDLE processHandle, DWORD64 chosenCellAddr);
};