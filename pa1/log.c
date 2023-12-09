//
// Created by Vitaliy on 09.12.2023
//

#include <stdio.h>
#include <unistd.h>

#include "common.h"
#include "pa1.h"
#include "log.h"


int log_init(void) {
    if (
        (pipes_log_f = fopen(pipes_log, "w")) == NULL ||
        (events_log_f = fopen(events_log, "w")) == NULL
    ) {
        perror("fopen() failed");
        log_retire();
        return -1;
    }

    return 0;
}

void log_retire(void) {
    fclose(pipes_log_f);
    fclose(events_log_f);
}

void log_started(local_id id) {
    printf(log_started_fmt, id, getpid(), getpid());
    fprintf(events_log_f, log_started_fmt, id, getpid(), getpid());
}

void log_received_all_started(local_id id) {
    printf(log_received_all_started_fmt, id);
    fprintf(events_log_f, log_received_all_started_fmt, id);
}

void log_done(local_id id) {
    printf(log_done_fmt, id);
    fprintf(events_log_f, log_done_fmt, id);
}

void log_received_all_done(local_id id) {
    printf(log_received_all_done_fmt, id);
    fprintf(events_log_f, log_received_all_started_fmt, id);
}

void log_pipes_created(local_id from, local_id to, int read_fd, int write_fd) {
    printf(log_pipes_created_fmt, from, to, read_fd, write_fd);
    fprintf(pipes_log_f, log_pipes_created_fmt, from, to, read_fd, write_fd);
}
