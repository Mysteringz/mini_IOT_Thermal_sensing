## Thermal Node Rev2.4
_Complete thermal node implementation_

Required Libraries:
- Adafruit BusIO
- Adafruit GFX
- U8g2
- XPowersLib
- MKRWAN
- FastLED

Requires use of the Heltec WiFi LoRa 32(V3) board, download zip from:
https://github.com/HelTecAutomation/Heltec_ESP32/tree/master  
Move unzipped Heltec folder (Heltec_ESP32-master) into ~/Arduino/libraries  

Use Tools > LoRaWAN Region > REGION_US915
If you are unable to upload the code, please try 460800 upload speed.

For Innowing, use WiFi RAK7268_7ABD
Connect to IP address: 192.168.230.1
(Password is root)

> Rev2_4_thermal_node/  
├── src/  
│   ├── libs/  
│   │   ├── MLX90640_API.cpp  
│   │   ├── MLX90640_API.h  
│   │   ├── MLX90640_I2C_Device.cpp  
│   │   ├── MLX90640_I2C_Device.h  
│   ├── Camera_algorithm.cpp  
│   ├── Camera_algorithm.h  
│   ├── User_config.h  
│   ├── Web_interface.cpp  
│   └── Web_interface.h  
├── README.md  
└── Rev2_4_thermal_node.ino

To edit:
- User_config.h for average room temp and
- Web_interface.cpp for website style