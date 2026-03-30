#include "mlx_sensor.h"
#include "config.h" // For MLX90640_ADDRESS, HEIGHT, WIDTH, TA_SHIFT

MlxSensor::MlxSensor() : _address(MLX90640_ADDRESS) {
    // Constructor
}

bool MlxSensor::init(uint8_t address, uint8_t sdaPin, uint8_t sclPin, uint32_t i2cSpeed) {
    _address = address;
    Wire.begin(sdaPin, sclPin);
    Wire.setClock(400000); // EEPROM reads at 400kHz, then we switch to desired speed after init
    DEBUG_PRINT("MLX90640 I2C init on SDA:");
    DEBUG_PRINT(sdaPin);
    DEBUG_PRINT(", SCL:");
    DEBUG_PRINT(sclPin);
    DEBUG_PRINT(" at ");
    DEBUG_PRINT(i2cSpeed / 1000);
    DEBUG_PRINTLN("kHz.");

    if (!isConnected()) {
        DEBUG_PRINTLN("MLX90640 not detected at default I2C address. Please check wiring.");
        return false;
    }
    DEBUG_PRINTLN("MLX90640 detected.");

    // Get device parameters - We only have to do this once
    uint16_t eeMLX90640[832];
    int status = MLX90640_DumpEE(_address, eeMLX90640);
    if (status != 0) {
        DEBUG_PRINTLN("Failed to load MLX90640 system parameters.");
        return false;
    }
    DEBUG_PRINTLN("MLX90640 EEPROM dumped.");

    status = MLX90640_ExtractParameters(eeMLX90640, &_mlx90640Params);
    if (status != 0) {
        DEBUG_PRINTLN("MLX90640 parameter extraction failed.");
        return false;
    }
    DEBUG_PRINTLN("MLX90640 parameters extracted.");

    // Set refresh rate (e.g., 1Hz effective)
    MLX90640_SetRefreshRate(_address, 0x02); // 1Hz
    // MLX90640_SetRefreshRate(_address, 0x04); // 4Hz
    DEBUG_PRINTLN("MLX90640 refresh rate set.");

    Wire.setClock(i2cSpeed);

    return true;
}

bool MlxSensor::isConnected() {
    Wire.beginTransmission(_address);
    return (Wire.endTransmission() == 0); // Sensor did ACK
}

bool MlxSensor::readFrame(float* frameBuffer) {
    for (byte x = 0 ; x < 2 ; x++) {
        uint16_t mlx90640Frame[834];
        int status = MLX90640_GetFrameData(_address, mlx90640Frame);

        float vdd = MLX90640_GetVdd(mlx90640Frame, &_mlx90640Params);
        float Ta = MLX90640_GetTa(mlx90640Frame, &_mlx90640Params);
        float tr = Ta - TA_SHIFT; // Reflected temperature based on the sensor ambient temperature
        float emissivity = 0.95;

        MLX90640_CalculateTo(mlx90640Frame, &_mlx90640Params, emissivity, tr, frameBuffer);
    }

    return true;
}