#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H

#include <Arduino.h>

// Global configurable parameters (extern means they are defined elsewhere, in serial_comms.cpp)
extern int DT_BG_THRESHOLD;
extern int DT_MAX_DISTANCE;
extern int MIN_PERSON_AREA;
extern int MAX_PERSON_AREA;
extern int BG_FRAME_COUNT;
extern float TEMP_MIN;
extern float TEMP_MAX;
extern float TEMP_RANGE; // Derived from TEMP_MIN and TEMP_MAX

// Flag to indicate if BG_FRAME_COUNT was changed, signaling a background reset
extern bool config_changed_reset_bg;

// Function prototypes
void serial_comms_init();
void serial_comms_check_for_commands();

#endif // SERIAL_COMMS_H