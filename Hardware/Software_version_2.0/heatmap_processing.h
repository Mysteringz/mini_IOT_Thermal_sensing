#pragma once
#include <vector>
#include <utility>

void backgroundEstimation();
void updateHeatmap(const float (&background_median)[HEIGHT][WIDTH]);
