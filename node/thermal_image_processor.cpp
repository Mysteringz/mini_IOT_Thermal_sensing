#include "thermal_image_processor.h"
#include <string.h> // For memset
#include <algorithm> // For std::min, std::max
#include "serial_comms.h" // To access global configurable parameters

// Helper function to constrain a value within a range
template <typename T>
T constrain_val(T val, T min_val, T max_val) {
    return std::max(min_val, std::min(val, max_val));
}

// Helper function for mapping a value from one range to another
long map_val(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void thermal_processor_init(ThermalProcessor* processor) {
    memset(processor->raw_frame, 0, sizeof(processor->raw_frame));
    memset(processor->thermal_image, 0, sizeof(processor->thermal_image));
    // Initialize all buffers to zero
    memset(processor->background, 0, sizeof(processor->background));
    memset(processor->work_buffer, 0, sizeof(processor->work_buffer));
    memset(processor->labeled_image, 0, sizeof(processor->labeled_image));
    memset(processor->distance_map, 0, sizeof(processor->distance_map));
    memset(processor->detected_people, 0, sizeof(processor->detected_people));
    processor->num_detected = 0;
    processor->bg_frames_collected = 0;
    processor->background_ready = false;
}

void convert_to_8bit_image(const float* thermal_frame, uint8_t image[IMAGE_HEIGHT][IMAGE_WIDTH]) {
    // Ensure TEMP_RANGE is valid to prevent division by zero
    float current_temp_range = ::TEMP_RANGE;
    if (current_temp_range <= 0.0f) {
        current_temp_range = 0.1f; // Use a small positive value if range is invalid
    }

    for (int i = 0; i < IMAGE_SIZE; ++i) {
        float temp = thermal_frame[i];
        // Clamp temperature to the defined range
        temp = constrain_val(temp, ::TEMP_MIN, ::TEMP_MAX);
        
        // Map temperature to 0-255
        // (temp - TEMP_MIN) / TEMP_RANGE gives a 0.0-1.0 normalized value
        image[i / IMAGE_WIDTH][i % IMAGE_WIDTH] = (uint8_t)(((temp - ::TEMP_MIN) / current_temp_range) * 255.0f);
    }
}

void thermal_processor_update_thermal_image(ThermalProcessor* processor) {
    convert_to_8bit_image(processor->raw_frame, processor->thermal_image);
}

void thermal_processor_reset_background(ThermalProcessor* processor) {
    memset(processor->background, 0, sizeof(processor->background));
    processor->bg_frames_collected = 0;
    processor->background_ready = false;
    processor->num_detected = 0;
}

uint8_t thermal_processor_step(ThermalProcessor* processor, uint8_t max_people) {
    if (!processor->background_ready) {
        const uint8_t init_alpha = 25;
        update_background(processor, processor->thermal_image, init_alpha);
        processor->bg_frames_collected++;
        processor->num_detected = 0;

        if (processor->bg_frames_collected >= (uint16_t)BG_FRAME_COUNT) {
            processor->background_ready = true;
        }

        return 0;
    }

    const uint8_t num_detected = process_thermal_frame(processor, max_people);

    // Update background only when no detections to avoid contamination.
    if (num_detected == 0) {
        const uint8_t track_alpha = 5;
        update_background(processor, processor->thermal_image, track_alpha);
    }

    return num_detected;
}

void update_background(ThermalProcessor* processor, const uint8_t current_frame[IMAGE_HEIGHT][IMAGE_WIDTH], uint8_t alpha) {
    // Exponential Moving Average: new_bg = (alpha * current_frame + (255 - alpha) * old_bg) / 255
    for (int r = 0; r < IMAGE_HEIGHT; ++r) {
        for (int c = 0; c < IMAGE_WIDTH; ++c) {
            processor->background[r][c] = (uint8_t)(((long)alpha * current_frame[r][c] + (255L - alpha) * processor->background[r][c]) / 255L);
        }
    }
}

void subtract_frames(const ThermalProcessor* processor, const uint8_t current_frame[IMAGE_HEIGHT][IMAGE_WIDTH], uint8_t output[IMAGE_HEIGHT][IMAGE_WIDTH]) {
    for (int r = 0; r < IMAGE_HEIGHT; ++r) {
        for (int c = 0; c < IMAGE_WIDTH; ++c) {
            // Ensure result is non-negative
            int diff = (int)current_frame[r][c] - (int)processor->background[r][c];
            output[r][c] = (uint8_t)std::max(0, diff);
        }
    }
}

void erode_3x3(const uint8_t input[IMAGE_HEIGHT][IMAGE_WIDTH], uint8_t output[IMAGE_HEIGHT][IMAGE_WIDTH]) {
    for (int r = 0; r < IMAGE_HEIGHT; ++r) {
        for (int c = 0; c < IMAGE_WIDTH; ++c) {
            uint8_t min_val = 255;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    int nr = r + dr;
                    int nc = c + dc;
                    if (nr >= 0 && nr < IMAGE_HEIGHT && nc >= 0 && nc < IMAGE_WIDTH) {
                        min_val = std::min(min_val, input[nr][nc]);
                    }
                }
            }
            output[r][c] = min_val;
        }
    }
}

void dilate_3x3(const uint8_t input[IMAGE_HEIGHT][IMAGE_WIDTH], uint8_t output[IMAGE_HEIGHT][IMAGE_WIDTH]) {
    for (int r = 0; r < IMAGE_HEIGHT; ++r) {
        for (int c = 0; c < IMAGE_WIDTH; ++c) {
            uint8_t max_val = 0;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    int nr = r + dr;
                    int nc = c + dc;
                    if (nr >= 0 && nr < IMAGE_HEIGHT && nc >= 0 && nc < IMAGE_WIDTH) {
                        max_val = std::max(max_val, input[nr][nc]);
                    }
                }
            }
            output[r][c] = max_val;
        }
    }
}

void gaussian_blur_3x3(const uint8_t input[IMAGE_HEIGHT][IMAGE_WIDTH], uint8_t output[IMAGE_HEIGHT][IMAGE_WIDTH]) {
    // 3x3 Gaussian kernel (approximate):
    // 1  2  1
    // 2  4  2
    // 1  2  1
    // Sum = 16
    const int kernel[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
    };

    for (int r = 0; r < IMAGE_HEIGHT; ++r) {
        for (int c = 0; c < IMAGE_WIDTH; ++c) {
            long sum = 0;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    int nr = r + dr;
                    int nc = c + dc;
                    if (nr >= 0 && nr < IMAGE_HEIGHT && nc >= 0 && nc < IMAGE_WIDTH) {
                        sum += (long)input[nr][nc] * kernel[dr + 1][dc + 1];
                    } else {
                        // Handle borders by replicating edge pixels
                        sum += (long)input[r][c] * kernel[dr + 1][dc + 1];
                    }
                }
            }
            output[r][c] = (uint8_t)(sum / GAUSSIAN_KERNEL_3X3_SCALE);
        }
    }
}


// Function to compute Euclidean distance transform (simplified)
// This is an approximation, a full EDT is more complex.
// This version computes distance to nearest background pixel (value 0).
void distance_transform(const uint8_t input[IMAGE_HEIGHT][IMAGE_WIDTH], uint8_t output[IMAGE_HEIGHT][IMAGE_WIDTH]) {
    // Initialize output: 0 for background, large value for foreground
    for (int r = 0; r < IMAGE_HEIGHT; ++r) {
        for (int c = 0; c < IMAGE_WIDTH; ++c) {
            if (input[r][c] > ::DT_BG_THRESHOLD) { // Use dynamic threshold
                output[r][c] = 255; // Max distance for foreground pixels
            } else {
                output[r][c] = 0; // Background
            }
        }
    }

    // First pass (forward scan)
    for (int r = 0; r < IMAGE_HEIGHT; ++r) {
        for (int c = 0; c < IMAGE_WIDTH; ++c) {
            if (output[r][c] > 0) { // If it's a foreground pixel
                // Check neighbors (left and top)
                uint8_t min_neighbor_dist = 255;
                if (c > 0) min_neighbor_dist = std::min(min_neighbor_dist, (uint8_t)(output[r][c-1] + 1));
                if (r > 0) min_neighbor_dist = std::min(min_neighbor_dist, (uint8_t)(output[r-1][c] + 1));
                output[r][c] = std::min(output[r][c], min_neighbor_dist);
            }
        }
    }

    // Second pass (backward scan)
    for (int r = IMAGE_HEIGHT - 1; r >= 0; --r) {
        for (int c = IMAGE_WIDTH - 1; c >= 0; --c) {
            if (output[r][c] > 0) { // If it's a foreground pixel
                // Check neighbors (right and bottom)
                uint8_t min_neighbor_dist = 255;
                if (c < IMAGE_WIDTH - 1) min_neighbor_dist = std::min(min_neighbor_dist, (uint8_t)(output[r][c+1] + 1));
                if (r < IMAGE_HEIGHT - 1) min_neighbor_dist = std::min(min_neighbor_dist, (uint8_t)(output[r+1][c] + 1));
                output[r][c] = std::min(output[r][c], min_neighbor_dist);
            }
        }
    }
}


// Simple Connected Components / Watershed-like approach
uint8_t watershed(const uint8_t distance_map[IMAGE_HEIGHT][IMAGE_WIDTH], 
                  ThermalProcessor* processor,
                  uint8_t max_people) {
    memset(processor->labeled_image, 0, sizeof(processor->labeled_image)); // Clear labels
    uint8_t current_label = 0;
    uint8_t num_detected = 0;

    // Use a simple queue for flood fill (BFS)
    struct Pixel {
        uint8_t r, c;
    };
    Pixel queue[IMAGE_SIZE]; // Max possible queue size
    int queue_head = 0;
    int queue_tail = 0;

    for (int r = 0; r < IMAGE_HEIGHT; ++r) {
        for (int c = 0; c < IMAGE_WIDTH; ++c) {
            // Find a pixel that is part of a potential person (high distance value)
            // and has not been labeled yet.
            if (distance_map[r][c] > ::DT_MAX_DISTANCE && processor->labeled_image[r][c] == 0) { // Use dynamic threshold
                if (current_label < max_people) {
                    current_label++; // New person detected
                    num_detected++;

                    DetectedPerson new_person = {0, 0, 0, 0};
                    long sum_x = 0;
                    long sum_y = 0;
                    uint16_t pixel_count = 0;
                    uint8_t max_dist_in_region = 0;

                    // Start flood fill (BFS) from this pixel
                    queue[queue_tail++] = {r, c};
                    processor->labeled_image[r][c] = current_label;

                    while (queue_head < queue_tail) {
                        Pixel p = queue[queue_head++];

                        sum_x += p.c;
                        sum_y += p.r;
                        pixel_count++;
                        max_dist_in_region = std::max(max_dist_in_region, distance_map[p.r][p.c]);

                        // Check 8-connected neighbors
                        for (int dr = -1; dr <= 1; ++dr) {
                            for (int dc = -1; dc <= 1; ++dc) {
                                if (dr == 0 && dc == 0) continue; // Skip self

                                int nr = p.r + dr;
                                int nc = p.c + dc;

                                if (nr >= 0 && nr < IMAGE_HEIGHT && nc >= 0 && nc < IMAGE_WIDTH &&
                                    distance_map[nr][nc] > ::DT_MAX_DISTANCE && // Only extend to pixels above threshold
                                    processor->labeled_image[nr][nc] == 0) {
                                    
                                    processor->labeled_image[nr][nc] = current_label;
                                    queue[queue_tail++] = {(uint8_t)nr, (uint8_t)nc};
                                }
                            }
                        }
                    }

                    // Calculate centroid and store person data
                    if (pixel_count > 0) {
                        new_person.x = (uint8_t)(sum_x / pixel_count);
                        new_person.y = (uint8_t)(sum_y / pixel_count);
                        new_person.area = pixel_count;
                        new_person.max_distance = max_dist_in_region;

                        // Filter by area using dynamic thresholds
                        if (new_person.area >= ::MIN_PERSON_AREA && new_person.area <= ::MAX_PERSON_AREA) {
                            processor->detected_people[current_label - 1] = new_person;
                        } else {
                            // If filtered out, decrement num_detected and reset label
                            num_detected--;
                        }
                    } else {
                        // Should not happen if pixel_count > 0 for starting pixel
                        num_detected--;
                    }
                    
                    queue_head = 0; // Reset queue for next detection
                    queue_tail = 0;
                }
            }
        }
    }
    return num_detected;
}


uint8_t process_thermal_frame(ThermalProcessor* processor, uint8_t max_people) {
    // Step 1: Background subtraction
    subtract_frames(processor, processor->thermal_image, processor->work_buffer);

    // Step 2: Morphological opening (Erode then Dilate) to remove noise
    erode_3x3(processor->work_buffer, processor->work_buffer);
    dilate_3x3(processor->work_buffer, processor->work_buffer);

    // Step 3: Gaussian blur for smoothing
    gaussian_blur_3x3(processor->work_buffer, processor->work_buffer);

    // Step 4: Distance Transform
    distance_transform(processor->work_buffer, processor->distance_map);

    // Step 5: Watershed segmentation
    processor->num_detected = watershed(processor->distance_map, processor, max_people);
    return processor->num_detected;
}