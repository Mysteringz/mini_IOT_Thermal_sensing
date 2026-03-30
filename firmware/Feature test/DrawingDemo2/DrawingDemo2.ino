#include <Wire.h>  
#include "HT_SSD1306Wire.h"
#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

#ifdef WIRELESS_STICK_V3
static SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED); // addr , freq , i2c group , resolution , rst
#else
static SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst
#endif

#define TA_SHIFT 8 //Default shift for MLX90640 in open air
const uint8_t MLX90640_address = 0x33;
float mlx90640To[768];
paramsMLX90640 mlx90640;

boolean isConnected()
{
  Wire.beginTransmission((uint8_t)MLX90640_address);
  if (Wire.endTransmission() != 0)
    return (false); //Sensor did not ACK
  return (true);
}

void drawLines() {
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(0, 0, i, display.getHeight()-1);
    display.display();
    delay(10);
  }
  for (int16_t i=0; i<display.getHeight(); i+=4) {
    display.drawLine(0, 0, display.getWidth()-1, i);
    display.display();
    delay(10);
  }
  delay(250);

  display.clear();
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(0, display.getHeight()-1, i, 0);
    display.display();
    delay(10);
  }
  for (int16_t i=display.getHeight()-1; i>=0; i-=4) {
    display.drawLine(0, display.getHeight()-1, display.getWidth()-1, i);
    display.display();
    delay(10);
  }
  delay(250);

  display.clear();
  for (int16_t i=display.getWidth()-1; i>=0; i-=4) {
    display.drawLine(display.getWidth()-1, display.getHeight()-1, i, 0);
    display.display();
    delay(10);
  }
  for (int16_t i=display.getHeight()-1; i>=0; i-=4) {
    display.drawLine(display.getWidth()-1, display.getHeight()-1, 0, i);
    display.display();
    delay(10);
  }
  delay(250);
  display.clear();
  for (int16_t i=0; i<display.getHeight(); i+=4) {
    display.drawLine(display.getWidth()-1, 0, 0, i);
    display.display();
    delay(10);
  }
  for (int16_t i=0; i<display.getWidth(); i+=4) {
    display.drawLine(display.getWidth()-1, 0, i, display.getHeight()-1);
    display.display();
    delay(10);
  }
  delay(250);
}

// Adapted from Adafruit_SSD1306
void drawRect(void) {
  for (int16_t i=0; i<display.getHeight()/2; i+=2) {
    display.drawRect(i, i, display.getWidth()-2*i, display.getHeight()-2*i);
    display.display();
    delay(10);
  }
}

// Adapted from Adafruit_SSD1306
void fillRect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<display.getHeight()/2; i+=3) {
    display.setColor((color % 2 == 0) ? BLACK : WHITE); // alternate colors
    display.fillRect(i, i, display.getWidth() - i*2, display.getHeight() - i*2);
    display.display();
    delay(10);
    color++;
  }
  // Reset back to WHITE
  display.setColor(WHITE);
}

// Adapted from Adafruit_SSD1306
void drawCircle(void) {
  for (int16_t i=0; i<display.getHeight(); i+=2) {
    display.drawCircle(display.getWidth()/2, display.getHeight()/2, i);
    display.display();
    delay(10);
  }
  delay(1000);
  display.clear();

  // This will draw the part of the circel in quadrant 1
  // Quadrants are numberd like this:
  //   0010 | 0001
  //  ------|-----
  //   0100 | 1000
  //
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000001);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000011);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00000111);
  display.display();
  delay(200);
  display.drawCircleQuads(display.getWidth()/2, display.getHeight()/2, display.getHeight()/4, 0b00001111);
  display.display();
}

void printBuffer(void) {
  // Initialize the log buffer
  // allocate memory to store 8 lines of text and 30 chars per line.
  display.setLogBuffer(2, 30);

  // Some test data
  const char* test[] = {
      "Hello",
      "World" ,
      "----",
      "Show off",
      "how",
      "the log buffer",
      "is",
      "working.",
      "Even",
      "scrolling is",
      "working"
  };

  for (uint8_t i = 0; i < 11; i++) {
    display.clear();
    // Print to the screen
    display.println(test[i]);
    // Draw it to the internal screen buffer
    display.drawLogBuffer(0, 0);
    // Display it on the screen
    display.display();
    delay(500);
  }
}

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

void setup() {
  if (isConnected() == false)
  {
    Serial.println("MLX90640 not detected at default I2C address. Please check wiring. Freezing.");
    while (1);
  }
  pinMode(LED_BUILTIN, OUTPUT);
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
  VextON();
  delay(100);

  display.init();
  display.clear();
  display.display();
  
  display.setContrast(255);

  drawLines();
  delay(1000);
  display.clear();

  drawRect();
  delay(1000);
  display.clear();

  fillRect();
  delay(1000);
  display.clear();

  drawCircle();
  delay(1000);
  display.clear();
  
  printBuffer();
  delay(1000);

  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.clear();
  display.display();
  display.screenRotate(ANGLE_0_DEGREE);
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth()/2, display.getHeight()/2-16/2, "ROTATE_0");
  display.display();
  delay(2000);

  display.clear();
  display.display();
  display.screenRotate(ANGLE_90_DEGREE);
  display.setFont(ArialMT_Plain_10);
  display.drawString(display.getWidth()/2, display.getHeight()/2-10/2, "ROTATE_90");
  display.display();
  delay(2000);

  display.clear();
  display.display();
  display.screenRotate(ANGLE_180_DEGREE);
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth()/2, display.getHeight()/2-16/2, "ROTATE_180");
  display.display();
  delay(2000);

  display.clear();
  display.display();
  display.screenRotate(ANGLE_270_DEGREE);
  display.setFont(ArialMT_Plain_10);
  display.drawString(display.getWidth()/2, display.getHeight()/2-10/2, "ROTATE_270");
  display.display();
  delay(2000);

}

void loop() { }