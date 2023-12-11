//
// Created by artem on 11.12.2023.
//
#include "banking.h"
#include "constants.h"
#include "lamport_logical_time.h"
#include "log.h"
#include <string.h>

void transfer(void * parent_data, local_id src, local_id dst, balance_t amount){
    process_content* processContent = (process_content*) parent_data;
    TransferOrder transfer_order;
    transfer_order.s_amount = amount;
    transfer_order.s_dst = dst;
    transfer_order.s_src = src;
    Message msg;
    MessageHeader message_header;
    message_header.s_magic = MESSAGE_MAGIC;
    message_header.s_payload_len = sizeof(TransferOrder);
    message_header.s_type = TRANSFER;
    increase_lamport_time(processContent);
    message_header.s_local_time = processContent->process_lamport_time;
    msg.s_header = message_header;
    memcpy(msg.s_payload, &transfer_order, sizeof(TransferOrder));
    while(send(processContent, dst, &msg)>0);
    logging_transfer_out(processContent->process_lamport_time,src, amount, dst);

    Message ack_message;
    while(receive(processContent, dst, &ack_message) > 0 || ack_message.s_header.s_type != ACK){
        lamport_time_from_message(ack_message.s_header.s_local_time, processContent);

    }
}
