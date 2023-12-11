//
// Created by artem on 10.12.2023.
//

#include "lamport_logical_time.h"

timestamp_t get_lamport_time(process_content* processContent){
    return processContent->process_lamport_time;
}

void lamport_time_from_message(timestamp_t time_from_msg, process_content* processContent){
    if(time_from_msg > processContent->process_lamport_time){
        processContent->process_lamport_time = time_from_msg;
    }
}

void increase_lamport_time(process_content* processContent){
    ++processContent->process_lamport_time;
}
