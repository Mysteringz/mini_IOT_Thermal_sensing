#include <Wire.h>
#include <string>
#include <vector>
#include <stack>
#include <utility>

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"
#include "LoRaWan_APP.h"
#include "Wire.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <esp_now.h>
using namespace std;

#include <FastLED.h>
#define NUM_LEDS 3
CRGBArray<NUM_LEDS> leds;

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 39;  // the number of the pushbutton pin
const int ledPin = LED_BUILTIN;    // the number of the LED pin

// variables will change:
int buttonState = 0;  // variable for reading the pushbutton status

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
/* Style */
String style =
"<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
"input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
"#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
"#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
"form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

/* Login page */
String loginIndex = 
"<form name=loginForm>"
"<h1>ESP32 Login</h1>"
"<input name=userid placeholder='User ID'> "
"<input name=pwd placeholder=Password type=Password> "
"<input type=submit onclick=check(this.form) class=btn value=Login></form>"
"<script>"
"function check(form) {"
"if(form.userid.value=='admin' && form.pwd.value=='admin')"
"{window.open('/serverIndex')}"
"else"
"{alert('Error Password or Username')}"
"}"
"</script>" + style;
 
/* Server Index Page */
String serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
"<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
"<label id='file-input' for='file'>   Choose file...</label>"
"<input type='submit' class=btn value='Update'>"
"<br><br>"
"<div id='prg'></div>"
"<br><div id='prgbar'><div id='bar'></div></div><br></form>"
"<script>"
"function sub(obj){"
"var fileName = obj.value.split('\\\\');"
"document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
"};"
"$('form').submit(function(e){"
"e.preventDefault();"
"var form = $('#upload_form')[0];"
"var data = new FormData(form);"
"$.ajax({"
"url: '/update',"
"type: 'POST',"
"data: data,"
"contentType: false,"
"processData:false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var per = evt.loaded / evt.total;"
"$('#prg').html('progress: ' + Math.round(per*100) + '%');"
"$('#bar').css('width',Math.round(per*100) + '%');"
"}"
"}, false);"
"return xhr;"
"},"
"success:function(d, s) {"
"console.log('success!') "
"},"
"error: function (a, b, c) {"
"}"
"});"
"});"
"</script>" + style;

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

const uint8_t MLX90640_address = 0x33; //Default 7-bit unshifted address of the MLX90640

#define TA_SHIFT 8 //Default shift for MLX90640 in open air

float mlx90640To[768];
paramsMLX90640 mlx90640;

const uint8_t calcStart = 33; //Pin that goes high/low when calculations are complete
//This makes the timing visible on the logic analyzer

const int HEIGHT = 24;
const int WIDTH = 32;
const int KERNEL_SIZE = 3;
const int BG_FRAME_COUNT = 25;
const float THRESHOLD_TEMP_VALUE = 23;    // TO BE ADJUST !!!   (in innowing)
const float THRESHOLD_DIFF_VALUE = 1.3;   // TO BE ADJUST !!!
const int MIN_COMPONENT_SIZE = 5;   
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
void led_set(uint8_t hue) {
  leds[0] = CHSV(hue,255,255);
  leds[1] = CHSV(hue,255,255);
  leds[2] = CHSV(hue,255,255);
}

uint8_t hue = 0;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, INPUT);
  buttonState = digitalRead(buttonPin);
  // Connect to WiFi network
  FastLED.addLeds<NEOPIXEL,2>(leds, NUM_LEDS);
  FastLED.setBrightness(255);


  if (buttonState == HIGH) {
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
      server.send(200, "text/html", serverIndex);
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
  } else {
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
  if (buttonState == HIGH) {
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

void updateHeatmap(const float (&background_median)[HEIGHT][WIDTH]) {

    readTemperatureData2D(temperatureFrames);

    smoothImage(temperatureFrames, smoothedData);

    applyBackgroundSubtraction(background_median, smoothedData, subtractedFrame);

    applyThreshold(subtractedFrame);

    vector<pair<int, int>> centerIndexOfComponents = findComponentCenters(subtractedFrame);

//    Serial.print("Number of components is ");
//    Serial.println(centerIndexOfComponents.size());
    appDataSize = 0;
    int appDataSizewifi = 0;
    appData[appDataSize++] = deviceid;
    if (centerIndexOfComponents.size() != 0) {
      for (auto const& component : centerIndexOfComponents) {
          Serial.print(component.first);
          Serial.print(",");
          Serial.print(component.second);
          Serial.print(";");
          myData.coords[appDataSizewifi++] = component.first;
          myData.coords[appDataSizewifi++] = component.second;
          appData[appDataSize++] = component.first;
          appData[appDataSize++] = component.second;
          led_set(0);
      }
    } else {
      led_set(30);
    }
    Serial.println();
    centerIndexOfComponents.clear();
    centerIndexOfComponents.shrink_to_fit();

}

void readTemperatureData2D(float (&output)[HEIGHT][WIDTH])
{

  for (uint8_t x = 0 ; x < 2 ; x++)
  {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);

    digitalWrite(calcStart, HIGH);
    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    float emissivity = 0.95;

    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);
    digitalWrite(calcStart, LOW);
    //Calculation time on a Teensy 3.5 is 71ms
  }

  for (int i = 0; i < HEIGHT; ++i) {
    for (int j = 0; j < WIDTH; ++j) {
      output[i][j] = mlx90640To[i * WIDTH + j];
    }
  }

//  Serial.println("temp array read");
//  for (int i = 0 ; i < HEIGHT ; i++) {
//    for (int j = 0 ; j < WIDTH ; j++) {
//      Serial.print(output[i][j], 2);
//      if (i != HEIGHT-1 || j != WIDTH-1) Serial.print(",");
//    }
//    Serial.println();
//  }
//  Serial.println();

}

// Smooth Image using kernel convolution (3x3)
void smoothImage(const float (&input)[HEIGHT][WIDTH], float (&output)[HEIGHT][WIDTH])
{

    vector<vector<float>> paddedArray = createPaddedArray(input);
    int pad = KERNEL_SIZE / 2;

    float kernel[KERNEL_SIZE][KERNEL_SIZE] = {
        {1.0f / 16, 2.0f / 16, 1.0f / 16},
        {2.0f / 16, 4.0f / 16, 2.0f / 16},
        {1.0f / 16, 2.0f / 16, 1.0f / 16},
    };

    for (int i = pad; i < HEIGHT + pad; ++i)
    {
        for (int j = pad; j < WIDTH + pad; ++j)
        {
            float sum = 0.0;

            // Convolve the kernel with the image
            for (int ki = -1 * pad; ki <= pad; ++ki)
            {
                for (int kj = -1 * pad; kj <= pad; ++kj)
                {
                    sum += paddedArray[i + ki][j + kj] * kernel[ki + 1][kj + 1];
                }
            }

            output[i - pad][j - pad] = sum;
        }
    }

//    Serial.println("smooth done");
}

// Apply constant threshold to the background subtracted image
void applyThreshold(float (&tempData)[HEIGHT][WIDTH])
{
    for (int row = 0; row < HEIGHT; row++) {
      for (int col = 0; col < WIDTH; col++) {
        tempData[row][col] = (tempData[row][col] > THRESHOLD_DIFF_VALUE ? 1 : 0);
      }
    }

// Uncomment this print function if would like to see the shape of the object
//    Serial.println("threshold done");
//    for (int i = 0; i < HEIGHT; i++) {
//      for (int j = 0; j < WIDTH; j++) {
//        Serial.print(tempData[i][j], 2);
//        if (i != HEIGHT-1 || j != WIDTH-1) Serial.print(",");
//      }
//      Serial.println();
//    }

}

// Add padding of size (KERNEL_SIZE) to facilitate the kernel traversing and dot product at the edge of the frames
vector<vector<float>> createPaddedArray(const float (&input)[HEIGHT][WIDTH])
{
    int pad = KERNEL_SIZE / 2;
    vector<vector<float>> paddedArray(HEIGHT + 2 * pad, vector<float>(WIDTH + 2 * pad, 0.0));

    // Center area
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            paddedArray[i + pad][j + pad] = input[i][j];
        }
    }

    // Top and Bottom mirror padding
    for (int j = 0; j < WIDTH; ++j)
    {
        for (int i = 0; i < pad; ++i)
        {
            paddedArray[pad - i - 1][j + pad] = input[i][j];                   // Top mirror
            paddedArray[HEIGHT + pad + i][j + pad] = input[HEIGHT - i - 1][j]; // Bottom mirror
        }
    }

    // Left and Right mirror padding
    for (int i = 0; i < HEIGHT; ++i)
    {
        for (int j = 0; j < pad; ++j)
        {
            paddedArray[i + pad][pad - j - 1] = input[i][j];                 // Left mirror
            paddedArray[i + pad][WIDTH + pad + j] = input[i][WIDTH - j - 1]; // Right mirror
        }
    }

    // Corner mirror padding
    for (int i = 0; i < pad; ++i)
    {
        for (int j = 0; j < pad; ++j)
        {
            paddedArray[pad - i - 1][pad - j - 1] = input[i][j];                                   // Top-left corner
            paddedArray[pad - i - 1][WIDTH + pad + j] = input[i][WIDTH - j - 1];                   // Top-right corner
            paddedArray[HEIGHT + pad + i][pad - j - 1] = input[HEIGHT - i - 1][j];                 // Bottom-left corner
            paddedArray[HEIGHT + pad + i][WIDTH + pad + j] = input[HEIGHT - i - 1][WIDTH - j - 1]; // Bottom-right corner
        }
    }

    return paddedArray;
}

// Create the background median from the first 25 frames for background subtraction
void backgroundEstimation()
{
    // Create dynamic array (to prevent stack overflow)
    // since float tempFrames[BG_FRAME_COUNT][HEIGHT][WIDTH] causes stack overflow
    float*** tempFrames = (float***)malloc(BG_FRAME_COUNT * sizeof(float**));
    for (int f = 0; f < BG_FRAME_COUNT; f++) {
        tempFrames[f] = (float**)malloc(HEIGHT * sizeof(float*));
        for (int i = 0; i < HEIGHT; i++) {
            tempFrames[f][i] = (float*)malloc(WIDTH * sizeof(float));
        }
    }

    // Copy the data into the dynamically allocated array
    // Because tempFrames is float** but the data from readTemperatureData2D is float (&)[24][32], so we cannot directly do tempFrames[f] = tempFrame and need to copy the value one-by-one
    float tempFrame[HEIGHT][WIDTH];
    for (int f = 0; f < BG_FRAME_COUNT; f++) {
        readTemperatureData2D(tempFrame);
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                tempFrames[f][i][j] = tempFrame[i][j];
            }
        }
    }

    // Find the median of background frames
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            vector<float> pixelValues(BG_FRAME_COUNT);
            for (int f = 0; f < BG_FRAME_COUNT; f++) {
                pixelValues[f] = tempFrames[f][i][j];
            }
            sort(pixelValues.begin(), pixelValues.end());
            background_median[i][j] = (BG_FRAME_COUNT % 2 == 0)
                ? (pixelValues[BG_FRAME_COUNT / 2 - 1] + pixelValues[BG_FRAME_COUNT / 2]) / 2.0
                : pixelValues[(BG_FRAME_COUNT - 1) / 2];
        }
    }

    // Free the allocated memory
    for (int f = 0; f < BG_FRAME_COUNT; f++) {
        for (int i = 0; i < HEIGHT; i++) {
            free(tempFrames[f][i]);
        }
        free(tempFrames[f]);
    }
    free(tempFrames);

//    Serial.println("background estimation done");

}

// subtract background median from the current frame
void applyBackgroundSubtraction(const float (&background)[HEIGHT][WIDTH], const float (&input)[HEIGHT][WIDTH], float (&output)[HEIGHT][WIDTH]) {
  
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            output[i][j] = ((input[i][j] > THRESHOLD_TEMP_VALUE) && (input[i][j] - background[i][j] > 0) ? input[i][j] - background[i][j] : 0);
        }
    }

//    Serial.println("background subtraction done");
//
//    for (int i = 0; i < HEIGHT; i++) {
//      for (int j = 0; j < WIDTH; j++) {
//        Serial.print(output[i][j], 2);
//        if (i != HEIGHT-1 || j != WIDTH-1) Serial.print(",");
//      }
//      Serial.println();
//    }
}   

// Perform DFS and find the connected component
void dfs(const int x, const int y, const float (&thresholdData)[HEIGHT][WIDTH], bool (&visited)[HEIGHT][WIDTH], vector<pair<int, int> > &component) {
    // all the grids connected to the focus grid
    int directions[8][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };

    // queue to inspect next
    stack<pair<int, int>> stack;
    stack.push({x, y});

    while (!stack.empty()) {
        auto [curX, curY] = stack.top();
        stack.pop();

        if (visited[curX][curY]) continue;

        visited[curX][curY] = true;
        component.push_back({curX, curY});

        for (auto &dir : directions) {
            int newX = curX + dir[0];
            int newY = curY + dir[1];

            if ((0 <= newX && newX < HEIGHT) && (0 <= newY && newY < WIDTH) && thresholdData[newX][newY] == 1 && !visited[newX][newY]) {
                stack.push({newX, newY});
            }
        }
    }
}

// Find the center of each connected component
vector<pair<int, int>> findComponentCenters(const float (&thresholdData)[HEIGHT][WIDTH]) {
    bool visited[HEIGHT][WIDTH] = {false};
    vector<pair<int, int>> centersOfComponents;

    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            if (thresholdData[i][j] == 1 && !visited[i][j]) {
                vector<pair<int, int>> component;   // stored the index of all connected elements inside the component

//                Serial.println("start dfs");
                
                dfs(i, j, thresholdData, visited, component);

//                Serial.println(component.size());
                // Ignore object with connected components less than the min size
                if (component.size() < MIN_COMPONENT_SIZE) continue;

                // Calculate the center of the component
                int sumX = 0, sumY = 0;
                for (const auto& p : component) {
                    sumY += p.first;    // because the first index is height (y-axis)
                    sumX += p.second;   // because the first index is height (x-axis)
                }
                int centerX = sumX / component.size();
                int centerY = sumY / component.size();
                centersOfComponents.push_back({centerX, centerY});
            }
        }
    }

    return centersOfComponents;
}



static void prepareTxFrame( uint8_t port )
{
  updateHeatmap(background_median);
  Serial.println("packet sent");
}