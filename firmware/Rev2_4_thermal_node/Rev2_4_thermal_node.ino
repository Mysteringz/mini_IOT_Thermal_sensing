#include <heltec.h>
#include "LoRaWan_APP.h"

#include <Wire.h>
#include <string>
#include <vector>
#include <stack>
#include <utility>

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

#include "Web_interface.h"
#include "Camera_algorithm.h"
#include "Data_packet.h"
#include "LED_Controller.h"
#include "User_config.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <esp_now.h>


using namespace std;
//-----------
//Button Pin Config
int protocolSwitch = 0;  // variable for reading the pushbutton status
//-----------

// REPLACE WITH THE RECEIVER'S MAC Address
uint8_t broadcastAddress[] = {0x14, 0x2B, 0x2F, 0xB8, 0xCC, 0x9C};

// Create peer interface
esp_now_peer_info_t peerInfo;

// callback when data is sent

void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  
}

void mcu_set_license() {   // Not needed for current code
  uint32_t license[4] = {
    0x4156F952,
    0xCA95DB5A,
    0x2135F10C,
    0xEE1EE920
  };

  Mcu.setlicense(license,sizeof(license)/sizeof(license[0]));
}


//LORA//------------------------------------------------------------------------------------

uint8_t devEui[] = DEV_EUI;
uint8_t appEui[] = APP_EUI;
uint8_t appKey[] = APP_KEY;

// ABP parameters
uint8_t nwkSKey[] = NWKS_KEY;
uint8_t appSKey[] = APPS_KEY;
uint32_t devAddr = DEV_ADDR;

/*LoraWan channelsmask, default channels 0-7*/ 
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = CLASS_C;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;

/*OTAA or ABP*/
bool overTheAirActivation = true;

/*ADR enable*/
bool loraWanAdr = true;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 2;
/*!
* Number of trials to transmit the frame, if the LoRaMAC layer did not
* receive an acknowledgment. The MAC performs a datarate adaptation,
* according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
* to the following table:
*
* Transmission nb | Data Rate
* ----------------|-----------
* 1 (first)       | DR
* 2               | DR
* 3               | max(DR-1,0)
* 4               | max(DR-1,0)
* 5               | max(DR-2,0)
* 6               | max(DR-2,0)
* 7               | max(DR-3,0)
* 8               | max(DR-3,0)
*
* Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
* the datarate, in case the LoRaMAC layer did not receive an acknowledgment
*/
uint8_t confirmedNbTrials = 4;

//LORA//------------------------------------------------------------------------------------

float mlx90640To[768];
paramsMLX90640 mlx90640;

const uint8_t calcStart = 33; //Pin that goes high/low when calculations are complete
//This makes the timing visible on the logic analyzer

const int deviceid = 2;        // TO BE ADJUST !!!

float background_median[HEIGHT][WIDTH] = {0};
float temperatureFrames[HEIGHT][WIDTH] = {0};
float smoothedData[HEIGHT][WIDTH] = {0};
float subtractedFrame[HEIGHT][WIDTH] = {0};

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // Show device is starting
  Serial.begin(115200); // Fast serial as possible
  delay(1000); // Wait for serial to start
  Serial.println("Device starting");
  Wire.begin();
  Wire.setClock(400000); //Increase I2C clock speed to 400kHz

  // the number of the pushbutton pin
  static const int hardwareSwitch = 39;
  pinMode(hardwareSwitch, INPUT);
  protocolSwitch = digitalRead(hardwareSwitch);

  led_init();

  // Connect to WiFi network
  if ((protocolSwitch == LOW) ||  (ACTIVATE_SERVER)) {
    Serial.println("Starting WiFi connection");
      // Uses LoRaWAN Protocol
    led_set(LED_YELLOW);

    FastLED.delay(1000);

    WiFi.begin(ssid, password);
    Serial.print("WiFi connecting");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  if (ACTIVATE_SERVER) {
    Serial.println("Debugging server selected");
    if (!beginMDNS(host)) {
      while (1) {
        delay(1000);
      }
    }
    setupWebServer();
  }
  

  if (protocolSwitch == LOW) {
    Serial.println("ESP-NOW Protocol Selected");
    Serial.println("Please reset after changing protocol mode");
    // Uses ESP-NOW Protocol

    led_set(LED_BLUE);
    FastLED.delay(1000);
    WiFi.mode(WIFI_STA);
      // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }
  }

  // // mcu_set_license();
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  // pinMode(calcStart, OUTPUT);

  // if (isConnected() == false)
  // {
  //   Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
  //   while (1);
  // }
  // else {Serial.println("MLX90640 detected, starting");}

  // digitalWrite(LED_BUILTIN, HIGH);

  // //Get device parameters - We only have to do this once
  // int status;
  // uint16_t eeMLX90640[832];
  // status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  // if (status != 0)
  //   Serial.println("Failed to load system parameters");

  // status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  // if (status != 0)
  //   Serial.println("Parameter extraction failed");

  // //Once params are extracted, we can release eeMLX90640 array

  // //Set refresh rate
  // //A rate of 0.5Hz takes 4Sec per reading because we have to read two frames to get complete picture
  // //MLX90640_SetRefreshRate(MLX90640_address, 0x00); //Set rate to 0.25Hz effective - Works
  // //MLX90640_SetRefreshRate(MLX90640_address, 0x01); //Set rate to 0.5Hz effective - Works
  // MLX90640_SetRefreshRate(MLX90640_address, 0x02); //Set rate to 1Hz effective - Works
  // //MLX90640_SetRefreshRate(MLX90640_address, 0x03); //Set rate to 2Hz effective - Works
  // //MLX90640_SetRefreshRate(MLX90640_address, 0x04); //Set rate to 4Hz effective - Works
  // //MLX90640_SetRefreshRate(MLX90640_address, 0x05); //Set rate to 8Hz effective - Works at 800kHz
  // //MLX90640_SetRefreshRate(MLX90640_address, 0x06); //Set rate to 16Hz effective - Works at 800kHz
  // //MLX90640_SetRefreshRate(MLX90640_address, 0x07); //Set rate to 32Hz effective - fails

  //Once EEPROM has been read at 400kHz we can increase to 1MHz
  Wire.setClock(1000000); //Teensy will now run I2C at 800kHz (because of clock division)
  digitalWrite(LED_BUILTIN, LOW);
  delay(5000);
  // backgroundEstimation();
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);

  if (ACTIVATE_SERVER) {
    handleWebServer();
  }

  if (protocolSwitch == HIGH){ // LoRaWAN Mode
  {
    switch( deviceState )
    {
      case DEVICE_STATE_INIT:
      {
  #if(LORAWAN_DEVEUI_AUTO)
        LoRaWAN.generateDeveuiByChipID();
  #endif
        LoRaWAN.init(loraWanClass,loraWanRegion);
        //both set join DR and DR when ADR off 
        LoRaWAN.setDefaultDR(3);
        break;
      }
      case DEVICE_STATE_JOIN:
      {
        LoRaWAN.join();
        break;
      }
      case DEVICE_STATE_SEND:
      {
        prepareTxFrame( appPort );
        LoRaWAN.send();
        deviceState = DEVICE_STATE_CYCLE;
        break;
      }
      case DEVICE_STATE_CYCLE:
      {
        // Schedule next packet transmission
        txDutyCycleTime = appTxDutyCycle + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
        LoRaWAN.cycle(txDutyCycleTime);
        deviceState = DEVICE_STATE_SLEEP;
        break;
      }
      case DEVICE_STATE_SLEEP:
      {
        LoRaWAN.sleep(loraWanClass);
        break;
      }
      default:
      {
        deviceState = DEVICE_STATE_INIT;
        break;
      }
    }
  }
}

if (protocolSwitch == LOW)
  {
    Serial.println("Sending via ESP-NOW...");
    int count_clear = 0;
    while (myData.coords[count_clear] != '\0') {
      myData.coords[count_clear++] = '\0';
    }
    myData.id = 2;
    updateHeatmap(background_median);

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) Serial.println("ESP-NOW sent");
    else Serial.println("ESP-NOW send failed");
  }
  
  leds.fadeToBlackBy(1);
  FastLED.delay(100);
}

//Returns true if the MLX90640 is detected on the I2C bus
boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}

static void prepareTxFrame( uint8_t port ) // Specified signature for LoRaWAN
{
  (void) port; // Suppress warning
  // updateHeatmap(background_median);

  appDataSize = 4;
  appData[0] = 0x00;
  appData[1] = 0x01;
  appData[2] = 0x02;
  appData[3] = 0x03;

  Serial.println("packet sent");
}
