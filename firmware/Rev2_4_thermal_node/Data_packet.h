//
// Created by Zhifang L on 05/03/2026.
//

#ifndef REV2_4_THERMAL_NODE_DATA_PACKET_H
#define REV2_4_THERMAL_NODE_DATA_PACKET_H

#include <Arduino.h>

// Structure example to send data
// Must match the receiver structure


// Define the struct type and declare the external instance
struct struct_message {
    uint8_t id; // must be unique for each sender board
    uint8_t coords[200];
};

extern struct struct_message myData;

#endif //REV2_4_THERMAL_NODE_DATA_PACKET_H
