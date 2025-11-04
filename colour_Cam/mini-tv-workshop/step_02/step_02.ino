/**
 * @file    mini_tv_master.ino
 * @brief   Check if screen and rotary encoder are working
 * @author  Aom, Ilo
 * @date    2025-09-12
 * @version 1.0
 */

// Required Libraries: Adafruit GFX, Adafruit ST7735, TFT, Arduino JSON

#include <TFT_eSPI.h> // Drawing text, images, and shapes on the TFT display
#include <SPI.h>      // Serial Data Protocol

#include <WiFi.h>        // WiFi connectivity
#include <HTTPClient.h>  // HTTP requests
#include <ArduinoJson.h> // JSON parsing
#include <time.h>

#include "Icons.h"

TFT_eSPI screen = TFT_eSPI();

// --- WiFi Configuration ---
#define WIFI_SSID "am"
#define WIFI_PASSWORD "0803536035"

// --- Display Parameters ---
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320
#define GMT_OFFSET 8

// --- Function Declaration ---
void displayText(const char *text, int textSize, int x, int y);

void displayText(const char *text, int textSize = 5, int x = 0, int y = 0)
{

  // screen.fillScreen(TFT_BLACK);           // clear screen with black background
  screen.fillRect(210, 0, SCREEN_WIDTH - 210, SCREEN_HEIGHT, TFT_BLACK);
  screen.setTextColor(TFT_WHITE, TFT_BLACK); // set text color
  screen.drawString(text, x, y, textSize);   // display text
}

class Page {
protected:
  void drawBitmapGif(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h, uint16_t frameCount)
  {
    uint32_t offset = 0;
    for (uint16_t frameId = 0; frameId < frameCount; frameId++)
    {
      offset = frameId * w * h;
      screen.pushImage(x, y, w, h, bitmap + offset);
      delay(500); // Delay between frames
    }
  }

  void APIGetRequest(const char *url, DynamicJsonDocument &doc)
  {
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode != HTTP_CODE_OK)
    {
      Serial.println("Error Calling API!");
      delay(5000);
      return;
    }

    String payload = http.getString();
    deserializeJson(doc, payload); // Parse the payload into a JSON document
    http.end();
  }

public:
  virtual void setup() {};

  virtual void render() = 0;
};

class PageIconText : public Page {
public:
  const IconSequence *icon;
  char text[128];

  int iconPosX = 30;
  int iconPosY = (SCREEN_HEIGHT - icon->height) / 2;

//  int textPosX = iconPosX + icon->width + 30;
  int textPosX = 250;
  int textPosY = SCREEN_HEIGHT / 2;

  PageIconText(const IconSequence *icon, char *text) :
      icon(icon) {
    strcpy(this->text, text);
  }

  void render() override {
    screen.fillScreen(TFT_BLACK);
    this->drawBitmapGif(iconPosX, iconPosY, icon->frames, icon->width, icon->height, icon->count);
    displayText(this->text, 6, this->textPosX, this->textPosY - 20);
  }
};

class PageWeather : public PageIconText {
public:
  explicit PageWeather() :
      PageIconText(&icon_temp, "") {
  }
  void setup() override {
      DynamicJsonDocument doc(8192);
      Serial.println("[PageWeather] Fetching weather...");
      const char *weatherAPI = "https://data.weather.gov.hk/weatherAPI/opendata/weather.php?dataType=rhrread";

      this->APIGetRequest(weatherAPI, doc);

      int temperature = doc["temperature"]["data"][0]["value"].as<int>();
      int humidity = doc["humidity"]["data"][0]["value"].as<int>();

      snprintf(this->text, sizeof(this->text), "%dc %dp", temperature, humidity);
      Serial.printf("[PageWeather] Temperature: ", temperature, ", Humidity: ", humidity);
  }
};

class PageTime : public PageIconText {
private:
  int gmtOffset = 8; // GMT+8
public:
  explicit PageTime(int textSize) :
      PageIconText(&icon_clock, "00:00") {
    }

  void setup() override {
      Serial.println("[PageTime] Setting up NTP...");
      configTime(this->gmtOffset * 60 * 60, 0, "pool.ntp.org");
  }

  void render() override {
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo)) {
          Serial.println("[PageTime] Unable to obtain time.");
          return;
      }
      Serial.printf("[PageTime] Current time: %s\n", this->text);

      strftime(this->text, sizeof(this->text), "%H:%M", &timeinfo);
      PageIconText::render();
      displayText(this->text, 6, this->textPosX, this->textPosY-20);
      delay(500);
  }
};

// --- Page Initialization ---
Page *pages[] = {
    new PageTime(GMT_OFFSET),
    new PageWeather()
};

Page *pageNoWifi = new PageIconText(&icon_ghost, "No WiFi");

int pageIdx = 0;

void setup()
{
  //  Screen Setup
  screen.init();
  screen.setRotation(1); // Landscape
  Serial.begin(115200);
  screen.fillScreen(TFT_BLACK);

  //  WiFi Setup
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connecting to WiFi...");
    displayText("Connecting to WiFi...", 4, 150, SCREEN_HEIGHT / 2);
    delay(500);
  }

  screen.fillScreen(TFT_BLACK);
  delay(1000);
  for (auto *page : pages)
  {
    page->setup();
  }
}

void loop()
{

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[PageNoWifi] Connecting to WiFi...");
    screen.fillScreen(TFT_BLACK);
    pageNoWifi->render();
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.printf(".");
    }
    for (auto *page : pages)
    {
      page->setup();
    }
  }
  pages[pageIdx]->render();
  pageIdx = (pageIdx + 1) % (sizeof(pages) / sizeof(pages[0]));
  delay(5000);
}
