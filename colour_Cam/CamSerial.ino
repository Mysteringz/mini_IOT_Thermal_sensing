#include <Arduino.h>
#include <esp_camera.h>

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// 4. Setup function runs once when the ESP32 powers on or resets
void setup() {
  // 4.1 Start the Serial communication at a very high baud rate
  Serial.begin(115200);
  delay(1000); // Give it a moment to start

  Serial.println("ESP32-CAM Image Sender Initializing...");

  // 5. Create a camera configuration object
  camera_config_t config;

  // 6. Assign the pre-defined pin constants from 'camera_pins.h' to the config
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // 8. Initialize the camera with our configuration
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x. Halted.", err);
    while (true); // Stop here forever if it fails.
  }

  Serial.println("Camera init successful! Starting loop...");
}

// 9. The main loop function runs repeatedly
void loop() {
  delay(1000); // Wait for 10 seconds

  Serial.print("IMG_START"); // Start of image transmission marker
  Serial.write('\r');            // Carriage return
  Serial.write('\n');            // New line
  Serial.flush();                // Ensure data is sent

  // Capture a frame from the camera
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Capture failed.");
    return; // Skip the rest of the loop and try again next time
  }

  // Send the image size and then the raw data itself
  Serial.println(fb->len);          // Send size as text
  Serial.write(fb->buf, fb->len);   // Send data as raw bytes
  Serial.println("IMG_END");        // End of transmission marker

  Serial.printf("Image captured and sent. Size: %d bytes\n", fb->len);

  // Return the frame buffer to be reused
  esp_camera_fb_return(fb);
}