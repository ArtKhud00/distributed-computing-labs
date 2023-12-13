//
// Created by artem on 10.12.2023.
//

#ifndef LAB2_LAMPORT_LOGICAL_TIME_H
#define LAB2_LAMPORT_LOGICAL_TIME_H

#include "banking.h"
#include "constants.h"

timestamp_t lamport_get_time();

timestamp_t lamport_inc_get_time();

timestamp_t lamport_receive_time(timestamp_t received_time);

#endif //LAB2_LAMPORT_LOGICAL_TIME_H
