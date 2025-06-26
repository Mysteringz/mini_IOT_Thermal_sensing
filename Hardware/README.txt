Hardware Architecture: Has an esp32 as a WiFi server to remotely flash code, has multiple esp32 for actual sensing

Files:
plain_thermal_node.ino for thermal detection
NODE_update_server.ino is for updating the ESP32 WiFi server
rgc_thermal_node.ino has thermal detection and working RGB lights

Requires use of the Heltec WiFi LoRa 32(V3) board, download zip from:
https://github.com/HelTecAutomation/Heltec_ESP32/tree/master

Move unzipped Heltec folder (Heltec_ESP32-master) into ~/Arduino/libraries
In Arduino IDE, go to Settings > Additional boards manager URLs: 
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
(make sure nothing else is pasted) and press OK


* The Heltec library contains syntax errors, fix the following:
In sx126x.c:
	line 9: Change 
		#include “Arduino.h”
		To
 		#include "Arduino.h"
In sx1262-board.c:
	line 34:Change 
		extern void lora_printf(const char *format, …);
		To
		extern void lora_printf(const char *format, ...);

Required Installations from Arduino IDE:
- U8g2
- XPowersLib

Use Tools > LoRaWAN Region > REGION_915

For Innowing, use WiFi RAK7268_7ABD
Connect to IP address: 192.168.230.1
(Password is root)

If unable to detect camera, wiggle the camera around and press RST. Repeat until it works.

Change credentials (devEui, AppEUI, and appKey) on line 115 of the .ino file to the credentials extracted from The Things Network. These are specific to your model of ESP32
