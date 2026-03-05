#include <heltec.h>

#include <Wire.h>
#include <string>
#include <vector>
#include <stack>
#include <utility>

#include "src/libs/MLX90640_API.h"
#include "src/libs/MLX90640_I2C_Driver.h"

#include "src/Web_interface.h"
#include "src/Camera_algorithm.h"

#include "LoRaWan_APP.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <esp_now.h>


using namespace std;

// LED Config
#include <FastLED.h>
#define NUM_LEDS 4
CRGBArray<NUM_LEDS> leds;
const int ledPin = LED_BUILTIN;    // the number of the LED pin

void led_set(uint8_t hue) {
    leds[0] = CHSV(hue,255,255);
    leds[1] = CHSV(hue,255,255);
    leds[2] = CHSV(hue,255,255);
    leds[3] = CHSV(hue,255,255);
}
uint8_t hue = 0;
//-----------

//Button Pin Config
const int hardwareSwitch = 39;  // the number of the pushbutton pin
int protocolSwitch = 0;  // variable for reading the pushbutton status
//-----------

// REPLACE WITH THE RECEIVER'S MAC Address
uint8_t broadcastAddress[] = {0x14, 0x2B, 0x2F, 0xB8, 0xCC, 0x9C};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
    uint8_t id; // must be unique for each sender board
    uint8_t coords[200];
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Create peer interface
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// uint32_t license[4] = {
//   0x4156F952,
//   0xCA95DB5A,
//   0x2135F10C,
//   0xEE1EE920
// };

const char* host = "iot2";
const char* ssid = "IOT_TEAM_WIFI";
const char* password = "12345678";
WebServer server(80);

//LORA//------------------------------------------------------------------------------------










// 70b3d50000000000

/* OTAA para*/
uint8_t devEui[8] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x07, 0x1A, 0x38};
uint8_t appEui[8] = {0x70, 0xB3, 0xD5, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t appKey[16] = {0xB0, 0xD8, 0xD8, 0xDF, 0xEF, 0x51, 0xE7, 0x99, 0x99, 0x83, 0x59, 0xA8, 0x6C, 0x7B, 0xD1, 0x72};

// ABP parameters
uint8_t nwkSKey[16] = { 0x15, 0xb1, 0xd0, 0xef, 0xa4, 0x63, 0xdf, 0xbe, 0x3d, 0x11, 0x18, 0x1e, 0x1e, 0xc7, 0xda, 0x85 };
uint8_t appSKey[16] = { 0xF2, 0xDE, 0x92, 0xDC, 0x9C, 0x66, 0x66, 0x72, 0xF9, 0x6A, 0xEF, 0xFE, 0x3D, 0xAA, 0x1B, 0x08 };
uint32_t devAddr = (uint32_t)0x007e6ae1;

/*LoraWan channelsmask, default channels 0-7*/ 
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = CLASS_C;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 500;

/*OTAA or ABP*/
bool overTheAirActivation = true;

/*ADR enable*/
bool loraWanAdr = false;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = true;

/* Application port */
uint8_t appPort = 1;
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
uint8_t confirmedNbTrials = 1;














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

void readTemperatureData2D(float (&output)[HEIGHT][WIDTH]);
void smoothImage(const float (&input)[HEIGHT][WIDTH], float (&output)[HEIGHT][WIDTH]);
void applyThreshold(float (&tempData)[HEIGHT][WIDTH]);
vector<vector<float>> createPaddedArray(const float (&input)[HEIGHT][WIDTH]);
void backgroundEstimation();
void updateHeatmap(const float (&background_median)[HEIGHT][WIDTH]);
vector<pair<int, int>> findComponentCenters(const float (&thresholdData)[HEIGHT][WIDTH]);
void dfs(const int x, const int y, const float (&thresholdData)[HEIGHT][WIDTH], bool (&visited)[HEIGHT][WIDTH], vector<pair<int, int> > &component);
void applyBackgroundSubtraction(const float (&background)[HEIGHT][WIDTH], const float (&input)[HEIGHT][WIDTH], float (&output)[HEIGHT][WIDTH]);

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(hardwareSwitch, INPUT);
  protocolSwitch = digitalRead(hardwareSwitch);
  // Connect to WiFi network
  FastLED.addLeds<NEOPIXEL,2>(leds, NUM_LEDS);
  FastLED.setBrightness(255);


  if (protocolSwitch == HIGH) {
      // Uses LoRaWAN Protocol
    led_set(10);
    FastLED.show();
    FastLED.delay(1000);

    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    /*use mdns for host name resolution*/
    if (!MDNS.begin(host)) { //http://esp32.local
      Serial.println("Error setting up MDNS responder!");
      while (1) {
        delay(1000);
      }
    }
    Serial.println("mDNS responder started");
    /*return index page which is stored in serverIndex */
    server.on("/", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", loginIndex);
    });
    server.on("/serverIndex", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", WebServer_config.serverIndex);
    });
    /*handling uploading firmware file */
    server.on("/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
    });
    server.begin();
  }

    else { //protocolSwitch = LOW
        // Uses ESP-NOW Protocol

    led_set(150);
    FastLED.show();
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

  Serial.begin(115200); //Fast serial as possible
  while(!Serial);
  // Mcu.setlicense(license,sizeof(license)/sizeof(license[0]));
  Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
  pinMode(calcStart, OUTPUT);
  Wire.begin();
  Wire.setClock(400000); //Increase I2C clock speed to 400kHz

  led_set(200);
  FastLED.show();
  FastLED.delay(1000);

  if (isConnected() == false)
  {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
    while (1);
  }

  digitalWrite(LED_BUILTIN, HIGH);

  //Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90640[832];
  status = MLX90640_DumpEE(MLX90640_address, eeMLX90640);
  if (status != 0)
    Serial.println("Failed to load system parameters");

  status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
  if (status != 0)
    Serial.println("Parameter extraction failed");

  //Once params are extracted, we can release eeMLX90640 array

  //Set refresh rate
  //A rate of 0.5Hz takes 4Sec per reading because we have to read two frames to get complete picture
  //MLX90640_SetRefreshRate(MLX90640_address, 0x00); //Set rate to 0.25Hz effective - Works
  //MLX90640_SetRefreshRate(MLX90640_address, 0x01); //Set rate to 0.5Hz effective - Works
  MLX90640_SetRefreshRate(MLX90640_address, 0x02); //Set rate to 1Hz effective - Works
  //MLX90640_SetRefreshRate(MLX90640_address, 0x03); //Set rate to 2Hz effective - Works
  //MLX90640_SetRefreshRate(MLX90640_address, 0x04); //Set rate to 4Hz effective - Works
  //MLX90640_SetRefreshRate(MLX90640_address, 0x05); //Set rate to 8Hz effective - Works at 800kHz
  //MLX90640_SetRefreshRate(MLX90640_address, 0x06); //Set rate to 16Hz effective - Works at 800kHz
  //MLX90640_SetRefreshRate(MLX90640_address, 0x07); //Set rate to 32Hz effective - fails

  //Once EEPROM has been read at 400kHz we can increase to 1MHz
  Wire.setClock(1000000); //Teensy will now run I2C at 800kHz (because of clock division)

  backgroundEstimation();
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  if (protocolSwitch == HIGH) {
      server.handleClient();
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
          digitalWrite(LED_BUILTIN, LOW);
          prepareTxFrame( appPort );
          LoRaWAN.send();
          deviceState = DEVICE_STATE_CYCLE;
          break;
        }
        case DEVICE_STATE_CYCLE:
        {
          // Schedule next packet transmission
          txDutyCycleTime = appTxDutyCycle;
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
  } else {
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
  FastLED.show();
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

static void prepareTxFrame( uint8_t port ) //This function is useless pretty much
{
  updateHeatmap(background_median);
  Serial.println("packet sent");
}