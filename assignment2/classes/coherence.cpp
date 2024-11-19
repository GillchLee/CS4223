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

//#define protocol 0 1 (MESI or DRAGON)

#define DRAMsize 0xFFFFFFFF // 4GB DRAM

int CACHE_SIZE = 4096;
int CACHE_ASSOC = 2;
int CACHE_BLOCK_SIZE = 32;
std::string FILE_NAME = "blackscholes_";
std::string PROTOCOL = "NONE";

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

    std::vector<std::string> filenames;
    std::vector<std::ifstream> files;

    for (int i = 0; i < 4; ++i) {       // put filenaems to filenames[0~3]
        std::string filename = generateFileName(i);
        filenames.push_back(filename);
        std::cout << "Generated filename: " << filename << std::endl;
    
    
        //Open files
        std::ifstream file("C:/Users/Diana/CLionProjects/untitled/CS4223/assignment2/data/blackscholes_0.data");
         if (!file.is_open())
         {
             std::cerr << "can't open file " << filename << std::endl;
             return 1; // stop program
         }
        files.push_back(std::move(file));
    }

    std::cout << "open files succeed" << std::endl;


    std::string line;
    std::pair<int, unsigned int> result;

    
    Bus bus;
    DRAM dram(DRAMsize);
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

while(Readfile_available) {
    Readfile_available = false;
    total_cycles++;

    if(std::getline(files[0], line)){
        if (!cpu1.Execute(result.first, result.second)) {
            result=readLabelAndData(line);
        }
        Readfile_available = true;
    }
}
//     if(std::getline(files[1],line)){
//         result=readLabelAndData(line);
//         cpu2.Execute(result.first, result.second);
//         Readfile_available = true;
//     }
//     if(std::getline(files[2], line)){
//         result=readLabelAndData(line);
//         cpu3.Execute(result.first, result.second);
//         Readfile_available = true;
//     }
//     if(std::getline(files[3],line)){
//         result=readLabelAndData(line);
//         cpu4.Execute(result.first, result.second);
//         Readfile_available = true;
//     }
//     total_cycles++;
// }

    std::cout << "TOTAL CYCLES: " << total_cycles << std::endl;
    cpu1.PrintStats();

    return 0;
}

