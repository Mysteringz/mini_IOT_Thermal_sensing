#ifndef CONFIG_H
#define CONFIG_H

// --- MLX90640 Sensor Configuration ---
const uint32_t MLX_I2C_SPEED = 1000000; // 1MHz I2C speed for MLX90640
const uint8_t MLX90640_ADDRESS = 0x33; // Default 7-bit unshifted address
const int TA_SHIFT = 8;                // Default shift for MLX90640 in open air

// --- Image Processing Configuration (Fixed) ---
const int IMAGE_WIDTH = 32;
const int IMAGE_HEIGHT = 24;
const int IMAGE_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT;
const int MAX_PEOPLE = 20;
const int GAUSSIAN_KERNEL_3X3_SCALE = 16;  // Fixed-point scale factor

// --- WiFi Telemetry Configuration (LoRa replacement path) ---
const bool WIFI_TELEMETRY_ENABLED = true;
const char WIFI_SSID[] = "Ilo";
const char WIFI_PASSWORD[] = "PHOTOSHOP";
const uint8_t WIFI_REMOTE_IP_A = 10;
const uint8_t WIFI_REMOTE_IP_B = 187;
const uint8_t WIFI_REMOTE_IP_C = 101;
const uint8_t WIFI_REMOTE_IP_D = 187;
const uint16_t WIFI_REMOTE_PORT = 5100;
const uint32_t WIFI_RECONNECT_INTERVAL_MS = 5000;
const uint8_t NODE_DEVICE_ID = 2;

// --- Serial Output Configuration ---
#define SERIAL_OUTPUT_MODE 1  // 0 = Human-readable, 1 = Computer-readable (binary)
#if SERIAL_OUTPUT_MODE == 0
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif // CONFIG_H