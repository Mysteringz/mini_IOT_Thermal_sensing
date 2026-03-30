#include <Wire.h>
#include "mlx_sensor.h"
#include "thermal_image_processor.h"
#include "thermal_serializer.h"
#include "wifi_transport.h"
#include "config.h"
#include "serial_comms.h"

// MLX sensor instance
MlxSensor mlx_sensor;

// Thermal processor instance
ThermalProcessor thermal_processor;

bool sensor_ready = false;
uint32_t last_sensor_init_attempt_ms = 0;

bool try_sensor_init() {
  const uint32_t now = millis();
  if (sensor_ready) {
    return true;
  }

  if (now - last_sensor_init_attempt_ms < WIFI_RECONNECT_INTERVAL_MS) {
    return false;
  }

  last_sensor_init_attempt_ms = now;
  sensor_ready = mlx_sensor.init(MLX90640_ADDRESS, 41, 42, MLX_I2C_SPEED);
  return sensor_ready;
}

void setup() {
  Serial.begin(57600);

  DEBUG_PRINTLN("=== CrowdAware Node 2026 ===");

  // Initialize serial command handling
  serial_comms_init();

  DEBUG_PRINTLN("Initializing MLX90640 thermal sensor...");
  sensor_ready = mlx_sensor.init(MLX90640_ADDRESS, 41, 42, MLX_I2C_SPEED);

  // Initialize thermal processor (clears buffers)
  thermal_processor_init(&thermal_processor);
  DEBUG_PRINTLN("Thermal processor initialized.");

  // Start WiFi telemetry path in non-blocking mode.
  wifi_transport_init();

  // Initial background setup message using the current BG_FRAME_COUNT
  DEBUG_PRINTLN("Beginning background frame initialization...");
  DEBUG_PRINT("Frames remaining: ");
  DEBUG_PRINTLN(BG_FRAME_COUNT);
}

void loop() {
  wifi_transport_update();

  // Always check for incoming serial commands at the start of the loop
  serial_comms_check_for_commands();

  // Check if configuration (specifically BG_FRAME_COUNT) was changed by serial command
  if (config_changed_reset_bg) {
    thermal_processor_reset_background(&thermal_processor);
    DEBUG_PRINTLN("Configuration changed (BG_FRAME_COUNT), resetting background initialization.");
    DEBUG_PRINT("New BG_FRAME_COUNT: ");
    DEBUG_PRINTLN(BG_FRAME_COUNT);
    config_changed_reset_bg = false;  // Clear the flag
  }

  if (!try_sensor_init()) {
    return;
  }

  // Read thermal frame from MLX90640
  if (!mlx_sensor.readFrame(thermal_processor.raw_frame)) {
    return;
  }

  // Convert float values to 8-bit grayscale (0-255)
  thermal_processor_update_thermal_image(&thermal_processor);

  // Non-blocking processing step (background learn + detect handled internally).
  const uint8_t num_detected = thermal_processor_step(&thermal_processor, MAX_PEOPLE);

  if (!thermal_processor.background_ready) {
    if (thermal_processor.bg_frames_collected % 5 == 0) {
      DEBUG_PRINT("Background init: ");
      DEBUG_PRINT(BG_FRAME_COUNT - thermal_processor.bg_frames_collected);
      DEBUG_PRINTLN(" frames remaining");
    }
    return;
  }

  // LoRa replacement path for now: send detection info over WiFi.
  wifi_transport_send_packet(&thermal_processor);

  // Keep serial binary packet for debug tooling compatibility.
  send_binary_packet(Serial, &thermal_processor);
}
