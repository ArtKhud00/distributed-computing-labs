//
// Created by artem on 07.12.2023.
//

#include "general_actions.h"
#include "ipc.h"
#include "log.h"
#include <stdio.h>

// for started and done messages
void recieve_messages_from_other_processes(process_content* processContent, MessageType type){
    for(uint8_t process_id = 1; process_id < processContent->process_num; ++process_id){
        if(process_id == processContent->this_process){
            continue;
        }
        Message recieved_msg;
        while(receive(processContent, process_id, &recieved_msg)>0);
//        if(receive(processContent, process_id, &recieved_msg) == 0){
//            if(recieved_msg.s_header.s_magic != MESSAGE_MAGIC){
//                //return NOT_MAGIC_SIGN;
//                printf("Message does not have magic sign\n");
//            }
//            if(recieved_msg.s_header.s_type != type){
//                printf("Message does not have specified type\n");
//                //return NOT_VALID_TYPE;
//            }
//            lamport_time_from_message(recieved_msg.s_header.s_local_time, processContent);
//            increase_lamport_time(processContent);
//        }
//        else{
//            //return RECIEVE_UNSUCCESFUL;
//            printf("Could not recieve message\n");
//        }
    }
    if(type == STARTED){
        logging_received_all_started_messages(processContent->process_lamport_time, processContent->this_process);
    }
    else if (type == DONE){
        logging_received_all_done_messages(processContent->process_lamport_time, processContent->this_process);
    }
}
