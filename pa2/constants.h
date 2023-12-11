//
// Created by artem on 04.12.2023.
//

#ifndef LAB1_CONSTANTS_H
#define LAB1_CONSTANTS_H
#define MAX_PROCESS_NUM 11

#include "ipc.h"
#include "banking.h"

typedef enum{
    NOT_MAGIC_SIGN = 1,
    SEND_UNSUCCESFUL,
    MULTICAST_SEND_UNSUCCESFUL,
    RECIEVE_UNSUCCESFUL,


}Error_statuses;


typedef struct {
    local_id this_process;
    int write_pipes[MAX_PROCESS_NUM][MAX_PROCESS_NUM];
    int read_pipes[MAX_PROCESS_NUM][MAX_PROCESS_NUM];
    uint8_t process_num;
    balance_t process_balance;
    timestamp_t process_lamport_time;
    BalanceHistory* balanceHistory;
} process_content;

balance_t process_balances[MAX_PROCESS_NUM];

#endif //LAB1_CONSTANTS_H
