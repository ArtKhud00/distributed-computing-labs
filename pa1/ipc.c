//
// Created by artem on 05.12.2023.
//
#include <unistd.h>
#include "ipc.h"
#include "constants.h"

int send(void * self, local_id dst, const Message * msg){
    //local_id* this_process = (local_id*) self;
    process_content* processContent = (process_content*) self;
//    if(msg->s_header.s_magic != MESSAGE_MAGIC){
//        return 1; // change to CONSTANT!!!
//    }
    int bytes_written = write(processContent->write_pipes[processContent->this_process][dst], msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
    return (bytes_written >= 0) ? 0 : 1;
}

int send_multicast(void * self, const Message * msg){
    //local_id* this_process = (local_id*) self;
    process_content* processContent = (process_content*) self;
    for(local_id process_id = 0; process_id != processContent->process_num; ++process_id){
        if(process_id == processContent->this_process){
            continue;
        }
        if(send(self, process_id, msg)){
            return 1;
        }
    }
    return 0;
}

int receive(void * self, local_id from, Message * msg){
    process_content* processContent = (process_content*) self;
    int bytes_read = read(processContent->read_pipes[from][processContent->this_process],&msg->s_header, sizeof(msg->s_header));
    if(bytes_read < 0){
        return 1;
    }
    if(msg->s_header.s_magic != MESSAGE_MAGIC){
        return 1;
    }
    bytes_read = read(processContent->read_pipes[from][processContent->this_process],&msg->s_payload, msg->s_header.s_payload_len);
    if(bytes_read < 0){
        return 1;
    }
    return 0;
}

int receive_any(void * self, Message * msg){
    //local_id* this_process = (local_id*) self;
    process_content* processContent = (process_content*) self;
    for(local_id process_id = 1; process_id != MAX_PROCESS_NUM; ++process_id){
        if(process_id == processContent->this_process){
            continue;
        }
        if(receive(self, process_id, msg)){
            return 1;
        }
    }
    return 0;
}
