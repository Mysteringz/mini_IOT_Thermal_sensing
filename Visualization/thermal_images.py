
# w_scaled = int((x_coord+4)*scale_x)
# z_scaled = int((y_coord+4)*scale_y)

import serial
import cv2
import numpy as np
import time
from collections import deque

# TRACKING AND VISUALIZATION CONFIG
FRAME_SIZE = (1280, 1440)
GRID_SIZE = (32, 24)

class Visualizer:
    def __init__(self,frame_size=FRAME_SIZE, grid_size=GRID_SIZE, max_history=6):
        self.median_frame = None
        self.frame_size = frame_size
        self.grid_size = grid_size
        self.history = deque(maxlen=max_history)
        #self.cap = cv2.VideoCapture(0)

    def convert_temperatures_to_image(self, temperature_array):
        min_temp = np.min(temperature_array)
        max_temp = np.max(temperature_array)
        if max_temp - min_temp == 0:
            normalized_array = np.zeros_like(temperature_array)
        else:
            normalized_array = (temperature_array - min_temp) / (max_temp - min_temp) * 255
        image = normalized_array.astype(np.uint8)
        return image

    def process_median_frame(self, line):
        # Remove the ';;;;' prefix and split the data
        median_data = line[4:].split(',')
        median_data = [element.strip() for element in median_data if element.strip()]
        self.median_frame = []
        for element in median_data:
            try:
                value = float(element)
            except ValueError:
                value = 0.0
            self.median_frame.append(value)

        # Ensure median_frame has 768 elements
        if len(self.median_frame) != 768:
            print(f"Warning: Median frame length is {len(self.median_frame)} instead of 768.")
            self.median_frame = np.array(self.median_frame)
            self.median_frame = np.pad(self.median_frame, (0, 768 - len(self.median_frame)), 'constant', constant_values=0)
        else:
            self.median_frame = np.array(self.median_frame)

    def process_data_frame(self, line):
        if self.median_frame is None:
            print("Median frame not initialized!")
            return

        # Split the line by ';'
        split_string = line.strip().split(';')
        if len(split_string) < 1:
            print("Invalid data frame!")
            return

        frame_data_str = split_string[0]
        coordinates = [tuple(map(int, s.split(','))) for s in split_string[1:] if s != '']

        # Process raw_frame
        raw_frame_data = frame_data_str.split(',')
        raw_frame_data = [element.strip() for element in raw_frame_data if element.strip()]
        raw_frame = []
        for element in raw_frame_data:
            try:
                value = float(element)
            except ValueError:
                value = 0.0
            raw_frame.append(value)

        # Ensure raw_frame has 768 elements
        if len(raw_frame) != 768:
            print(f"Warning: Raw frame length is {len(raw_frame)} instead of 768.")
            raw_frame = np.array(raw_frame)
            raw_frame = np.pad(raw_frame, (0, 768 - len(raw_frame)), 'constant', constant_values=0)
        else:
            raw_frame = np.array(raw_frame)

        # Compute subtracted_frame
        subtracted_frame = raw_frame - self.median_frame
        subtracted_frame[subtracted_frame < 0] = 0

        # Thresholding
        threshold_diff_value = 1.3
        threshold_frame = np.where(subtracted_frame > threshold_diff_value, 1, 0)
        threshold_frame = threshold_frame.astype(np.uint8)

        # Reshape frames to (24,32)
        raw_frame = raw_frame.reshape((24, 32))
        subtracted_frame_img = subtracted_frame.reshape((24, 32))
        threshold_frame_img = threshold_frame.reshape((24, 32))
        median_frame_img = self.median_frame.reshape((24, 32))

        # Convert to images
        median_frame_img_disp = self.convert_temperatures_to_image(median_frame_img)
        raw_frame_img_disp = self.convert_temperatures_to_image(raw_frame)
        subtracted_frame_img_disp = self.convert_temperatures_to_image(subtracted_frame_img)
        # For threshold_frame, since it's binary, multiply by 255 to convert to 0 or 255
        threshold_frame_disp = threshold_frame_img * 255
        threshold_frame_disp = threshold_frame_disp.astype(np.uint8)

        # Apply colormap
        median_frame_img_colored = cv2.applyColorMap(median_frame_img_disp, cv2.COLORMAP_PLASMA)
        raw_frame_img_colored = cv2.applyColorMap(raw_frame_img_disp, cv2.COLORMAP_PLASMA)
        subtracted_frame_img_colored = cv2.applyColorMap(subtracted_frame_img_disp, cv2.COLORMAP_PLASMA)
        threshold_frame_img_colored = cv2.applyColorMap(threshold_frame_disp, cv2.COLORMAP_PLASMA)

        # Resize images for display
        size = FRAME_SIZE
        median_frame_img_colored = cv2.resize(median_frame_img_colored, size, interpolation=cv2.INTER_NEAREST)
        raw_frame_img_colored = cv2.resize(raw_frame_img_colored, size, interpolation=cv2.INTER_NEAREST)
        subtracted_frame_img_colored = cv2.resize(subtracted_frame_img_colored, size, interpolation=cv2.INTER_NEAREST)
        threshold_frame_img_colored = cv2.resize(threshold_frame_img_colored, size, interpolation=cv2.INTER_NEAREST)
        threshold_frame_resized = cv2.resize(threshold_frame_img, size, interpolation=cv2.INTER_NEAREST)
        # For circles, create a blank image (black background)
        circle_frame_img = np.zeros((24, 32), dtype=np.uint8)
        circle_frame_img_resized = cv2.resize(circle_frame_img, size, interpolation=cv2.INTER_NEAREST)
        circle_frame_img_colored = cv2.cvtColor(circle_frame_img_resized, cv2.COLOR_GRAY2BGR)

        # Process threshold_frame to find bounding boxes
        # Since threshold_frame is a binary image, cv2.findContours can be used
        contours, _ = cv2.findContours(threshold_frame_img, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        # For drawing, we need scaling factors because images were resized
        scale_x = size[0] / 32
        scale_y = size[1] / 24

        # Draw bounding boxes on the raw frame image
        for cnt in contours:
            # Calculate area to filter small noises, optional
            area = cv2.contourArea(cnt)
            if area > 2:  # Adjust as needed based on your data
                x, y, w, h = cv2.boundingRect(cnt)
                # Since images were resized, scale the coordinates
                x_scaled = int(x * scale_x)
                y_scaled = int(y * scale_y)
                w_scaled = int(w * scale_x)
                h_scaled = int(h * scale_y)
                cv2.rectangle(raw_frame_img_colored, (x_scaled, y_scaled),
                              (x_scaled + w_scaled, y_scaled + h_scaled), (0, 255, 0), 2)
                
        # # Draw circles on the circle image with higher resolution
        # for coord in coordinates:
        #     # Parse the coordinate string into integers
        #     try:
        #         x_coord, y_coord = map(int, coord.strip().split(','))
        #     except ValueError:
        #         continue  # Skip invalid coordinates

        #     # Scale the coordinates
        #     x_scaled = int(x_coord * scale_x)
        #     y_scaled = int(y_coord * scale_y)
        #     radius_scaled = int(3 * ((scale_x + scale_y)/2))  # Scale radius accordingly

        #     # Draw a filled circle on the circle image
        #     cv2.circle(circle_frame_img_colored, (x_scaled, y_scaled), radius_scaled, (255, 255, 255), thickness=-1)  # White color

        # Display images
        self.history.append(coordinates)
        print(coordinates)
        
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
            radius = max(30, (20 - idx_from_newest * 2)*3)
            
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
        
        # # Resize camera_frame to match the width of heatmap
        # # so we can stack them vertically
        # cam_h, cam_w, _ = camera_frame.shape
        # if cam_w != w:
        #     # Scale camera_frame to width = w
        #     ratio = w / float(cam_w)
        #     new_height = int(cam_h * ratio)
        #     camera_frame = cv2.resize(camera_frame, (w, new_height), interpolation=cv2.INTER_AREA)
        
        # # If the camera frame height is different from heatmap’s height,
        # # we will either pad or resize the heatmap to match; here we keep 
        # # the heatmap fixed and pad the camera if needed for neat stacking.
        # cam_h, cam_w, _ = camera_frame.shape
        # if cam_h != h:
        #     # If camera frame is smaller or bigger, let's do a quick resize
        #     # to match the heatmap's height for a simpler vertical stack.
        #     camera_frame = cv2.resize(camera_frame, (w, h), interpolation=cv2.INTER_AREA)
        
        # # Now both camera_frame and heatmap should be (h, w, 3).
        # # Stack them vertically: camera on top, heatmap on bottom.
        # combined = cv2.vconcat([camera_frame, heatmap])
        
        cv2.imshow("Iot Dots", heatmap)
        
        cv2.imshow("Raw Frame with Bounding Boxes", raw_frame_img_colored)
        # cv2.imshow("Median Frame", median_frame_img_colored)
        # cv2.imshow("Subtracted Frame", subtracted_frame_img_colored)
        # cv2.imshow("Threshold", threshold_frame_img_colored)
        # cv2.imshow("Circles", circle_frame_img_colored)

        cv2.waitKey(1)

    def visualize(self, line):
        if line.startswith(';;;;'):
            self.process_median_frame(line)
        else:
            self.process_data_frame(line)


def main():
    baud_rate = 250000
    serial_port = 'COM6'

    # Initialize Visualizer
    visualizer = Visualizer()

    # Set up serial connection
    ser = serial.Serial(serial_port, baud_rate, timeout=1)
    ser.flush()
    ser.readline()
    data_buffer = ""

    while True:
        if ser.in_waiting > 0:
            data = ser.read_until(b'\r').decode()
            print(data)
            data_buffer += data

            # Check if we have a complete message (ends with '\r')
            if '\r' in data_buffer:
                lines = data_buffer.strip().split('\r')
                for line in lines:
                    if line:
                        visualizer.visualize(line)
                data_buffer = ""

        time.sleep(0.01)  # Slight delay to prevent high CPU usage


if __name__ == "__main__":
    main()
