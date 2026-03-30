#ifndef MLX_SENSOR_H
#define MLX_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include "MLX90640_API.h"      // External library for MLX90640
#include "MLX90640_I2C_Driver.h" // External library for MLX90640 I2C driver
#include "config.h"            // For MLX90640_ADDRESS, HEIGHT, WIDTH, TA_SHIFT

class MlxSensor {
public:
    MlxSensor();
    bool init(uint8_t address, uint8_t sdaPin, uint8_t sclPin, uint32_t i2cSpeed);
    bool isConnected();
    bool readFrame(float* frameBuffer); // Reads a single frame into a flat array

private:
    uint8_t _address;
    paramsMLX90640 _mlx90640Params; // Stores MLX90640 parameters
};

#endif // MLX_SENSOR_H