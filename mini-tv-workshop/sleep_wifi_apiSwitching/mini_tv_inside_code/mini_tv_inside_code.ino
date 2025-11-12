/*
Sleep, WiFi, and API switching features.

User input can switch between different modes of the HKO API
https://www.hko.gov.hk/en/weatherAPI/doc/files/HKO_Open_Data_API_Documentation.pdf

Currently uses serial for input and output.
This will later be changed to use the encoder and screen
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_sleep.h"

// Wi-Fi credentials
const char* ssid = "Ilo";
const char* password = "photoshop";

// Base server URL
const char* serverName = "https://data.weather.gov.hk/weatherAPI/opendata/weather.php";

// Default language
const char* lang = "en";

// Valid dataType options
const char* validTypes[] = {"flw", "fnd", "rhrread", "warnsum", "warningInfo", "swt"};
const int numTypes = 6;

// RTC memory to keep dataType across deep sleep
RTC_DATA_ATTR char currentDataType[12] = "flw";  
RTC_DATA_ATTR int bootCount = 0;

// Conversion factor and sleep time
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP  10   // seconds

// --- Utility Functions ---

void printWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void makeRequest(const char* dataType) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(serverName) + "?lang=" + lang + "&dataType=" + dataType;
    http.begin(url);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println("Response:");
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
    connectWiFi();
  }

  Serial.println("Options: flw, fnd, rhrread, warnsum, warningInfo, swt, sleep");
}

void printInstructions() {
  Serial.println("\n--- Instructions ---");
  Serial.println("Type one of the following dataType options to switch:");
  Serial.println("Weather Forecast : flw (Today), fnd (9-day)");
  Serial.println("Weather Report   : rhrread");
  Serial.println("Weather Warning  : warnsum (Summary), warningInfo (Information), swt (Special tips)");
  Serial.println("sleep      : Enter deep sleep for 10 seconds (remembers last dataType)");
  Serial.print("Any other input will repeat the last request (");
  Serial.print(currentDataType);
  Serial.println(").");
  Serial.println("--------------------\n");
}

void goToSleep() {
  Serial.println("Going to sleep now for 10 seconds...");
  delay(1000);
  Serial.flush();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void handleInput(String input) {
  input.trim();
  if (input.equalsIgnoreCase("sleep")) {
    goToSleep();
    return;
  }

  bool isValid = false;
  for (int i = 0; i < numTypes; i++) {
    if (input.equalsIgnoreCase(validTypes[i])) {
      strncpy(currentDataType, validTypes[i], sizeof(currentDataType) - 1);
      currentDataType[sizeof(currentDataType) - 1] = '\0'; // ensure null termination
      isValid = true;
      break;
    }
  }

  if (isValid) {
    Serial.print("Switching to dataType: ");
    Serial.println(currentDataType);
    makeRequest(currentDataType);
  } else {
    Serial.println("Executing last known request...");
    makeRequest(currentDataType);
  }
}

// --- Setup & Loop ---

void setup() {
  Serial.begin(115200);
  delay(1000);

  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  printWakeupReason();

  connectWiFi();
  printInstructions();
  makeRequest(currentDataType);
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    handleInput(input);
  }
}