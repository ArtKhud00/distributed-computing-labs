//
// Created by artem on 07.12.2023.
//

#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include "parent_operations.h"
#include "general_actions.h"

void recieve_child_messages(process_content* processContent){
    recieve_messages_from_other_processes(processContent, STARTED);
    recieve_messages_from_other_processes(processContent, DONE);
    //recieve_messages_from_other_processes(processContent, STARTED);
}

void wait_for_childs() {
    pid_t child_pid = 0;
    int status = 0;
    while ((child_pid = wait(&status)) > 0) {
        printf("child process %d finished with %d.\n", child_pid, status);
    }
}
