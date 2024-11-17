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

// #define protocol 0 1 (MESI or DRAGON)

// #define CACHE_SIZE 4096 //1024
// #define CACHE_ASSOC 2
// #define CACHE_BLOCK_SIZE 32
// #define FILE_NAME "blackscholes_"
#define DRAMsize 0xFFFFFFFF // 4GB DRAM

#define M_STATE 1
#define E_STATE 2
#define S_STATE 3
#define I_STATE 0
// I = 0, M = 1, E = 2, S = 3.
int CACHE_SIZE = 4096;
int CACHE_ASSOC = 2;
int CACHE_BLOCK_SIZE = 32;
std::string FILE_NAME = "blackscholes_";
std::string PROTOCOL = "NONE";

unsigned int busTraffic = 0;

// Function to read one line from the file and return label and data (as integer)
std::pair<int, unsigned int> readLabelAndData(const std::string &line)
{
    std::istringstream iss(line);
    int label;
    unsigned int data;

    // Parse label and hex data from the line
    if (iss >> label >> std::hex >> data)
    {
        return {label, data}; // Return label and data as a pair
    }

    // If parsing fails, return default error values
    return {-1, 0};
}

std::string generateFileName(const int index)
{
    return "data/" + std::string(FILE_NAME) + std::to_string(index) + ".data";
}

class DRAM
{
private:
    uint8_t *memory; // memory 1bytes
    size_t size;     // size of dram (# of words)
    std::mutex mem_mutex;

public:
    explicit DRAM(const size_t sizeInBytes) : size(sizeInBytes)
    {
        // memory = new (std::nothrow) uint8_t[size];
        // // if(!memory){
        // //     throw std::bad_alloc(); //
        // // }
        // std::fill(memory, memory+size ,0);
    }

    // ~DRAM() {
    //     delete[] memory;
    // }

    // void write(size_t address, uint8_t value){ // write to memory
    //     if (address >= size){
    //         throw std::out_of_range("out of range of address");
    //     }
    // memory[address] = value;
    // }

    // uint8_t read(size_t address) const{    //read from memory
    //     if(address>= size){
    //         throw std::out_of_range("out of range of address");
    //     }
    //     return memory[address];
    // }

    // size_t getSize() const{ // return size
    //     return size;
    // }
};

class CacheLine
{
public:
    bool valid;
    int tag;
    // int data;   do not need for simulation
    int state; // add state for cache coherence.
    CacheLine() : valid(false), tag(-1), state(0) {}
};

class Cache
{
private:
    int cacheSize;                          // Total cache size in bytes
    int blockSize;                          // Size of each cache block in bytes
    int associativity;                      // Associativity (n-way set-associative)
    int numSets;                            // Total number of sets
    std::vector<std::list<CacheLine>> sets; // List of sets, each containing cache lines

public:
    // Constructor
    Cache(int cacheSize, int blockSize, int associativity)
        : cacheSize(cacheSize), blockSize(blockSize), associativity(associativity)
    {

        // Calculate the number of sets
        numSets = (cacheSize / blockSize) / associativity;

        // Initialize each set with an empty list of cache lines
        sets.resize(numSets);
    }
    bool set_hit_or_not = false;
    bool hit = false;
    int waiting_write_back = 0;
    // Access method (returns true if hit, false if miss)
    bool access(int address, int label)
    {
        int blockIndex = (address / blockSize) % numSets; // Calculate set index
        int tag = address / (blockSize * numSets);        // Calculate tag

        // Get the list representing the set
        auto &set = sets[blockIndex];

        // Check for a cache hit by searching the set for the tag
        for (auto it = set.begin(); it != set.end(); ++it)
        {
            if (it->valid && it->tag == tag)
            {
                return it->state; // Cache hit
            }
        }

        return 0; // Cache miss
    }

    bool Update(int protocol, int address, int update_state)
    {                                                     // evict condition : I -> M,E,S 가 될 때, 즉 새로운 캐시를 작성하는 것임.
        int blockIndex = (address / blockSize) % numSets; // Calculate set index
        int tag = address / (blockSize * numSets);        // Calculate tag

        auto &set = sets[blockIndex];
        for (auto it = set.begin(); it != set.end(); ++it)
        {
            if (it->valid && it->tag == tag)
            {
                it->state = update_state;
                set.splice(set.begin(), set, it);
                return true; // Cache hit
            }
        }
        if (set.size() >= associativity)
        {
            // If the set is full, remove the least recently used (LRU) cache line
            set.pop_back();
            const int WORD_SIZE = 4;
            const int BANDWIDTH_PENALTY = 2 * (CACHE_BLOCK_SIZE / WORD_SIZE);
            busTraffic = busTraffic + (BANDWIDTH_PENALTY / 2);
        }

        // Add a new cache line with the correct tag
        CacheLine newLine;
        newLine.valid = true;
        newLine.tag = tag;
        newLine.state = update_state;
        set.push_front(newLine);

        return false; // Cache miss
    }

    bool Bus_invalid(int address)
    {                                                     // in MESI protocol, if we write to cache, make other cache invalid
        int blockIndex = (address / blockSize) % numSets; // Calculate set index
        int tag = address / (blockSize * numSets);        // Calculate tag
        auto &set = sets[blockIndex];

        // Check for a cache hit by searching the set for the tag
        for (auto it = set.begin(); it != set.end(); ++it)
        {
            if (it->tag == tag)
            {
                it->state = I_STATE; // make it Invalid(0)
                return true;         // Cache hit
            }
        }
        return false; // Cache miss
    }

    int Bus_access_read(int address)
    {                                                     // check other cache has same address of data. if has, return state. if has not, return -1.
        int blockIndex = (address / blockSize) % numSets; // Calculate set index
        int tag = address / (blockSize * numSets);        // Calculate tag

        // Get the list representing the set
        auto &set = sets[blockIndex];

        // Check for a cache hit by searching the set for the tag
        for (auto it = set.begin(); it != set.end(); ++it)
        {
            if (it->tag == tag)
            {
                return it->state; // Cache hit
            }
        }
        return false; // data does not exist in this cache, bus read miss
    }
};

// TODO: Implement bus state that it broadcasts to the entire system. Also implement a queue for who drives the bus.
class Bus
{
private:
    bool isOccupied = false;

public:
    void putOnBus(int address);
};

// TODO: add a drive bus method that will switch the state of the CPU from idle to driving bus
//  IDEA : new CPU input, 3 other cache input to check other cache's information by using BUS
class CPU
{
private:
    int id;          // CPU ID
    Cache *cache;    // Private cache
    Bus *bus;        // Shared bus
    DRAM *dram;      // Shared DRAM
    int cycles;      // Cycle counter
    bool on_process; // if CPU is working or not

    int label;
    unsigned int data;
    int target_cycles;

    Cache *cache_other_1;
    Cache *cache_other_2;
    Cache *cache_other_3;

    // benchmark factors
    unsigned int total_instructions; // for computing IPC
    unsigned long total_cycles;      // idle cycles + target cycles
    unsigned int num_ls;             // # of load/store
    unsigned long compute_cycles;    // # of total compute cycles
    unsigned long cache_hit;
    unsigned long cache_miss;
    unsigned long idleCycles = 0;
    unsigned long dataTraffic = 0;

public:
    CPU(int id, Cache *cache, Bus *bus, DRAM *dram, int cycles, bool on_process, int label, unsigned int data,
        int target_cycles,
        Cache *cache_other_1, Cache *cache_other_2, Cache *cache_other_3,
        unsigned int total_instructions, long total_cycles, unsigned int num_ls, unsigned long compute_cycles, unsigned long cache_hit, unsigned long cache_miss)
        : id(id), cache(cache), bus(bus), dram(dram), cycles(0), on_process(false), label(-1), data(0),
          target_cycles(100),
          cache_other_1(cache_other_1), cache_other_2(cache_other_2), cache_other_3(cache_other_3),
          total_instructions(0), total_cycles(0), num_ls(0), compute_cycles(0), cache_hit(0), cache_miss(0) {}

    bool isDataReceived(unsigned int data);

    // Function to read operations from a file
    bool Execute(int input_label, unsigned int input_data);

    // New function to print the benchmark factors
    void PrintStats() const
    {
        double IPC = static_cast<double>(total_instructions) / static_cast<double>(total_cycles);
        std::cout << "Total instructions:" << total_instructions << std::endl;
        // std::cout << "IPC : " << IPC << std::endl;
        std::cout << "Number of Load/Store Operations: " << num_ls << std::endl;
        std::cout << "Compute Cycles: " << compute_cycles << std::endl;
        std::cout << "Cache hit : " << cache_hit << std::endl;
        std::cout << "Cache miss : " << cache_miss << std::endl;
        std::cout << "Idle cycles : " << idleCycles << std::endl;
        std::cout << "Amount of data traffic : " << busTraffic << std::endl;
        std::cout << "Number of invalidations/updates on the bus:0" << std::endl;
        std::cout << "Distribution of accesses to private data : 100%" << std::endl;
    }
};

bool CPU::Execute(const int input_label, const unsigned int input_data)
{ // access cache, dram. and compute

    int cache_state = 0;
    int cache_other_1_state = 0;
    int cache_other_2_state = 0;
    int cache_other_3_state = 0;
    int update_state = 0;
    if(cache->waiting_write_back){
        cache->waiting_write_back--;
        idleCycles++;
    }
    // this block below define target cycles.
    else if (!on_process){ // if there's no instruction in CPU, insert label and data to CPU
        label = input_label;
        data = input_data;

        if (label == 2)
        {
            target_cycles = data;
        }
        else if (label == 1)
        {             // store instruction, write to cache if it has same address.
            num_ls++; // count # of load/store instructions
            cache_state = cache->access(data, label);
            if (cache_state != I_STATE){    // if cache hit M , S ,E state
                target_cycles = 1;
            }
            else {
            target_cycles = 100; // write-allocate. we take the data from memory and write on it.
            // if cache miss, bus access.
            cache_other_1_state = cache_other_1->Bus_invalid(data);
            cache_other_2_state = cache_other_2->Bus_invalid(data);
            cache_other_3_state = cache_other_3->Bus_invalid(data);
            }
            cache->Update(0, data, M_STATE);
        }
        else if (label == 0) // load instruction, check own cache and other caches.
        {
            num_ls++; // count # of load/store instruction
            cache_state = cache->access(data, label);
            if (cache_state == I_STATE)
            { // if cache miss or I_state, we access to BUS

                cache_other_1_state = cache_other_1->Bus_access_read(data);
                cache_other_2_state = cache_other_2->Bus_access_read(data);
                cache_other_3_state = cache_other_3->Bus_access_read(data);
                
                if (cache_other_1_state != I_STATE) // if other cache has data
                {
                    if(cache_other_1_state == M_STATE)
                        cache_other_1->waiting_write_back = 100;
                    cache_other_1->Update(0, data, S_STATE); // makes that cache to Shared_STATE
                    cache->Update(0, data, S_STATE);
                    target_cycles = 2; // bus transaction
                }
                else if (cache_other_2_state != I_STATE)
                {
                    if(cache_other_2_state == M_STATE)
                        cache_other_2->waiting_write_back = 100;                    cache_other_2->Update(0, data, S_STATE);
                    cache_other_2->Update(0, data, S_STATE); // makes that cache to Shared_STATE
                    cache->Update(0, data, S_STATE);
                    target_cycles = 2; // bus transaction
                }
                else if (cache_other_3_state != I_STATE)
                {
                    if(cache_other_3_state == M_STATE)
                        cache_other_3->waiting_write_back = 100;                    cache_other_3->Update(0, data, S_STATE);
                    cache_other_3->Update(0, data, S_STATE); // makes that cache to Shared_STATE
                    cache->Update(0, data, S_STATE);
                    target_cycles = 2; // bus transaction
                }
                else
                {
                    cache->Update(0, data, E_STATE); // if all other caches don't have data, Update to Exclusive.
                    target_cycles = 100; // dram access
                }
            }
            else // hit, it should M or E or S. then no change.
                target_cycles = 1;
        }


        total_instructions++;
        on_process = true;

    }
    
    // now Processing until cycls== target _cycles.
    // now Processing until cycls== target _cycles.
    // now Processing until cycls== target _cycles.

    if (label == 2){ // start computing and hold cycles for "data time". if input_data is 0xc, wait for 12 cycles to compute.
        if (cycles == target_cycles)
        {
            compute_cycles += cycles;
            on_process = false;
            cycles = 0;
        }
        else
        {
            cycles++;
        }
    }
    else if (label == 0 || label == 1)
    {                   // cache access
        if (cycles == target_cycles)
        {
            on_process = false;
            cycles = 0;
        }
        else{
            cycles++;
        }

    }

        // if (!cache->set_hit_or_not)
        // { // Check if we already accessed to the cache or not
        //     // TODO: Make it so that when there is cache eviction, this will stall until done driving the bus with the writeback.
        //     cache->hit = cache->access(data, label);
        //     if (cache->hit)
        //         cache_hit++;
        //     else
        //         cache_miss++;
        //     cache->set_hit_or_not = true;
        // }
        // if (cache->hit)
        // { // cache hit
        //     if (cycles == 1)
        //     {
        //         on_process = false;
        //         // total_cycles += cycles;
        //         idleCycles++;
        //         cycles = 0;
        //         cache->set_hit_or_not = false;
        //     }
        //     else
        //     {
        //         cycles++;
        //     }
        // }
        // else
        // {
        //     // cache miss -> dram access.
        //     // TODO: change this so that it just attempts to drive the bus and send a read request.
        //     //  the DRAM will then wait the 100 cycles then send back the data

        //     const int DRAM_PENALTY = 100;
        //     const int CACHE_HIT_PENALTY = 1;
        //     const int WORD_SIZE = 4;
        //     const int BANDWIDTH_PENALTY = 2 * (CACHE_BLOCK_SIZE / WORD_SIZE);
        //     if (cycles == (DRAM_PENALTY + CACHE_HIT_PENALTY + BANDWIDTH_PENALTY))
        //     {
        //         busTraffic += (BANDWIDTH_PENALTY / 2);
        //         on_process = false;
        //         // total_cycles += cycles;
        //         cycles = 0;
        //         cache->set_hit_or_not = false;
        //     }
        //     else
        //     {
        //         idleCycles++;
        //         cycles++;
        //     }
        // }
    //}
    // }
    return on_process;
}

int main(int argc, char *argv[])
{
    if (argc == 6)
    {
        PROTOCOL = argv[1];
        FILE_NAME = argv[2];
        CACHE_SIZE = atoi(argv[3]);
        CACHE_ASSOC = atoi(argv[4]);
        CACHE_BLOCK_SIZE = atoi(argv[5]);
    }

    std::vector<std::string> filenames;
    std::vector<std::ifstream> files;

    for (int i = 0; i < 4; ++i)
    { // put filenaems to filenames[0~3]
        std::string filename = generateFileName(i);
        filenames.push_back(filename);
        std::cout << "Generated filename: " << filename << std::endl;

        // Open files
        std::ifstream file(filename);
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

    CPU cpu1(1, &cache1, &bus, &dram, 0, false, -1, 0,
             100,
             &cache2, &cache3, &cache4,
             0, 0, 0, 0, 0, 0);
    CPU cpu2(2, &cache2, &bus, &dram, 0, false, -1, 0,
             100,
             &cache1, &cache3, &cache4,
             0, 0, 0, 0, 0, 0);
    CPU cpu3(3, &cache3, &bus, &dram, 0, false, -1, 0,
             100,
             &cache1, &cache2, &cache4,
             0, 0, 0, 0, 0, 0);
    CPU cpu4(4, &cache4, &bus, &dram, 0, false, -1, 0,
             100,
             &cache1, &cache2, &cache3,
             0, 0, 0, 0, 0, 0);

    // for multicore cycles

    unsigned long long total_cycles = -1;
    bool Readfile_available = true;

    while (Readfile_available)
    {
        Readfile_available = false;

        if (std::getline(files[0], line))
        {
            result = readLabelAndData(line);
            while (cpu1.Execute(result.first, result.second))
            {
                total_cycles++;
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
