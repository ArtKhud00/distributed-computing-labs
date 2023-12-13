//
// Created by artem on 11.12.2023.
//
#include "banking.h"
#include "constants.h"
#include "lamport_logical_time.h"
#include "log.h"
#include <string.h>

//void transfer(void * parent_data, local_id src, local_id dst, balance_t amount){
//    process_content* processContent = (process_content*) parent_data;
//    TransferOrder transfer_order;
//    transfer_order.s_amount = amount;
//    transfer_order.s_dst = dst;
//    transfer_order.s_src = src;
//    Message msg;
//    MessageHeader message_header;
//    message_header.s_magic = MESSAGE_MAGIC;
//    message_header.s_payload_len = sizeof(TransferOrder);
//    message_header.s_type = TRANSFER;
//    increase_lamport_time(processContent);
//    message_header.s_local_time = processContent->process_lamport_time;
//    msg.s_header = message_header;
//    memcpy(msg.s_payload, &transfer_order, sizeof(transfer_order));
//    //while(send(processContent, dst, &msg)>0);
//    send(processContent, dst, &msg);
//    logging_transfer_out(processContent->process_lamport_time,src, amount, dst);
//
//    Message ack_message;
////    while(receive(processContent, dst, &ack_message) > 0 || ack_message.s_header.s_type != ACK){
////        lamport_time_from_message(ack_message.s_header.s_local_time, processContent);
////
////    }
//    receive(processContent, dst, &ack_message);
//    lamport_time_from_message(ack_message.s_header.s_local_time, processContent);
//}

void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    printf("transfer from src %d to dst %d amount %d.\n", src, dst, amount);
    TransferOrder order = {.s_src = src, .s_dst = dst, .s_amount = amount};
    //process_content* processContent = (process_content*) parent_data;
    //increase_lamport_time(processContent);
    Message transfer_message = {.s_header = {
            .s_magic = MESSAGE_MAGIC,
            .s_payload_len = sizeof(order),
            .s_type = TRANSFER,
            //.s_local_time = get_lamport_time(processContent) //lamport_inc_get_time()
            .s_local_time = lamport_inc_get_time()
    }};
    memcpy(transfer_message.s_payload, &order, sizeof(order));
    //assert(send(parent_data, src, &transfer_message) == 0);
    if(send(parent_data, src, &transfer_message)!=0){
        printf("transfer from src %d to dst %d unsuccessful", src, dst);
    }
    Message ack_message;
    //assert(receive(parent_data, dst, &ack_message) == 0);
    receive(parent_data, dst, &ack_message);
    //assert(ack_message.s_header.s_magic == MESSAGE_MAGIC);
    //assert(ack_message.s_header.s_type == ACK);
    //assert(ack_message.s_header.s_payload_len == 0);
    lamport_receive_time(ack_message.s_header.s_local_time);
    //lamport_time_from_message(ack_message.s_header.s_local_time, processContent);
    //lamport_receive_time(ack_message.s_header.s_local_time);
}
