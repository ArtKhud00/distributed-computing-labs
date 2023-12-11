#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ipc.h"
#include "constants.h"
#include "pipe_operations.h"
#include "child_operations.h"
#include "parent_operations.h"
#include "log.h"
#include "banking.h"
#include "lamport_logical_time.h"

process_content processContent;

void parse_process_balances(char *argv[], int proc_count);

int main(int argc, char *argv[]) {

    uint8_t process_count;
    if (argc >= 3 && strcmp(argv[1], "-p") == 0) {
        process_count = (uint8_t)strtol(argv[2], NULL, 10);
    } else{
        return 1;
    }
    parse_process_balances(argv, process_count);
    logging_preparation();  // logger
    process_count = process_count + 1;
    set_pipe_descriptors(&processContent, process_count);

    modify_pipe_set_non_blocking(&processContent, process_count);

    processContent.process_num = process_count;
    processContent.this_process = PARENT_ID;
    for(uint8_t id = 1; id < process_count; ++id ){
        int child_process = fork();
        if(child_process == -1){
            perror("child process does not created"); // error
            exit(EXIT_FAILURE);
        }
        if(child_process == 0){
            processContent.this_process = id;
            processContent.process_lamport_time = 0;
            processContent.process_balance = process_balances[id - 1];
            close_extra_pipes(&processContent, process_count);
            save_balance_state(&processContent,processContent.process_lamport_time, processContent.process_balance);
            send_recieve_STARTED_message(&processContent);


            send_recieve_DONE_message(&processContent);
            exit(0);
        }
        if(child_process > 0){
            processContent.this_process = PARENT_ID;
        }
    }
    printf("In parent section\n");
    close_extra_pipes(&processContent, process_count);
    processContent.process_balance = 0;
    processContent.process_lamport_time = 0;
    recieve_child_messages(&processContent, STARTED);
    increase_lamport_time(&processContent);
    bank_robbery(&processContent, process_count);
    send_stop_messages(&processContent);
    recieve_child_messages(&processContent, DONE);
    recieve_BalanceHistory_messages(&processContent);
    wait_for_childs();
    logging_finalize();  // logger
    return 0;
}

void parse_process_balances(char *argv[], int proc_count) {
    const int position = 3;
    int k = 0;
    const int end_position = position + proc_count;
    for(int pos = position; pos < end_position; ++pos, ++k){
        process_balances[k] = (balance_t )strtol(argv[pos], NULL, 10);
    }
}
