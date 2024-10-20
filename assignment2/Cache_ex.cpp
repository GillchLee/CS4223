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
#include <sstream>

//#define protocol 0 1 (MESI or DRAGON)

#define CACHE_SIZE 4096 //1024
#define CACHE_ASSOC 2
#define CACHE_BLOCK_SIZE 16
#define FILE_NAME "blackscholes_"
#define DRAMsize 0xFFFFFFFF // 4GB DRAM


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



std::string generateFileName(int index) {
    return "data/" + std::string(FILE_NAME) + std::to_string(index) + ".data";
}

class DRAM {    
private:
    uint8_t* memory;   //memory 1bytes 
    size_t size;        //size of dram (# of words)
    std::mutex mem_mutex;

public:
    DRAM(size_t sizeInBytes) : size(sizeInBytes){
        memory = new (std::nothrow) uint8_t[size];
        if(!memory){
            throw std::bad_alloc(); //
        }
        std::fill(memory, memory+size ,0);
    }

    ~DRAM() {
        delete[] memory;
    }
    
    void write(size_t address, uint8_t value){ // write to memory
        if (address >= size){
            throw std::out_of_range("out of range of address");
        }
    memory[address] = value;
    }

    uint8_t read(size_t address) const{    //read from memory
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
    bool set_hit_or_not =0 ;
    bool hit = 0 ;
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
    bool on_process;    // if CPU is working or not

    int label;
    unsigned int data;
    int target_cycles;



    // benchmark factors
    unsigned long total_cycles;  // idle cycles + target cycles
    unsigned int num_ls;        // # of load/store
    unsigned long compute_cycles; // # of total compute cycles
public:
    CPU(int id, Cache* cache, Bus* bus, DRAM* dram, int cycles, bool on_process, int label, unsigned int data, int target_cycles,unsigned long total_cycles, unsigned int num_ls,unsigned long compute_cycles) 
        : id(id), cache(cache), bus(bus), dram(dram), cycles(0), on_process(0),label(-1), data(0), target_cycles(100), total_cycles(0), num_ls(0), compute_cycles(0){}

    // Function to read operations from a file
    bool Execute( const int input_label, const unsigned int input_data);

    // New function to print the benchmark factors
    void PrintStats() const {
        std::cout << "Total Cycles: " << total_cycles << std::endl;
        std::cout << "Number of Load/Store Operations: " << num_ls << std::endl;
        std::cout << "Compute Cycles: " << compute_cycles << std::endl;
    }
};

bool CPU::Execute( const int input_label, const unsigned int input_data){ // access cache, dram. and compute 
    
    if(!on_process){                // if there's no instruction in CPU, insert label and data to CPU
        label = input_label;
        if(label == 0 || label == 1)  num_ls++;  // for counting # of load/store 
        data  = input_data;
        target_cycles = data;
        on_process = true;
    }

    if(label == 2){ // start computing and hold cycles for "data time". if input_data is 0xc, wait for 12 cycles to compute.
        if( target_cycles == cycles){
            on_process = false;
            compute_cycles += cycles;
            cycles = 0;
        }
        else{
            cycles++;
        }     
    }
    else if(label == 0 || label==1){ //cache access
        
        if( !cache->set_hit_or_not){        // Check if we already accesse to the cache or not
            cache->hit = cache->access(data);
            cache->set_hit_or_not = true;
        }
        
        if( cache->hit){ // cache hit
            if(cycles == 1){
                on_process = false;
                total_cycles += cycles;
                cycles = 0;
                cache->set_hit_or_not = false;
            }
            else{
                cycles ++;
            }
        }
        else{                               //cache miss -> dram access.
            if(cycles == 100){
                on_process = false;
                total_cycles += cycles;
                cycles = 0;
                cache ->set_hit_or_not = false;
            }       
            else{
                cycles++;
            }
        }
        
        
    }
    return on_process;
}

void ThreadExecuteCPU( CPU* CPU, std::ifstream& file){

std::string line;
int label;
unsigned int input_data;
std::pair<int, unsigned int> result;

while ( std::getline(file,line) ) {
        result = readLabelAndData(line);
        label = result.first;
        input_data = result.second;
        while(1){
            if(!CPU->Execute(label,input_data))
                break;
        }
    }
}




int main()
{

    // while ( std::getline(files[0],line) ) {

    //     result = readLabelAndData(line);
    //     label = result.first;
    //     input_data = result.second;
    //     while(1){
    //         if(!cpu1.Execute(label,input_data))
    //             break;
    //     }    
    // }

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
    std::string line;
    int label;
    unsigned int input_data;
    std::pair<int, unsigned int> result;
    int count = 3;
    
    
    Bus bus;
    DRAM dram(DRAMsize);
    Cache cache1(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC); // Cache size, block size, n-way associative
    Cache cache2(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC);
    Cache cache3(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC);
    Cache cache4(CACHE_SIZE, CACHE_BLOCK_SIZE, CACHE_ASSOC);

    
    CPU cpu1(1, &cache1, &bus, &dram, 
             0, false, -1, 0, 100, 
             0, 0, 0);
    CPU cpu2(2, &cache2, &bus, &dram, 
             0, false, -1, 0, 100, 
             0, 0, 0);    
    CPU cpu3(3, &cache3, &bus, &dram, 
             0, false, -1, 0, 100, 
             0, 0, 0);
    CPU cpu4(1, &cache4, &bus, &dram, 
             0, false, -1, 0, 100, 
             0, 0, 0);


    // Launch threads for each CPU
    std::thread t1(ThreadExecuteCPU, &cpu1, std::ref(files[0]));
//   std::thread t2(ThreadExecuteCPU, &cpu2, std::ref(files[1]));
//    std::thread t3(ThreadExecuteCPU, &cpu3, std::ref(files[2]));
//    std::thread t4(ThreadExecuteCPU, &cpu4, std::ref(files[3]));

    // Wait for all threads to complete
    t1.join();
//    t2.join();
//    t3.join();
//    t4.join();



    for( auto& file : files) {
            file.close();
    }

    cpu1.PrintStats();


    return 0;

    // Bus bus;
    // DRAM dram(DRAMsize);


}

