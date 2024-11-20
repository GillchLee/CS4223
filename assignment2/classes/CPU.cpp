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
        cache->set_hit_or_not = false;
        cycles = 0;
    }
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
            cache->hit = cache->access(data, bus);
            if(cache->hit) cache_hit++;
            else cache_miss++;
            cache->set_hit_or_not = true;
        }
        if(cache->hit){ // cache hit
            if(cycles == 1){
                on_process = false;
            }
            else{
                cycles ++;
            }
            idleCycles++;
        }
        else {
            //cache miss -> dram access.
            if(bus->currentTransaction != nullptr && bus->currentTransaction->address == input_data && bus->currentTransaction->type == BusTransaction::ReadResponse){
                cache->hit = true;
            }
            idleCycles++;
        }
    }
    total_cycles++;
    return on_process;
}