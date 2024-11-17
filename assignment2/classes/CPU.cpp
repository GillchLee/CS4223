//
// Created by Diana on 11/17/2024.
//

#include "CPU.h"

bool CPU::Execute( const int input_label, const unsigned int input_data){ // access cache, dram. and compute
    if(!on_process){                // if there's no instruction in CPU, insert label and data to CPU

        label = input_label;
        data  = input_data;
        target_cycles = data;

        if(label == 0 || label == 1)  num_ls++;  // for counting # of load/store
        total_instructions++;
        on_process = true;
    }
    // else{
        if(label == 2){ // start computing and hold cycles for "data time". if input_data is 0xc, wait for 12 cycles to compute.
            if( target_cycles == cycles){
                total_instructions += cycles-1;
                compute_cycles += cycles;
                on_process = false;
                cycles = 0;
            }
            else{
                cycles++;
            }
        }
        else if(label == 0 || label==1){ //cache access
            if( !cache->set_hit_or_not){        // Check if we already accessed to the cache or not
                //TODO: Make it so that when there is cache eviction, this will stall until done driving the bus with the writeback.
                cache->hit = cache->access(data);
                if(cache->hit) cache_hit++;
                else cache_miss++;
                cache->set_hit_or_not = true;
            }
            if(cache->hit){ // cache hit
                if(cycles == 1){
                    on_process = false;
                    // total_cycles += cycles;
                    idleCycles++;
                    cycles = 0;
                    cache->set_hit_or_not = false;
                }
                else{
                    cycles ++;
                }
            }
            else {
                //cache miss -> dram access.
                //TODO: change this so that it just attempts to drive the bus and send a read request.
                // the DRAM will then wait the 100 cycles then send back the data

                const int DRAM_PENALTY = 100;
                const int CACHE_HIT_PENALTY = 1;
                const int WORD_SIZE = 4;
                const int BANDWIDTH_PENALTY = 2 * (blockSize / WORD_SIZE);
                if(cycles == (DRAM_PENALTY + CACHE_HIT_PENALTY + BANDWIDTH_PENALTY)){
                    busTraffic += (BANDWIDTH_PENALTY/2);
                    on_process = false;
                    // total_cycles += cycles;
                    cycles = 0;
                    cache ->set_hit_or_not = false;
                }
                else{
                    idleCycles++;
                    cycles++;
                }
            }
        }
    // }
    return on_process;
}