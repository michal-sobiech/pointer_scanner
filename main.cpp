#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <exception>
#include <vector>
#include <chrono>
#include <bits/stdc++.h>

#include "UI.hpp"

#define BYTES_PER_CELL 8
#define ADDRESS_LISTING_TRESHOLD 5  // The max number of addresses found that will result in printing them out
#define HEX_ADDRESS_LENGTH 15       // i.e. 0xFF has length 5 (null sign included)


void sleep(unsigned int timeInSec){
    printf("Waiting %i seconds for the data to update...\n", timeInSec);
    Sleep(timeInSec * 1000);
    printf("Data updated\n");
    return;
}

int main(){
    UI userInterface = UI();

    try {
        userInterface.start();
    }
    catch (std::invalid_argument& exception){
        printf("===== ERROR =====\n");
        printf((exception.what() + std::string("\n")).c_str());
        printf("=================\n");
        return 1;
    }
    return 0;
};
