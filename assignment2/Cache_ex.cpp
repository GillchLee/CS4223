#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <cstdint> // for using uint32_t 
#include <stdexcept>
#include <list>
#include <mutex>
#include <thread>


//#define protocol 0 1 (MESI or DRAGON)

#define CACHE_SIZE 4096 //1024
#define CACHE_ASSOC 2
#define CACHE_BLOCK_SIZE 16
#define FILE_NAME "blackscholes_"
#define DRAMsize 0xFFFFFFFF // 4GB DRAM

bool readLineFromFile(std::ifstream& file, std::string& line) {
    if (std::getline(file, line)) {
        return true;  // read success
    }
    return false;  //  EOF or error
}

std::string generateFileName(int index) {
    return "data/" + std::string(FILE_NAME) + std::to_string(index) + ".data";
}

class DRAM {
private:
    uint32_t* memory;   //memory 4bytes 
    size_t size;        //size of dram (# of words)
    // std::mutex mem_mutex;

public:
    DRAM(size_t sizeInWords) : size(sizeInWords){
        memory = new (std::nothrow) uint32_t[size];
        if(!memory){
            throw std::bad_alloc(); //
        }
        std::fill(memory, memory+size ,0);
    }

    ~DRAM() {
        delete[] memory;
    }
    
    void write(size_t address, uint32_t value){ // write to memory
        if (address >= size){
            throw std::out_of_range("out of range of address");
        }
    memory[address] = value;
    }

    uint32_t read(size_t address) const{    //read from memory
        if(address>= size){
            throw std::out_of_range("out of range of address");
        }
        return memory[address];
    }

    size_t getSize() const{ // return size
        return size;
    }

};

class CacheLine {
public:
    bool valid;
    int tag;
    //int data;   do not need for simulation

    CacheLine() : valid(false), tag(-1) {}
};

class Cache {
private:
    int cacheSize;      // Total cache size in bytes
    int blockSize;      // Size of each cache block in bytes
    int associativity;  // Associativity (n-way set-associative)
    int numSets;        // Total number of sets
    std::vector<std::list<CacheLine>> sets;  // List of sets, each containing cache lines

public:
    // Constructor
    Cache(int cacheSize, int blockSize, int associativity)
        : cacheSize(cacheSize), blockSize(blockSize), associativity(associativity) {
        
        // Calculate the number of sets
        numSets = (cacheSize / blockSize) / associativity;

        // Initialize each set with an empty list of cache lines
        sets.resize(numSets);
    }

    // Access method (returns true if hit, false if miss)
    bool access(int address) {
        int blockIndex = (address / blockSize) % numSets;  // Calculate set index
        int tag = address / (blockSize * numSets);          // Calculate tag

        // Get the list representing the set
        auto& set = sets[blockIndex];

        // Check for a cache hit by searching the set for the tag
        for (auto it = set.begin(); it != set.end(); ++it) {
            if (it->valid && it->tag == tag) {
                // On a cache hit, move the cache line to the front (LRU policy)
                set.splice(set.begin(), set, it);
                return true;  // Cache hit
            }
        }

        // Handle a cache miss
        if (set.size() >= associativity) {
            // If the set is full, remove the least recently used (LRU) cache line
            set.pop_back();
        }

        // Add a new cache line with the correct tag
        CacheLine newLine;
        newLine.valid = true;
        newLine.tag = tag;
        set.push_front(newLine);

        return false;  // Cache miss
    }
};

class Bus {
private:
    std::mutex busLock; // Ensures mutual exclusion on the bus

public:
    void requestBus() {
        busLock.lock(); // Acquire the bus
    }

    void releaseBus() {
        busLock.unlock(); // Release the bus
    }
};

class CPU {
private:
    int id;           // CPU ID
    Cache* cache;      // Private cache
    Bus* bus;          // Shared bus
    DRAM* dram;        // Shared DRAM
    int cycles;        // Cycle counter

public:
    CPU(int id, Cache* cache, Bus* bus, DRAM* dram) 
        : id(id), cache(cache), bus(bus), dram(dram), cycles(0) {}

};


int main()
{

// -----------------------READ FILE EXAMPLE -----------------------------
// -----------------------READ FILE EXAMPLE -----------------------------

    std::vector<std::string> filenames;
    std::vector<std::ifstream> files;

    for (int i = 0; i < 4; ++i) {       // put filenaems to filenames[0~3]
        std::string filename = generateFileName(i);
        filenames.push_back(filename);
        std::cout << "Generated filename: " << filename << std::endl;
    
    
        //Open files
        std::ifstream file(filename);
        if (!file.is_open())
        {   
            std::cerr << "can't open file" << filename << std::endl;
            return 1; // stop program
        }
        files.push_back(std::move(file));
    }

    std::cout << "open files succeed" << std::endl;


    bool anyFileHasData = true;
    long count_mem_access[4] = {0};
    while(anyFileHasData)                                               // read data untill no data to read
    {
        anyFileHasData = false;
    std::string line;
    for (size_t i = 0; i < files.size(); ++i) {
        if (std::getline(files[i], line)){                              // read line
            //std::cout << "Files " << i << ": " << line << std::endl;  // print line
            count_mem_access[i]++;
            anyFileHasData = true;
        } else {
            std::cout << "Files" << i << "can not read" << std::endl;
        }

        }
    }
        for ( int i = 0 ; i < 4 ; i ++)
        {
            std::cout << " mem access " << i << " : " << std::dec << count_mem_access[i] << std::endl;
        }
        for( auto& file : files) {
            file.close();
        }
        return 0;
    // --------------------------READ File example ends------------------------- //
    // --------------------------READ File example ends------------------------- //

    Bus bus;
    DRAM dram(DRAMsize);

    Cache cache1(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC); // Cache size, block size, n-way associative
    Cache cache2(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC);
    Cache cache3(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC);
    Cache cache4(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC);

    CPU cpu1(1, &cache1, &bus, &dram);
    CPU cpu2(2, &cache2, &bus, &dram);
    CPU cpu3(3, &cache3, &bus, &dram);
    CPU cpu4(4, &cache4, &bus, &dram);


}

