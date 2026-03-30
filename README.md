# CrowdAware

CrowdAware is a privacy-aware crowd monitoring prototype that uses low-resolution thermal sensors (MLX90640, 32x24) and edge processing on ESP32 nodes.

Instead of using RGB video, each node detects moving heat blobs, estimates person centroids, and transmits compact telemetry to a central receiver for live visualization and analytics.

## Why this project

Conventional crowd monitoring systems often trade privacy for visibility. CrowdAware is designed to keep personally identifying information out of the data path while still enabling:

- occupancy awareness
- movement and flow analysis
- congestion monitoring

## Current implementation status

This repository currently includes:

- ESP32 (Heltec WiFi LoRa 32 v3) node firmware for MLX90640 capture and on-device detection
- Wi-Fi UDP telemetry transport (current active path)
- Windows Python parser/visualizer for receiving and rendering node output
- hardware design files for a custom node PCB

The report references LoRaWAN architecture goals. In this codebase, Wi-Fi UDP is the active telemetry path used for development and visualization.

## Repository structure

```
Firmware/
	node/                          # ESP32 + MLX90640 node firmware
Visualization/
	parser_win.py                  # UDP receiver + OpenCV live visualizer
PCB Files/
	Altium Files/                  # Schematic + PCB source
	Gerber Files/                  # Manufacturing outputs
README.md
```

## System architecture

1. Sensor capture
- ESP32 reads thermal frames from MLX90640 (32x24) over I2C.

2. On-node processing
- Convert raw temperatures to 8-bit grayscale.
- Build/update a running background model.
- Detect moving hot regions with a lightweight pipeline.
- Extract person candidates (centroid and area).

3. Telemetry
- Serialize processing outputs and detections into a binary packet.
- Fragment into 4 UDP datagrams with sequence metadata.
- Transmit to the visualization host.

4. Central parsing and visualization
- Reassemble fragments.
- Decode intermediate images and detections.
- Render a 2x2 diagnostic view with overlays.

## Detection pipeline (firmware)

Implemented in the thermal processor:

1. Background subtraction
- Current frame minus running average background.

2. Morphological opening
- 3x3 erosion then 3x3 dilation to reduce noise and split weak bridges.

3. Gaussian blur
- 3x3 blur to smooth local artifacts.

4. Distance transform
- Approximate foreground distance map.

5. Watershed-like region extraction
- Connected-region labeling on thresholded distance map.
- Per-region centroid and area extraction.
- Area filtering via configurable min/max thresholds.

Adaptive behavior:

- Background is initialized over N frames.
- Background updates continue only when no people are detected to reduce contamination.

## Packet protocol

### Binary packet (reassembled payload)

- Header magic: `FE 01 FE 01`
- Payload length: uint16 little-endian
- Payload layout:
	- thermal image (768 bytes)
	- background image (768 bytes)
	- distance map (768 bytes)
	- labeled image (768 bytes)
	- num_detected (1 byte)
	- per detection: `y (1), x (1), area (uint16 LE)`

### UDP fragment packet

- Header magic: `FE 02 FE 02`
- Sequence: uint16 little-endian
- Fragment index: uint8
- Fragment count: uint8 (fixed at 4)
- Payload size: uint16 little-endian
- Payload bytes

Parser behavior:

- Waits for all 4 fragments for a `(sender_ip, sequence)` frame.
- Concatenates fragments in index order.
- Drops stale partial frames after timeout.

## Quick start

### 1) Firmware (Heltec WiFi LoRa 32 v3 node)

Prerequisites:

- Arduino IDE with Heltec WiFi LoRa 32(v3) board support
- MLX90640-compatible libraries required by the firmware

Open and configure:

- Sketch: `Firmware/node/node.ino`
- Main config: `Firmware/node/config.h`

Update at minimum:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `WIFI_REMOTE_IP_A..D` (host running parser)
- `WIFI_REMOTE_PORT` (default `5100`)
- `NODE_DEVICE_ID` (if using multiple nodes)

Upload firmware to ESP32, then open serial monitor (57600 baud) if needed for diagnostics.

### 2) Visualization receiver (Windows)

From repository root:

```powershell
pip install numpy opencv-python
python Visualization/parser_win.py
```

Expected behavior:

- Listener starts on `0.0.0.0:5100`.
- OpenCV window shows a 2x2 panel:
	- Raw image
	- Background
	- Distance map
	- Watershed labels
- Detected blobs and `(x,y)` coordinates are overlaid on the raw panel.

Press `q` to close the visualizer.

## Runtime tuning via serial commands

The firmware accepts newline-terminated commands over serial to tune processing without recompiling.

Supported command format:

```text
SET_DT_BG_THRESHOLD=<int>
SET_DT_MAX_DISTANCE=<int>
SET_MIN_PERSON_AREA=<int>
SET_MAX_PERSON_AREA=<int>
SET_BG_FRAME_COUNT=<int>
SET_TEMP_MIN=<float>
SET_TEMP_MAX=<float>
```

Notes:

- Changing `SET_BG_FRAME_COUNT` triggers background re-initialization.
- `TEMP_RANGE` is derived internally from min/max values.

## Hardware

Node hardware files are provided in:

- `PCB Files/Altium Files/`
- `PCB Files/Gerber Files/`

These include board-level design artifacts for the thermal node platform.

## Known limitations

- Current telemetry path in code is Wi-Fi UDP, not production LoRaWAN.
- Deployment and parameter management still rely on source-level/serial workflows.
- Multi-node global fusion and de-duplication are currently outside this repository's Python visualizer scope.

## Suggested next milestones

- Add a central server service for multi-node ingestion and global coordinate fusion.
- Build an operator/admin UI for node registration, calibration, and parameter control.
- Add replay logging and offline evaluation tools for algorithm benchmarking.

## Team

Innovation Wing IoT Team

- Ilo Japar
- Zhifang Li
- Nuth Kitchongcharoenying
- Netitorn Kawmali
- Pitimontreekul Chalida
