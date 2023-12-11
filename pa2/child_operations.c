//
// Created by artem on 07.12.2023.
//
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "child_operations.h"
#include "ipc.h"
#include "pa1.h"
#include "general_actions.h"
#include "log.h"
#include "lamport_logical_time.h"

void send_recieve_STARTED_message(process_content* processContent) {
    Message msg;
    memset(&msg,0,sizeof(msg));
    MessageHeader msg_header;
    increase_lamport_time(processContent);
    msg_header.s_magic = MESSAGE_MAGIC;
    msg_header.s_type = STARTED;
    msg_header.s_local_time = processContent->process_lamport_time;
    int len = sprintf(msg.s_payload, log_started_fmt, processContent->this_process, getpid(), getppid());
    msg_header.s_payload_len = len + 1;
    msg.s_header = msg_header;
    if(send_multicast(processContent, &msg) != 0) { // add logging
        printf("error send started message");
    }
    logging_process_started(processContent->process_lamport_time
                            , processContent->this_process
                            , processContent->process_balance);  // logger
    recieve_messages_from_other_processes(processContent, STARTED);
//    logging_received_all_started_messages(processContent->process_lamport_time, processContent->this_process);  // logger
}

void save_balance_state(process_content* processContent, timestamp_t operation_time, balance_t pending_balance_in){
    uint8_t history_len = processContent->balanceHistory->s_history_len;
    if( history_len > 0){
        BalanceState last_known_state = processContent->balanceHistory->s_history[history_len - 1];
        if(processContent->process_lamport_time == operation_time){
            return;
        }
        for(timestamp_t time = last_known_state.s_time + 1; time < operation_time; ++time){
            BalanceState intermediate_state;
            intermediate_state.s_balance = last_known_state.s_balance;
            intermediate_state.s_balance_pending_in = last_known_state.s_balance_pending_in; // обратить внимание
            intermediate_state.s_time = time;
            processContent->balanceHistory->s_history[history_len] = intermediate_state;
            ++history_len;
            processContent->balanceHistory->s_history_len = history_len;
        }
    }
    BalanceState updated_state;
    updated_state.s_balance = processContent->process_balance;
    updated_state.s_balance_pending_in = pending_balance_in;
    updated_state.s_time = operation_time;
    processContent->balanceHistory->s_history[processContent->balanceHistory->s_history_len] = updated_state;
    ++processContent->balanceHistory->s_history_len;
}

void send_DONE_message(process_content* processContent) {
    Message msg;
    memset(&msg,0,sizeof(msg));
    MessageHeader msg_header;
    msg_header.s_magic = MESSAGE_MAGIC;
    msg_header.s_type = DONE;
    msg_header.s_local_time = 0;
    int len = sprintf(msg.s_payload, log_done_fmt, processContent->this_process);
    msg_header.s_payload_len = len + 1;
    msg.s_header = msg_header;
    if(send_multicast(processContent, &msg) != 0) { // add logging
        printf("error send done message");
    }
    logging_process_done(processContent->process_lamport_time
                        , processContent->this_process
                        , processContent->process_balance);  // logger
    //recieve_messages_from_other_processes(processContent, DONE);
    //logging_received_all_done_messages(processContent->process_lamport_time, processContent->this_process);  // logger
}

void process_transfer_queries(process_content* processContent){
    int permission_to_work = 1;
    int num_DONE_process = 0;
    int num_C_processes = processContent->process_num - 2;
    while(permission_to_work || num_DONE_process < num_C_processes){
        Message msg;
        while(receive_any(processContent, &msg));
        switch (msg.s_header.s_type) {
            case STOP:
            {
                permission_to_work = 0;
                send_DONE_message(processContent);
                break;
            }
            case DONE:
            {
                num_DONE_process++;
                break;
            }
            case TRANSFER:
            {
                TransferOrder recieved_order;
                memcpy(&recieved_order, msg.s_payload, msg.s_header.s_payload_len);
                if(processContent->this_process == recieved_order.s_src) {
                    increase_lamport_time(processContent);
                    processContent->process_balance -= recieved_order.s_amount;
                    save_balance_state(processContent, processContent->process_lamport_time, recieved_order.s_amount);
                    msg.s_header.s_local_time = processContent->process_lamport_time;
                    while(send(processContent, recieved_order.s_dst, &msg) > 0);
                }
                if(processContent->this_process == recieved_order.s_dst) {
                    processContent->process_balance += recieved_order.s_amount;
                    save_balance_state(processContent, processContent->process_lamport_time, 0);
                    Message ack_message;
                    MessageHeader messageHeader;
                    messageHeader.s_magic = MESSAGE_MAGIC;
                    messageHeader.s_type = ACK;
                    messageHeader.s_payload_len = 0;
                    increase_lamport_time(processContent);
                    messageHeader.s_local_time = get_lamport_time(processContent);
                    ack_message.s_header = messageHeader;
                    while (send(processContent, PARENT_ID, &ack_message) > 0);
                }
            }
            default:
                break;
        }

    }
    logging_received_all_done_messages(processContent->process_lamport_time, processContent->this_process);
    unsigned int  message_length = processContent->balanceHistory->s_history_len * sizeof(BalanceState); // carefully check
    Message history_message;
    MessageHeader history_message_header;
    history_message_header.s_magic = MESSAGE_MAGIC;
    history_message_header.s_type = BALANCE_HISTORY;
    increase_lamport_time(processContent);
    history_message_header.s_local_time = processContent->process_lamport_time;
    history_message_header.s_payload_len = message_length;
    history_message.s_header = history_message_header;

    memcpy(&history_message, processContent->balanceHistory->s_history, message_length);
    while(send(processContent, PARENT_ID, &history_message) > 0);
}
