#include "lora_config.h"
#include "heatmap_processing.h"
#include "constants.h"
#include "LoRaWan_APP.h"

uint8_t appData[64];
uint8_t appDataSize;

DeviceState_t deviceState = DEVICE_STATE_INIT;
uint32_t appTxDutyCycle = 1000;

void setupLoRa() {
  LoRaWAN.init(CLASS_C, ACTIVE_REGION);
  LoRaWAN.setDefaultDR(3);
}

void handleLoRaStateMachine() {
  switch (deviceState) {
    case DEVICE_STATE_INIT:
      LoRaWAN.join();
      deviceState = DEVICE_STATE_JOIN;
      break;
    case DEVICE_STATE_JOIN:
      deviceState = DEVICE_STATE_SEND;
      break;
    case DEVICE_STATE_SEND:
      prepareTxFrame(1);
      LoRaWAN.send();
      deviceState = DEVICE_STATE_CYCLE;
      break;
    case DEVICE_STATE_CYCLE:
      LoRaWAN.cycle(appTxDutyCycle);
      deviceState = DEVICE_STATE_SLEEP;
      break;
    case DEVICE_STATE_SLEEP:
      LoRaWAN.sleep(CLASS_C);
      break;
    default:
      deviceState = DEVICE_STATE_INIT;
      break;
  }
}

void prepareTxFrame(uint8_t port) {
  updateHeatmap(background_median);
  Serial.println("LoRa packet prepared");
}
