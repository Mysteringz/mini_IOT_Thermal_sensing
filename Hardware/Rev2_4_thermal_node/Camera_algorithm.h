//
// Created by Zhifang L on 04/03/2026.
//

#ifndef MINI_IOT_THERMAL_SENSING_CAMERA_ALGORITHM_H
#define MINI_IOT_THERMAL_SENSING_CAMERA_ALGORITHM_H

#include <Arduino.h>
#include <vector>
#include <stack>
#include <algorithm>

#include "Data_packet.h"
#include "MLX90640_API.h"

using namespace std;

// MLX90640 configuration
#define MLX90640_address 0x33

// Image dimensions
#define HEIGHT 24
#define WIDTH 32

// Algorithm parameters
#define THRESHOLD_DIFF_VALUE 1.3        // TO BE ADJUST !!!
#define THRESHOLD_TEMP_VALUE 23.0       // TO BE ADJUST !!!
#define MIN_COMPONENT_SIZE 5
#define BG_FRAME_COUNT 25
#define KERNEL_SIZE 3
#define TA_SHIFT 8

// Pin definitions
extern const uint8_t calcStart;

// Global variables
extern float mlx90640To[HEIGHT * WIDTH];
extern paramsMLX90640 mlx90640;
extern float background_median[HEIGHT][WIDTH];
extern float temperatureFrames[HEIGHT][WIDTH];
extern float smoothedData[HEIGHT][WIDTH];
extern float subtractedFrame[HEIGHT][WIDTH];
extern const int deviceid;
extern uint8_t appData[];
extern uint8_t appDataSize;

// Data structure for WiFi transmission
extern struct_message myData;

// MLX90640 functions (these should be declared elsewhere in your code)
int MLX90640_GetFrameData(uint8_t slaveAddr, uint16_t *frameData);
float MLX90640_GetVdd(uint16_t *frameData, struct mlx90640 *mlx90640);
float MLX90640_GetTa(uint16_t *frameData, struct mlx90640 *mlx90640);
void MLX90640_CalculateTo(uint16_t *frameData, struct mlx90640 *mlx90640, float emissivity, float tr, float *result);

// Core algorithm functions
void updateHeatmap(const float (&background_median)[HEIGHT][WIDTH]);
void backgroundEstimation();
void readTemperatureData2D(float (&output)[HEIGHT][WIDTH]);
void smoothImage(const float (&input)[HEIGHT][WIDTH], float (&output)[HEIGHT][WIDTH]);
vector<vector<float>> createPaddedArray(const float (&input)[HEIGHT][WIDTH]);
void applyBackgroundSubtraction(const float (&background)[HEIGHT][WIDTH], const float (&input)[HEIGHT][WIDTH], float (&output)[HEIGHT][WIDTH]);
void applyThreshold(float (&tempData)[HEIGHT][WIDTH]);

// Connected component functions
vector<pair<int, int>> findComponentCenters(const float (&thresholdData)[HEIGHT][WIDTH]);
void dfs(const int x, const int y, const float (&thresholdData)[HEIGHT][WIDTH], bool (&visited)[HEIGHT][WIDTH], vector<pair<int, int> > &component);


#endif //MINI_IOT_THERMAL_SENSING_CAMERA_ALGORITHM_H
