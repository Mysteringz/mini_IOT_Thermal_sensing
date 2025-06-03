import cv2
import numpy as np
import time
import random
import json
import ssl
from collections import deque
from paho.mqtt import client as mqtt_client
import serial

# MQTT CONFIGURATION -- FILL THESE WITH OUR VALUES
BROKER = 'as1.cloud.thethings.industries'
NAME = 'iot-lite-1'
PORT = 8883
USERNAME = 'thermal-detection-lite@innowingiot' #iot
PASSWORD = 'NNSXS.OUN72UG5JKTYAR5PNQIS5SCUAPRAH2OHVSRENRI.XA35FFYUPVBBLFU3HDRW5EZJBF4BMBUK5K637TQ6T7Q3R6M2HWYQ' #API / PASSWORD
TOPIC = f'v3/{USERNAME}/devices/{NAME}/up'
CLIENT_ID = f"mqtt_viz_{random.randint(0, 1e4)}"

# TRACKING AND VISUALIZATION CONFIG
FRAME_SIZE = (960, 540)
GRID_SIZE = (32, 24)

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

class TrackedNode:
    def __init__(self, node_id, x, y, color):
        self.id = node_id
        self.x = x
        self.y = y
        self.color = color
        self.last_seen = 0
        self.position_history = deque(maxlen=6)  # Store position history for trails

class YourTrackingClass:
    def __init__(self, frame_size=(960, 540), grid_size=(32, 24)):
        # Existing tracking setup
        self.tracked_nodes = []
        self.next_node_id = 0
        self.max_inactive_frames = 5
        self.distance_threshold = 50
        
        # Visualization parameters
        self.frame_size = frame_size
        self.grid_size = grid_size
        self.scale_x = frame_size[0] / grid_size[0]
        self.scale_y = frame_size[1] / grid_size[1]

    def _calculate_distances(self, points1, points2):
        """Pure NumPy implementation of pairwise distances"""
        pts1 = np.array(points1).reshape(-1, 1, 2)
        pts2 = np.array(points2).reshape(1, -1, 2)
        return np.sqrt(np.sum((pts1 - pts2)**2, axis=2))

    def process_data_frame(self, line):
        # Convert input line to pixel coordinates
        split_string = line.strip().split(';')
        current_grid_pts = [tuple(map(int, s.split(','))) for s in split_string[1:] if s]
        current_pixel_pts = [
            (int(x * self.scale_x), int(y * self.scale_y))
            for (x, y) in current_grid_pts
        ]
        matches = []
        # Update existing nodes -------------------------------------------------
        if self.tracked_nodes:
            # Get positions of tracked nodes
            tracked_positions = [(node.x, node.y) for node in self.tracked_nodes]
            
            # Calculate distance matrix (tracked nodes vs current points)
            dist_matrix = self._calculate_distances(
                tracked_positions, current_pixel_pts
            )
            
            # Find best matches using Hungarian algorithm (greedy version)
            matches = []
            available_current = set(range(len(current_pixel_pts)))
            
            # Sort tracked nodes by last_seen (prioritize recently seen nodes)
            sorted_nodes = sorted(
                enumerate(self.tracked_nodes),
                key=lambda x: x[1].last_seen
            )
            
            for tracked_idx, node in sorted_nodes:
                if not available_current:
                    break
                
                # Find closest current point that's within threshold
                current_idx = np.argmin(dist_matrix[tracked_idx, list(available_current)])
                current_global_idx = list(available_current)[current_idx]
                distance = dist_matrix[tracked_idx, current_global_idx]
                
                if distance < self.distance_threshold:
                    matches.append((tracked_idx, current_global_idx))
                    available_current.remove(current_global_idx)

            # Update matched nodes
            for tracked_idx, current_idx in matches:
                x, y = current_pixel_pts[current_idx]
                node = self.tracked_nodes[tracked_idx]
                node.x = x
                node.y = y
                node.last_seen = 0

        # Create new nodes for unmatched points ----------------------------------
        unmatched_current = set(range(len(current_pixel_pts))) - {idx for _, idx in matches}
        for idx in unmatched_current:
            x, y = current_pixel_pts[idx]
            # Generate unique color based on node ID
            hue = (self.next_node_id * 30) % 180  # 30° steps for distinct colors
            color = cv2.cvtColor(np.uint8([[[hue, 255, 255]]]), cv2.COLOR_HSV2BGR)[0][0].tolist()
            self.tracked_nodes.append(
                TrackedNode(self.next_node_id, x, y, color)
            )
            self.next_node_id += 1

        # Remove stale nodes ----------------------------------------------------
        self.tracked_nodes = [
            node for node in self.tracked_nodes
            if node.last_seen < self.max_inactive_frames
        ]

        # Increment last_seen counters ------------------------------------------
        for node in self.tracked_nodes:
            node.last_seen += 1

        # Draw visualization ----------------------------------------------------
        img = np.ones((self.frame_size[1], self.frame_size[0], 3), dtype=np.uint8) * 255
        
        # Draw all tracked nodes with trails
        for node in self.tracked_nodes:
            # Update position history
            node.position_history.append((node.x, node.y))
            
            # Draw historical positions (oldest first)
            history = list(node.position_history)
            for i in range(len(history)):
                idx_from_newest = (len(history) - 1) - i
                intensity = 50 * idx_from_newest
                radius = max(5, 20 - idx_from_newest * 2)
                x, y = history[i]
                
                # Draw interpolated points between frames
                if i > 0:
                    prev_x, prev_y = history[i-1]
                    for t in np.linspace(0, 1, 3):
                        interp_x = int(prev_x + t * (x - prev_x))
                        interp_y = int(prev_y + t * (y - prev_y))
                        cv2.circle(img, (interp_x, interp_y), radius,
                                   (intensity, intensity, intensity), -1, cv2.LINE_AA)
                
                # Draw actual historical point
                cv2.circle(img, (x, y), radius,
                           (intensity, intensity, intensity), -1, cv2.LINE_AA)
            
            # Draw current position with ID (on top)
            cv2.circle(img, (node.x, node.y), 10, node.color, -1)
            cv2.putText(
                img, str(node.id), (node.x + 15, node.y),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, node.color, 2
            )
        
        cv2.imshow("IoT Tracking", img)
        cv2.waitKey(1)

class Visualizer:
    """
    A simpler visualizer that opens your Mac’s built-in camera,
    shows the camera feed on TOP, and the heatmap/dots tracking on BOTTOM.
    """
    def __init__(self, frame_size=FRAME_SIZE, grid_size=GRID_SIZE, max_history=6):
        self.frame_number = 0
        self.frame_size = frame_size
        self.grid_size = grid_size
        # We'll store up to 6 frames of points (the current one + 5 older)
        self.history = deque(maxlen=max_history)

        # Open camera; adjust index if 0 doesn't work on your system
        self.cap = cv2.VideoCapture(0)
        # If desired, you can set a resolution here:
        # self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        # self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    def process_data_frame(self, line):
        # Read a frame from the camera
        ret, camera_frame = self.cap.read()
        if not ret:
            # If no frame is captured, skip
            print("Warning: Could not read from camera.")
            return

        # Add new points to history
        split_string = line.strip().split(';')
        coordinates = [tuple(map(int, s.split(','))) for s in split_string[1:] if s != '']
        self.history.append(coordinates)
        
        # Keep only the last N frames to control trail length
        max_history = 10  # Adjust this for longer/shorter trails
        if len(self.history) > max_history:
            self.history.pop(0)
        
        # Prepare white background for the "heatmap"
        w, h = self.frame_size
        gw, gh = self.grid_size
        scale_x, scale_y = w / gw, h / gh
        heatmap = np.ones((h, w, 3), dtype=np.uint8) * 255  # White background
        
        # Draw from oldest to newest
        for i, pts in enumerate(self.history):
            idx_from_newest = (len(self.history) - 1) - i
            intensity = 50 * idx_from_newest
            radius = max(5, 20 - idx_from_newest * 2)
            
            for (x, y) in pts:
                # Scale from grid coordinates to pixel space
                cx, cy = int(x * scale_x), int(y * scale_y)
                
                # Draw interpolated points between previous and current positions
                if i > 0 and len(self.history[i-1]) == len(pts):
                    # For simplicity, we assume same number of points in each frame
                    prev_x, prev_y = self.history[i-1][pts.index((x, y))]
                    for t in np.linspace(0, 1, 3):
                        interp_x = prev_x + t * (x - prev_x)
                        interp_y = prev_y + t * (y - prev_y)
                        interp_cx = int(interp_x * scale_x)
                        interp_cy = int(interp_y * scale_y)
                        cv2.circle(heatmap, (interp_cx, interp_cy), radius,
                                   (intensity, intensity, intensity), -1, cv2.LINE_AA)
                
                # Draw the actual point
                cv2.circle(heatmap, (cx, cy), radius,
                           (intensity, intensity, intensity), -1, cv2.LINE_AA)
        
        # Resize camera_frame to match the width of heatmap
        # so we can stack them vertically
        cam_h, cam_w, _ = camera_frame.shape
        if cam_w != w:
            # Scale camera_frame to width = w
            ratio = w / float(cam_w)
            new_height = int(cam_h * ratio)
            camera_frame = cv2.resize(camera_frame, (w, new_height), interpolation=cv2.INTER_AREA)
        
        # If the camera frame height is different from heatmap’s height,
        # we will either pad or resize the heatmap to match; here we keep 
        # the heatmap fixed and pad the camera if needed for neat stacking.
        cam_h, cam_w, _ = camera_frame.shape
        if cam_h != h:
            # If camera frame is smaller or bigger, let's do a quick resize
            # to match the heatmap's height for a simpler vertical stack.
            camera_frame = cv2.resize(camera_frame, (w, h), interpolation=cv2.INTER_AREA)
        
        # Now both camera_frame and heatmap should be (h, w, 3).
        # Stack them vertically: camera on top, heatmap on bottom.
        combined = cv2.vconcat([camera_frame, heatmap])
        
        cv2.imshow("Camera (top) + IoT Dots Tracking (bottom)", combined)
        cv2.waitKey(1)


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
    print(payload)
    checkpoint_id, points = parse_mqtt_payload(payload)
    if checkpoint_id is not None and points:
        print(f"Processing {len(points)} points from checkpoint {checkpoint_id}")
        # Call our visualizer
        userdata['visualizer'].process_payload(checkpoint_id, points)
    else:
        print("No valid data in message")

def main():
    # Instantiate the new Visualizer that also shows camera feed on top
    visualizer = Visualizer()
    # If you want to switch to the more sophisticated YourTrackingClass, do so:
    # visualizer = YourTrackingClass()

    baud_rate = 250000
    serial_port = '/dev/tty.usbserial-58930346921'
    ser = serial.Serial(serial_port, baud_rate, timeout=1)
    ser.flush()
    ser.readline()

    data_buffer = ""
    while True:
        if ser.in_waiting > 0:
            data = ser.read_until(b'\r').decode()
            data_buffer += data

            # Check if we have a complete message (ends with '\r')
            if '\r' in data_buffer:
                lines = data_buffer.strip().split('\r')
                for line in lines:
                    if line:
                        visualizer.process_data_frame(line)
                data_buffer = ""

        time.sleep(0.01)  # Slight delay to prevent high CPU usage


if __name__ == "__main__":
    main()
