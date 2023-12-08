//
// Created by artem on 04.12.2023.
//

#ifndef LAB1_CONSTANTS_H
#define LAB1_CONSTANTS_H
#define MAX_PROCESS_NUM 11

#include "ipc.h"


//static const int MAX_PROCESS_NUM = 10;
typedef struct {
    local_id this_process;
    int write_pipes[MAX_PROCESS_NUM][MAX_PROCESS_NUM];
    int read_pipes[MAX_PROCESS_NUM][MAX_PROCESS_NUM];
    uint8_t process_num;
} process_content;



//int writing_pipe[MAX_PROCESS_NUM][MAX_PROCESS_NUM];
//int reading_pipe[MAX_PROCESS_NUM][MAX_PROCESS_NUM];

#endif //LAB1_CONSTANTS_H
