import time
import random
import json
import ssl
from collections import deque
from dataclasses import dataclass, field
from typing import List, Tuple

import cv2
import numpy as np
from paho.mqtt import client as mqtt_client

# MQTT CONFIGURATION -- FILL THESE WITH OUR VALUES
BROKER = 'as1.cloud.thethings.industries'
PORT = 8883
USERNAME = 'thermal-detection-lite@innowingiot' #iot 
PASSWORD = 'NNSXS.OUN72UG5JKTYAR5PNQIS5SCUAPRAH2OHVSRENRI.XA35FFYUPVBBLFU3HDRW5EZJBF4BMBUK5K637TQ6T7Q3R6M2HWYQ' #API / PASSWORD
TOPIC = f'v3/{USERNAME}/devices/+/up' # receiving messages from all devices (iot-lite-1 to iot-lite-8)
CLIENT_ID = f"mqtt_viz_{random.randint(0, 1e4)}"

# TRACKING AND VISUALIZATION CONFIG
EDGE = 3
FRAME_SIZE = (960, 540)
GRID_SIZE = ((76 + 32) + (2 * EDGE), (38 + 14) + (2 * EDGE))
# BACKGROUND = cv2.imread("floor_plan.jpeg")
BACKGROUND = np.ones((FRAME_SIZE[1], FRAME_SIZE[0], 3), dtype=np.uint8) * 255
CHECKPOINTS_STARTING_COORDINATES = {
    1: (22 + EDGE, 0 + EDGE),
    4: (54 + EDGE, 0 + EDGE),   # id 1 and 4 overlapped by x=10
    5: (76 + EDGE, 0 + EDGE),   # id 4 and 5 overlapped by x=10
    8: (0 + EDGE, 14 + EDGE),
    3: (32 + EDGE, 24 + EDGE),   
    2: (54 + EDGE, 24 + EDGE),  # id 1 and 3 overlapped by y=10
    6: (76 + EDGE, 24 + EDGE),
    9: (32 + EDGE, 38 + EDGE),
    7: (76 + EDGE, 38 + EDGE),
}
OVERLAP_AREA = {
    1: (0, 0),
    4: (11, 0),
    5: (0, 0),
    3: (11, 11),
    2: (11, 11),
    6: (0, 11),
    7: (0, 11),
    8: (0, 0),
}

@dataclass
class Checkpoint:
    id: int
    coordinates: List[Tuple[int, int]] = field(default_factory=list)
    color: Tuple[int, int, int] = (random.randint(64,255), random.randint(64,255), random.randint(64,255))
    radius: int = 2
    starting_coordinates: Tuple[int, int] = (0, 0)

    def __post_init__(self):
        self.starting_coordinates = CHECKPOINTS_STARTING_COORDINATES.get(self.id, (0, 0))


class Visualizer:
    def __init__(self, frame_size=FRAME_SIZE, grid_size=GRID_SIZE, background=BACKGROUND):
        # Heatmap visualization
        self.frame_size = frame_size
        self.grid_size = grid_size
        self.scale_x = frame_size[0] / grid_size[0]
        self.scale_y = frame_size[1] / grid_size[1]
        self.last_update_coord = 0
        self.checkpoints = {}
        self.heatmap = np.zeros((frame_size[1], frame_size[0], 3), dtype=np.uint8)
        self.background = cv2.resize(background, (self.frame_size[0], self.frame_size[1]))

        self._draw_grid_background()
        self._init_checkpoints()

    def  _init_checkpoints(self):
        checkpoint_color = {
            1: (0, 255, 0),     # Green
            2: (255, 0, 0),     # Blue
            3: (0, 0, 255),     # Red
            7: (255, 69, 0),    # Orange Red
            5: (255, 0, 255),   # Magenta
            6: (0, 255, 255),   # Cyan
            4: (200, 200, 200), # Gray
            8: (128, 0, 128),   # Purple
        }

        for id, color in checkpoint_color.items():
            self.checkpoints[id] = Checkpoint(id, color=color)

    def _draw_grid_background(self):
        vertical_lines = [
            EDGE,
            CHECKPOINTS_STARTING_COORDINATES[1][0], 
            CHECKPOINTS_STARTING_COORDINATES[3][0], 
            CHECKPOINTS_STARTING_COORDINATES[4][0],
            CHECKPOINTS_STARTING_COORDINATES[7][0], 
            CHECKPOINTS_STARTING_COORDINATES[5][0],
            self.grid_size[0] - EDGE,
        ]
        horizontal_lines = [
            EDGE, 
            CHECKPOINTS_STARTING_COORDINATES[8][1],
            CHECKPOINTS_STARTING_COORDINATES[3][1], 
            CHECKPOINTS_STARTING_COORDINATES[9][1], 
            self.grid_size[1] - EDGE,
        ]

        edge_x = self._scale_x(EDGE)
        edge_y = self._scale_y(EDGE)

        # Draw vertical lines
        for i, x in enumerate(vertical_lines):
            x = self._scale_x(x)
            if i == 0:
                cv2.line(self.background, (x, self._scale_y(horizontal_lines[1])), (x, self._scale_y(horizontal_lines[3])), (200, 200, 200), 2)
            elif i == 1:
                cv2.line(self.background, (x, edge_y), (x, self._scale_y(horizontal_lines[2])), (200, 200, 200), 2)
            elif i == 2:
                cv2.line(self.background, (x, self._scale_y(horizontal_lines[1])), (x, self._scale_y(horizontal_lines[3])), (200, 200, 200), 2)
            elif i == 6:
                cv2.line(self.background, (x, edge_y), (x, self.frame_size[1] - edge_y), (200, 200, 200), 2)
            elif i == 4:
                cv2.line(self.background, (x, self._scale_y(horizontal_lines[-2])), (x, self._scale_y(horizontal_lines[-1])), (200, 200, 200), 2)
            else:
                cv2.line(self.background, (x, edge_y), (x, self._scale_y(horizontal_lines[3])), (200, 200, 200), 2)

        # horizontal lines
        for i, y in enumerate(horizontal_lines):
            y = self._scale_y(y)
            if i == 0 or i == 2:
                cv2.line(self.background, (self._scale_x(vertical_lines[1]), y), (self.frame_size[0] - edge_x, y), (200, 200, 200), 2)
            elif i == 3:
                cv2.line(self.background, (edge_x, y), (self.frame_size[0] - edge_x, y), (200, 200, 200), 2)
            elif i == 1:
                cv2.line(self.background, (edge_x, y), (self._scale_x(vertical_lines[2]), y), (200, 200, 200), 2)
            elif i == len(horizontal_lines) - 1:
                cv2.line(self.background, (self._scale_x(vertical_lines[4]), y), (self._scale_x(vertical_lines[-1]), y), (200, 200, 200), 2)

    def _scale_x(self, x):
        return int(x * self.scale_x)
    
    def _scale_y(self, y):
        return int(y * self.scale_y)

    def update_coordinates(self, checkpoint_id, coordinates):
        # Update the coordinates of the checkpoint
        if checkpoint_id in CHECKPOINTS_STARTING_COORDINATES:
            checkpoint = self.checkpoints[checkpoint_id]
            checkpoint.coordinates = [(x, abs(24-y)) for (x, y) in coordinates if x >= OVERLAP_AREA[checkpoint_id][0] and abs(24-y) >= OVERLAP_AREA[checkpoint_id][1]]
            print(f"Updated coordinates for checkpoint {checkpoint_id}: {checkpoint.coordinates}")
        else:
            print(f"Checkpoint {checkpoint_id} not found.")
    
    def draw_checkpoints(self):
        # Draw the checkpoints on the transparent layer
        heatmap = self.background.copy()
        for checkpoint in self.checkpoints.values():
            color = checkpoint.color
            radius = int(checkpoint.radius * self.scale_x)
            start_x, start_y = checkpoint.starting_coordinates
            overlap_x, overlap_y = OVERLAP_AREA.get(checkpoint.id, (0, 0))

            for x, y in checkpoint.coordinates:
                new_x = x - overlap_x
                new_y = y - overlap_y
                cx = int((new_x + start_x) * self.scale_x)
                cy = int((new_y + start_y) * self.scale_y)
                cv2.circle(heatmap, (cx, cy), radius, color, -1, cv2.LINE_AA)
                cv2.putText(heatmap, f"({new_x}, {new_y})", (cx-50, cy), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 1, cv2.LINE_AA)
                print(f"Drawing checkpoint {checkpoint.id} at ({cx}, {cy}) equals to ({x}, {y}) or ({new_x + start_x}, {new_y + start_y})")

        return heatmap
            
    def update_heatmap(self):
        self.heatmap = self.draw_checkpoints()
        frame = cv2.addWeighted(self.background, 0.5, self.heatmap, 0.5, 0)

        # Add checkpoint labels
        for checkpoint in self.checkpoints.values():
            color = checkpoint.color
            start_x, start_y = checkpoint.starting_coordinates
            cx = int((start_x) * self.scale_x)
            cy = int((start_y) * self.scale_y)
            cv2.putText(
                frame, 
                f"Checkpoint {checkpoint.id}", 
                (cx, cy), 
                cv2.FONT_HERSHEY_SIMPLEX, 
                0.7, 
                color, 
                2, 
                cv2.LINE_AA
            )

        return frame

    def run_visualization(self):
        # Set up the visualization window
        window_name = "IoT Occupancy Tracking"
        cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
        cv2.resizeWindow(window_name, self.frame_size[0], self.frame_size[1])

        try:
            while True:
                current_time = time.time()
                if current_time - self.last_update_coord >= 5:
                    frame = self.update_heatmap()
                    self.last_update_coord = current_time
                    cv2.imshow(window_name, frame)

                if cv2.waitKey(50) == 27:  # ESC to quit
                    break
        except KeyboardInterrupt:
            print("\nExiting...")
        finally:
            cv2.destroyAllWindows()


def parse_mqtt_payload(msg):
    """
    Extracts the decoded_payload from a full TTN MQTT message.
    Returns (checkpoint_id, [(x1, y1), (x2, y2), ...])
    """
    try:
        data = json.loads(msg)
        payload = data.get("uplink_message", {}).get("decoded_payload", {})
        decoded = payload.get("decoded", [])
        checkpoint_id = payload.get("id", None)
        points = list(zip(decoded[::2], decoded[1::2])) if decoded else []
        return checkpoint_id, points
    except Exception as e:
        print("Parse error:", e)
        return None, []

def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print("Connected to TTN MQTT Broker!")
        client.subscribe(TOPIC)
        print("Subscribed to:", TOPIC)
    else:
        print(f"Failed to connect, return code {rc}")

def on_message(client, userdata, msg):
    print(f"Received message on device {msg}")
    payload = msg.payload.decode()
    checkpoint_id, points = parse_mqtt_payload(payload)
    if checkpoint_id is not None:
        userdata['visualizer'].update_coordinates(checkpoint_id, points)
        # userdata['visualizer'].update_human_count()

def main():
    visualizer = Visualizer()
    client = mqtt_client.Client(
        client_id=CLIENT_ID, 
        protocol=mqtt_client.MQTTv311, 
        userdata={'visualizer': visualizer,},
    )
    client.username_pw_set(USERNAME, PASSWORD)
    client.tls_set(cert_reqs=ssl.CERT_NONE)
    client.tls_insecure_set(True)
    client.on_connect = on_connect
    client.on_message = on_message

    print("Connecting to MQTT broker...")
    client.connect(BROKER, PORT)
    client.loop_start()

    visualizer.run_visualization()
    client.loop_stop()

if __name__ == "__main__":
    main()