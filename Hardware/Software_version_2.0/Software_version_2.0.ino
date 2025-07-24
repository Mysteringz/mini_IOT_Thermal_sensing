#include "web_server.h"
#include "lora_config.h"
#include "mlx90640_driver.h"
#include "heatmap_processing.h"

void setup() {
  Serial.begin(115200);
  setupMLX90640();
  setupLoRa();
  setupWebServer();
  backgroundEstimation();
}

void loop() {
  server.handleClient();
  handleLoRaStateMachine();
}