#include "heatmap_processing.h"
#include "constants.h"
#include <Wire.h>
#include <string>
#include <vector>
#include <stack>
#include <utility>

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

void updateHeatmap(const float (&background_median)[HEIGHT][WIDTH]) {

    readTemperatureData2D(temperatureFrames);

    smoothImage(temperatureFrames, smoothedData);

    applyBackgroundSubtraction(background_median, smoothedData, subtractedFrame);

    applyThreshold(subtractedFrame);

    vector<pair<int, int>> centerIndexOfComponents = findComponentCenters(subtractedFrame);

//    Serial.print("Number of components is ");
//    Serial.println(centerIndexOfComponents.size());
    appDataSize = 0;
    appData[appDataSize++] = deviceid;
    if (centerIndexOfComponents.size() != 0) {
      for (auto const& component : centerIndexOfComponents) {
          Serial.print(component.first);
          Serial.print(",");
          Serial.print(component.second);
          Serial.print(";");
          appData[appDataSize++] = component.first;
          appData[appDataSize++] = component.second;
      }
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