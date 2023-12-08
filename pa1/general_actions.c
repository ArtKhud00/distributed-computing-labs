//
// Created by artem on 07.12.2023.
//

#include "general_actions.h"
#include "ipc.h"
#include <stdio.h>
#include <unistd.h>

void recieve_messages_from_other_processes(process_content* processContent, MessageType type){
    for(uint8_t process_id = 1; process_id < processContent->process_num; ++process_id){
        if(process_id == processContent->this_process){
            continue;
        }
        Message recieved_msg;
        if(receive(processContent, process_id, &recieved_msg) != 0){
            if(type == STARTED){
                printf("error get started message from %d process", process_id);
            }
            if(type == DONE){
                printf("error get done message from %d process", process_id);
            }
        }
        if(recieved_msg.s_header.s_magic != MESSAGE_MAGIC){
            printf("error, message from %d process is not magic", process_id);
        }
        if(recieved_msg.s_header.s_type != type){
            if(type == STARTED)
                printf("error, message from %d process in process %d is not STARTED\n", process_id, getpid());
            if(type == DONE)
                printf(" ZDES error, message from %d process in process %d is not DONE\n", process_id, getpid());
        }
    }
}
