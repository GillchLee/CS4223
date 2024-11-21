#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <mutex>
#include <thread>
#include <sstream>

#include "CPU.h"
#include "Bus.h"
#include "Cache.h"
#include "DRAM.h"

// #define DRAMsize 0xFFFFFFFF // 4GB DRAM

// Function to read one line from the file and return label and data (as integer)
std::pair<int, unsigned int> readLabelAndData(const std::string& line) {
    std::istringstream iss(line);
    int label;
    unsigned int data;

    // Parse label and hex data from the line
    if (iss >> label >> std::hex >> data) {
        return {label, data};  // Return label and data as a pair
    }

    // If parsing fails, return default error values
    return {-1, 0};
}

std::string generateFileName(const int index) {
    return "data/" + std::string(FILE_NAME) + std::to_string(index) + ".data";
}

int main(int argc, char* argv[])
{
    if (argc == 6) {
        PROTOCOL = argv[1];
        FILE_NAME = argv[2];
        CACHE_SIZE = atoi(argv[3]);
        CACHE_ASSOC = atoi(argv[4]);
        CACHE_BLOCK_SIZE = atoi(argv[5]);
    }
    // else {
    //     return -6;
    // }

    std::vector<std::string> filenames;
    std::vector<std::ifstream> files;

    for (int i = 0; i < 4; ++i) {       // put filenaems to filenames[0~3]
        std::string filename = generateFileName(i);
        filename = "D:/Clion/CS4223AAAA/assignment2/data/blackscholes_" + std::to_string(i) + ".data";
        filenames.push_back(filename);
        std::cout << "Generated filename: " << filename << std::endl;

        //Open files
        std::ifstream file(filename);
         if (!file.is_open())
         {
             std::cerr << "can't open file " << filename << std::endl;
             return 1; // stop program
         }
        files.push_back(std::move(file));
    }

    std::cout << "open files succeed" << std::endl;


    std::string line1;
    std::string line2;
    std::string line3;
    std::string line4;
    std::pair<int, unsigned int> result1;
    std::pair<int, unsigned int> result2;
    std::pair<int, unsigned int> result3;
    std::pair<int, unsigned int> result4;

    
    Bus bus;
    DRAM dram;
    Cache cache1(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC); // Cache size, block size, n-way associative
    Cache cache2(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC);
    Cache cache3(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC);
    Cache cache4(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC);

    CPU cpu1(CACHE_BLOCK_SIZE, 1, &cache1, &bus, &dram, 0, false, -1, 0,
            100, 
            0, 0, 0, 0, 0, 0);
    CPU cpu2(CACHE_BLOCK_SIZE, 2, &cache2, &bus, &dram, 0, false, -1, 0,
            100, 
            0, 0, 0, 0, 0 ,0);    
    CPU cpu3(CACHE_BLOCK_SIZE, 3, &cache3, &bus, &dram, 0, false, -1, 0,
            100, 
            0, 0, 0, 0, 0, 0);
    CPU cpu4(CACHE_BLOCK_SIZE, 4, &cache4, &bus, &dram, 0, false, -1, 0,
             100,
            0, 0, 0, 0, 0, 0);


// for multicore cycles

unsigned long long total_cycles = -1;
bool Readfile_available = true;
bool flag1;
bool flag2;
bool flag3;
bool flag4;
if (std::getline(files[0], line1)){
  flag1=true;
  }
if (std::getline(files[1], line2)){
    flag2=true;
}
if (std::getline(files[2], line3)){
    flag3=true;
}
if (std::getline(files[3], line4)){
    flag4=true;
}

while(Readfile_available) {
    Readfile_available = false;
    total_cycles++;
    bus.execute(&dram);
    if (flag1){
        if (!cpu1.Execute(result1.first, result1.second)) {
            if (std::getline(files[0], line1)){
              flag1=true;
            } else{
              flag1=false;
              }
            result1=readLabelAndData(line1);
        }
        Readfile_available = true;
    }
    // if (flag2){
    //     if (!cpu2.Execute(result2.first, result2.second)) {
    //         if (std::getline(files[1], line2)){
    //             flag2=true;
    //         } else{
    //             flag2=false;
    //         }
    //         result2=readLabelAndData(line2);
    //     }
    //     Readfile_available = true;
    // }
    // if (flag3){
    //     if (!cpu3.Execute(result3.first, result3.second)) {
    //         if (std::getline(files[2], line3)){
    //             flag3=true;
    //         } else{
    //             flag3=false;
    //         }
    //         result3=readLabelAndData(line3);
    //     }
    //     Readfile_available = true;
    // }
    // if (flag4){
    //     if (!cpu4.Execute(result4.first, result4.second)) {
    //         if (std::getline(files[3], line4)){
    //             flag4=true;
    //         } else{
    //             flag4=false;
    //         }
    //         result4=readLabelAndData(line4);
    //     }
    //     Readfile_available = true;
    // }
    dram.execute(&bus);
}
    cpu1.PrintStats();
    cpu2.PrintStats();
    cpu3.PrintStats();
    cpu4.PrintStats();
    return 0;
}

