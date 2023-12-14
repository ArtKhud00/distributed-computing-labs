//
// Created by artem on 14.12.2023.
//

#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "child_operations.h"
#include "ipc.h"
#include "pa2345.h"
#include "log.h"
#include "lamport_logical_time.h"

typedef struct{
    process_content * procContent;
    int* num_done_process;
} Interaction_struct;


void send_STARTED_message(process_content* processContent) {
    Message msg;
    memset(&msg,0,sizeof(msg));
    MessageHeader msg_header;
    msg_header.s_magic = MESSAGE_MAGIC;
    msg_header.s_type = STARTED;
    msg_header.s_local_time = increase_lamport_time_and_get_it();
    int len = sprintf(msg.s_payload, log_started_fmt, get_lamport_time_value()
            , processContent->this_process, getpid(), getppid(), 0);
    msg_header.s_payload_len = len + 1;
    msg.s_header = msg_header;
    if(send_multicast(processContent, &msg) != 0) { // add logging
        printf("error send started message");
    }
    logging_process_started(get_lamport_time_value()
            , processContent->this_process
            , 0);  // logger
}

void add_request_to_queue(ProcessQueue *queue, Pairs time_pid) {
    if (queue->len == 0) {
        queue->processes[queue->len++] = time_pid;
        return;
    }
    queue->len++;
    //assert(queue->length <= MAX_PROCESS_ID);
    for (int i = 0; i < queue->len - 1; i++) {
        Pairs compared_pairs = queue->processes[i];
        int request_have_smaller_time = time_pid.lamport_time < compared_pairs.lamport_time;
        int time_is_equal_but_smaller_id = time_pid.lamport_time == compared_pairs.lamport_time && time_pid.process_id < compared_pairs.process_id;
        if (request_have_smaller_time || time_is_equal_but_smaller_id) {
            for (int j = queue->len - 2; j >= i; j--) {
                queue->processes[j + 1] = queue->processes[j];
            }
            queue->processes[i] = time_pid;
            return;
        }
    }
    queue->processes[queue->len - 1] = time_pid;
}

void remove_first_request_from_queue(ProcessQueue *queue) {
    for (int i = 0; i < queue->len - 1; i++) {
        queue->processes[i] = queue->processes[i + 1];
    }
    queue->len = queue->len - 1;
}

void processes_interaction(process_content* processContent, Message* msg, int* proc_done_count){
    get_lamport_time_from_message(msg->s_header.s_local_time);
    switch (msg->s_header.s_type) {
        case CS_REQUEST: {
            // add to queue
            Message reply_msg;
            MessageHeader reply_msg_header;
            reply_msg_header.s_magic = MESSAGE_MAGIC;
            reply_msg_header.s_type = CS_REPLY;
            reply_msg_header.s_local_time = increase_lamport_time_and_get_it();
            reply_msg_header.s_payload_len = 0;
            reply_msg.s_header = reply_msg_header;
            Pairs time_and_pid;
            memcpy(&time_and_pid, msg->s_payload, msg->s_header.s_payload_len);
            add_request_to_queue(&processContent->process_queue,time_and_pid);
            send(processContent, time_and_pid.process_id, &reply_msg);
            break;
        }
        case CS_RELEASE: {
            remove_first_request_from_queue(&processContent->process_queue);
            // clear queue
            break;
        }
        case DONE: {
            //*proc_done_count++;
            *proc_done_count = *proc_done_count + 1;
            break;
        }
    }
}

void send_to_other_childs(process_content* processContent, Message *msg){
    for(int id = 1; id < processContent->process_num; ++id){
        if(id == processContent->this_process){
            continue;
        }
        send(processContent, id, msg);
    }
}

//void request_access_to_cs(process_content* processContent, int* proc_done_count){
int request_cs(const void * self){
    //process_content* processContent = (process_content*) self;
    Interaction_struct * interactionStruct = (Interaction_struct*) self;
    Pairs time_and_pid;
    timestamp_t lamp_time = increase_lamport_time_and_get_it();
    time_and_pid.lamport_time = lamp_time;
    time_and_pid.process_id = interactionStruct->procContent->this_process;

    Message request_msg;
    MessageHeader request_msg_header;
    request_msg_header.s_magic = MESSAGE_MAGIC;
    request_msg_header.s_type = CS_REQUEST;
    request_msg_header.s_local_time = lamp_time;
    request_msg_header.s_payload_len = sizeof(time_and_pid);
    request_msg.s_header = request_msg_header;
    memcpy(request_msg.s_payload, &time_and_pid, sizeof(time_and_pid));
    add_request_to_queue(&interactionStruct->procContent->process_queue, time_and_pid);
    send_to_other_childs(interactionStruct->procContent, &request_msg);

    uint8_t number_child = interactionStruct->procContent->process_num - 2;
    uint8_t replies_count = 0;
    while(replies_count < number_child){
        //printf("Waiting for reqs, proc %d, time %d\n", interactionStruct->procContent->this_process, get_lamport_time_value());
        Message msg;
        receive_any(interactionStruct->procContent, &msg);
        if(msg.s_header.s_type == CS_REPLY) {
            get_lamport_time_from_message(msg.s_header.s_local_time);
            ++replies_count;
        }
        else {
            processes_interaction(interactionStruct->procContent, &msg, interactionStruct->num_done_process);
        }
    }
    return 0;
}

int release_cs(const void* self){
    Interaction_struct * interactionStruct = (Interaction_struct*) self;
    // delete first element from queue
    Message release_msg;
    MessageHeader release_msg_header;
    release_msg_header.s_local_time = increase_lamport_time_and_get_it();
    release_msg_header.s_magic = MESSAGE_MAGIC;
    release_msg_header.s_type = CS_RELEASE;
    release_msg_header.s_payload_len = 0;
    release_msg.s_header = release_msg_header;
    remove_first_request_from_queue(&interactionStruct->procContent->process_queue);
    send_to_other_childs(interactionStruct->procContent, &release_msg);
    return 0;
}

//void process_queries(process_content* processContent){
//    int num_done_messages = 0;
//    uint8_t number_child = processContent->process_num - 2;
//    for(int i = 1; i <= processContent->print_iterations; ++i){
//        char loop_operation[70] = {0};
//        sprintf(loop_operation, log_loop_operation_fmt, processContent->this_process, i, processContent->print_iterations);
//        print(loop_operation);
//        logging_loop_iteration(processContent->this_process, i, processContent->print_iterations);
//    }
//    send_DONE_message(processContent);
//    logging_process_done(get_lamport_time_value(), processContent->this_process, 0);
//    while(num_done_messages < number_child){
//        Message msg;
//        receive_any(processContent, &msg);
//        get_lamport_time_from_message(msg.s_header.s_local_time);
//        if(msg.s_header.s_type == DONE){
//            ++num_done_messages;
//        }
//    }
//    logging_received_all_done_messages(get_lamport_time_value(), processContent->this_process);
//}

void process_queries(process_content* processContent, int using_mutex){
    int num_done_messages = 0;
    uint8_t number_child = processContent->process_num - 2;
    for(int i = 1; i <= processContent->print_iterations; ++i){
        if(using_mutex){
            //printf("_Before request_cs, process %d, time %d\n", processContent->this_process, get_lamport_time_value());
            Interaction_struct int_struct;
            int_struct.procContent = processContent;
            int_struct.num_done_process = &num_done_messages;
            request_cs(&int_struct);
        }
        printf("Enter CS, process %d, time %d\n", processContent->this_process, get_lamport_time_value());
        char loop_operation[70] = {0};
        sprintf(loop_operation, log_loop_operation_fmt, processContent->this_process, i, processContent->print_iterations);
        print(loop_operation);
        logging_loop_iteration(processContent->this_process, i, processContent->print_iterations);
        if(using_mutex){
            //printf("_Before release_cs, process %d, time %d\n", processContent->this_process, get_lamport_time_value());
            Interaction_struct int_struct;
            int_struct.procContent = processContent;
            int_struct.num_done_process = &num_done_messages;
            release_cs(&int_struct);
        }
    }
    send_DONE_message(processContent);
    logging_process_done(get_lamport_time_value(), processContent->this_process, 0);
    while(num_done_messages < number_child){
        Message msg;
        receive_any(processContent, &msg);
        processes_interaction(processContent, &msg, &num_done_messages);
//        get_lamport_time_from_message(msg.s_header.s_local_time);
//        if(msg.s_header.s_type == DONE){
//            ++num_done_messages;
//        }
    }
    logging_received_all_done_messages(get_lamport_time_value(), processContent->this_process);
}

//void process_queries_cs(process_content* processContent){
//    int num_done_messages = 0;
//    uint8_t number_child = processContent->process_num - 2;
//    int cycle_iter = 1;
//    int request_sent = 0;
//    int done_sent = 0;
//    int replies_count = 0;
//    while(cycle_iter < processContent->print_iterations || num_done_messages < number_child){
//        if(done_sent == 0 && cycle_iter == processContent->print_iterations){
//            send_DONE_message(processContent);
//            logging_process_done(get_lamport_time_value(), processContent->this_process, 0);
//            done_sent = 1;
//            request_sent = 0;
//        }
//        if(request_sent == 0 && cycle_iter < processContent->print_iterations){
//
//            request_sent = 1;
//        }
//    }
//    logging_received_all_done_messages(get_lamport_time_value(), processContent->this_process);
//}

void send_DONE_message(process_content* processContent) {
    Message msg;
    memset(&msg,0,sizeof(msg));
    MessageHeader msg_header;
    msg_header.s_magic = MESSAGE_MAGIC;
    msg_header.s_type = DONE;
    msg_header.s_local_time = increase_lamport_time_and_get_it();
    int len = sprintf(msg.s_payload, log_done_fmt, get_lamport_time_value(),
                      processContent->this_process, 0);
    msg_header.s_payload_len = len + 1;
    msg.s_header = msg_header;
    if(send_multicast(processContent, &msg) != 0) { // add logging
        printf("error send done message");
    }
    logging_process_done(get_lamport_time_value()
            , processContent->this_process
            , 0);  // logger
}
