#include "serial_comms.h"
#include "config.h" // For DEBUG_PRINT, DEBUG_PRINTLN

// Initialize global configurable parameters with default values
int DT_BG_THRESHOLD = 30;
int DT_MAX_DISTANCE = 255;
int MIN_PERSON_AREA = 20;
int MAX_PERSON_AREA = 200;
int BG_FRAME_COUNT = 25;
float TEMP_MIN = 10.0f;
float TEMP_MAX = 35.0f;
float TEMP_RANGE = TEMP_MAX - TEMP_MIN;

// Flag to indicate if BG_FRAME_COUNT was changed, signaling a background reset
bool config_changed_reset_bg = false;

// Buffer for incoming serial commands
#define CMD_BUFFER_SIZE 64
char cmd_buffer[CMD_BUFFER_SIZE];
uint8_t cmd_buffer_idx = 0;

void serial_comms_init() {
    // Clear the command buffer on initialization
    memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
    cmd_buffer_idx = 0;
}

// Helper to update TEMP_RANGE when TEMP_MIN or TEMP_MAX change
void update_temp_range() {
    TEMP_RANGE = TEMP_MAX - TEMP_MIN;
    if (TEMP_RANGE <= 0.0f) { // Prevent division by zero or negative range
        TEMP_RANGE = 0.1f;    // Use a small positive value to avoid issues
    }
}

// Process a single received command string
void process_command(const char* cmd) {
#if SERIAL_OUTPUT_MODE == 0
    DEBUG_PRINT("Received command: ");
    DEBUG_PRINTLN(cmd);
#endif

    // Parse commands using strncmp for prefix matching and atoi/atof for value conversion
    if (strncmp(cmd, "SET_DT_BG_THRESHOLD=", 20) == 0) {
        DT_BG_THRESHOLD = atoi(cmd + 20);
#if SERIAL_OUTPUT_MODE == 0
        DEBUG_PRINT("DT_BG_THRESHOLD set to: "); DEBUG_PRINTLN(DT_BG_THRESHOLD);
#endif
    } else if (strncmp(cmd, "SET_DT_MAX_DISTANCE=", 20) == 0) {
        DT_MAX_DISTANCE = atoi(cmd + 20);
#if SERIAL_OUTPUT_MODE == 0
        DEBUG_PRINT("DT_MAX_DISTANCE set to: "); DEBUG_PRINTLN(DT_MAX_DISTANCE);
#endif
    } else if (strncmp(cmd, "SET_MIN_PERSON_AREA=", 20) == 0) {
        MIN_PERSON_AREA = atoi(cmd + 20);
#if SERIAL_OUTPUT_MODE == 0
        DEBUG_PRINT("MIN_PERSON_AREA set to: "); DEBUG_PRINTLN(MIN_PERSON_AREA);
#endif
    } else if (strncmp(cmd, "SET_MAX_PERSON_AREA=", 20) == 0) {
        MAX_PERSON_AREA = atoi(cmd + 20);
#if SERIAL_OUTPUT_MODE == 0
        DEBUG_PRINT("MAX_PERSON_AREA set to: "); DEBUG_PRINTLN(MAX_PERSON_AREA);
#endif
    } else if (strncmp(cmd, "SET_BG_FRAME_COUNT=", 19) == 0) {
        int new_bg_frame_count = atoi(cmd + 19);
        if (new_bg_frame_count != BG_FRAME_COUNT) {
            BG_FRAME_COUNT = new_bg_frame_count;
            config_changed_reset_bg = true; // Signal node.ino to reset background init
        }
#if SERIAL_OUTPUT_MODE == 0
        DEBUG_PRINT("BG_FRAME_COUNT set to: "); DEBUG_PRINTLN(BG_FRAME_COUNT);
#endif
    } else if (strncmp(cmd, "SET_TEMP_MIN=", 13) == 0) {
        TEMP_MIN = atof(cmd + 13);
        update_temp_range();
#if SERIAL_OUTPUT_MODE == 0
        DEBUG_PRINT("TEMP_MIN set to: "); DEBUG_PRINTLN(TEMP_MIN);
#endif
    } else if (strncmp(cmd, "SET_TEMP_MAX=", 13) == 0) {
        TEMP_MAX = atof(cmd + 13);
        update_temp_range();
#if SERIAL_OUTPUT_MODE == 0
        DEBUG_PRINT("TEMP_MAX set to: "); DEBUG_PRINTLN(TEMP_MAX);
#endif
    } else {
#if SERIAL_OUTPUT_MODE == 0
        DEBUG_PRINT("Unknown command: ");
        DEBUG_PRINTLN(cmd);
#endif
    }
}

// Checks for and processes incoming serial commands
void serial_comms_check_for_commands() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') { // Command ends with newline character
            cmd_buffer[cmd_buffer_idx] = '\0'; // Null-terminate the string
            process_command(cmd_buffer);
            cmd_buffer_idx = 0; // Reset buffer index for next command
            memset(cmd_buffer, 0, CMD_BUFFER_SIZE); // Clear buffer
        } else if (cmd_buffer_idx < CMD_BUFFER_SIZE - 1) {
            cmd_buffer[cmd_buffer_idx++] = c;
        } else {
            // Buffer overflow, clear buffer and reset to prevent corrupted commands
#if SERIAL_OUTPUT_MODE == 0
            DEBUG_PRINTLN("Command buffer overflow! Clearing buffer.");
#endif
            cmd_buffer_idx = 0;
            memset(cmd_buffer, 0, CMD_BUFFER_SIZE);
        }
    }
}