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
#include "lamport_logical_time.h"
#include "general_actions.h"

process_content processContent;

int parse_extra_parameter(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    uint8_t process_count;
    uint8_t using_mutex = 0;
    if (argc >= 2 && strcmp(argv[1], "-p") == 0) {
        process_count = (uint8_t)strtol(argv[2], NULL, 10);
        using_mutex = (uint8_t)parse_extra_parameter(argc, argv);
    } else{
        return 1;
    }
    //printf("using mutex = %d", using_mutex);
    logging_preparation();  // logger
    process_count = process_count + 1;
    set_pipe_descriptors(&processContent, process_count);
    modify_pipe_set_non_blocking(&processContent, process_count);
    processContent.process_num = process_count;
    for(uint8_t id = 1; id < process_count; ++id ){
        int child_process = fork();
        if(child_process == -1){
            perror("child process does not created"); // error
            exit(EXIT_FAILURE);
        }
        if(child_process == 0){
            processContent.this_process = id;
            processContent.print_iterations = id * 5;
            ProcessQueue process_queue;
            process_queue.len = 0;
            processContent.process_queue = process_queue;
            close_extra_pipes(&processContent, process_count);

            send_STARTED_message(&processContent);
            recieve_messages_from_other_processes(&processContent, STARTED);
            process_queries(&processContent, using_mutex);

            exit(0);
        }
        if(child_process > 0){
            processContent.this_process = PARENT_ID;
        }
    }
    close_extra_pipes(&processContent, process_count);
    recieve_child_messages(&processContent, STARTED);
    recieve_child_messages(&processContent, DONE);
    wait_for_childs();
    logging_finalize();  // logger
    return 0;
}

int parse_extra_parameter(int argc,char *argv[]){
    for(int i=1; i < argc; ++i){
        if(strcmp(argv[i], "--mutexl") == 0){
            return 1;
        }
    }
    return 0;
}
