/*
 * LoRaWAN uplink / downlink test (Heltec LoRaWan_APP)
 *
 * Uplink:  4 bytes, big-endian = millis() — milliseconds since the ESP32 booted.
 *          (Wraps after ~49 days; fine for checking payloads in a network console.)
 *
 * Downlink: server should send 4 bytes, big-endian = Unix time in whole seconds
 *          (same idea as Python int(time.time())), on the same FPort as appPort.
 *
 * Class A: the device only listens in RX1/RX2 right after it transmits, so you
 *          only get a downlink slot when an uplink was sent (or scheduled).
 */

#include "LoRaWan_APP.h"

// ----- LoRaWAN credentials (edit for your network) -----
uint8_t devEui[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t appEui[] = { 0xD1, 0xB4, 0xEE, 0x59, 0x5C, 0xCF, 0x1B, 0xC2 };
uint8_t appKey[] = { 0x21, 0x9B, 0x6E, 0xF3, 0x5A, 0x8A, 0x47, 0xB2, 0xA9, 0xF3, 0x41, 0x33, 0x04, 0x73, 0x2D, 0x16 };

uint8_t nwkSKey[] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85 };
uint8_t appSKey[] = { 0xd7, 0x2c, 0x78, 0x75, 0x8c, 0xdc, 0xca, 0xbf, 0x55, 0xee, 0x4a, 0x77, 0x8d, 0x16, 0xef, 0x67 };
uint32_t devAddr = (uint32_t)0x007e6ae1;

uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
DeviceClass_t  loraWanClass = CLASS_A;

// Time between uplinks (ms). Small value = more tests; respect network fair-use rules.
uint32_t appTxDutyCycle = 10000;

bool overTheAirActivation = true;
bool loraWanAdr = true;

// false = unconfirmed uplink (less airtime / retries). Downlink still works in Class A after TX.
bool isTxConfirmed = false;

// FPort for both directions in this test. Server must enqueue downlink on this port.
uint8_t appPort = 2;

uint8_t confirmedNbTrials = 4;

// ----- Uplink payload: millis() as 32-bit big-endian -----

static void prepareTxFrame(uint8_t port)
{
    (void)port;

    uint32_t m = millis();

    appDataSize = 4;
    appData[0] = (uint8_t)(m >> 24);
    appData[1] = (uint8_t)(m >> 16);
    appData[2] = (uint8_t)(m >> 8);
    appData[3] = (uint8_t)m;

    Serial.printf("[UL] millis=%lu (4 bytes BE)\r\n", (unsigned long)m);
}

/*
 * Heltec calls this when application data arrives from the network.
 * mcpsIndication->Buffer[0..BufferSize-1] is the decrypted payload.
 *
 */
void downLinkDataHandle(McpsIndication_t *mcpsIndication)
{
    if (mcpsIndication == nullptr) {
        return;
    }

    const char *win = mcpsIndication->RxSlot ? "RX2" : "RX1";

    Serial.printf("[DL] %s port=%u bytes=%u\r\n",
                  win,
                  (unsigned)mcpsIndication->Port,
                  (unsigned)mcpsIndication->BufferSize);

    if (mcpsIndication->BufferSize == 4 && mcpsIndication->Port == appPort) {
        uint32_t unixSec =
            (uint32_t)mcpsIndication->Buffer[0] << 24 |
            (uint32_t)mcpsIndication->Buffer[1] << 16 |
            (uint32_t)mcpsIndication->Buffer[2] << 8 |
            (uint32_t)mcpsIndication->Buffer[3];

        Serial.printf("[DL] Unix time from server: %lu s since 1970-01-01 UTC\r\n",
                      (unsigned long)unixSec);
        return;
    }

    if (mcpsIndication->BufferSize > 0) {
        Serial.print("[DL] raw HEX: ");
        for (uint8_t i = 0; i < mcpsIndication->BufferSize; i++) {
            Serial.printf("%02X", mcpsIndication->Buffer[i]);
        }
        Serial.println();
    }
}

RTC_DATA_ATTR bool firstrun = true;

void setup()
{
    Serial.begin(115200);
    Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
#ifdef WIFI_LORA_32_V4
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
#endif
    if (firstrun) {
        LoRaWAN.displayMcuInit();
        firstrun = false;
    }
}

void loop()
{
    switch (deviceState) {
    case DEVICE_STATE_INIT:
#if (LORAWAN_DEVEUI_AUTO)
        LoRaWAN.generateDeveuiByChipID();
#endif
        LoRaWAN.init(loraWanClass, loraWanRegion);
        LoRaWAN.setDefaultDR(3);
        break;

    case DEVICE_STATE_JOIN:
        LoRaWAN.displayJoining();
        LoRaWAN.join();
        break;

    // One extra step: send uplink, then cycle timer, then sleep (radio low power).
    case DEVICE_STATE_SEND:
        LoRaWAN.displaySending();
        prepareTxFrame(appPort);
        LoRaWAN.send();
        deviceState = DEVICE_STATE_CYCLE;
        break;

    case DEVICE_STATE_CYCLE:
        txDutyCycleTime = appTxDutyCycle + randr(-APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND);
        LoRaWAN.cycle(txDutyCycleTime);
        deviceState = DEVICE_STATE_SLEEP;
        break;

    case DEVICE_STATE_SLEEP:
        LoRaWAN.displayAck();
        LoRaWAN.sleep(loraWanClass);
        break;

    default:
        deviceState = DEVICE_STATE_INIT;
        break;
    }
}
