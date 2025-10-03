"""
This file is for calculating bounding box coordinates from YOLOv5 format
"""
import cv2

# Specify bounding box result from yolov5 inference
bbox = [
"0 0.875 0.421875 0.1875 0.34375",
"0 0.921875 0.328125 0.15625 0.15625",
"0 0.15625 0.109375 0.1875 0.15625",
"0 0.78125 0.5625 0.1875 0.1875"
]

bbox_list = [[float(b) for b in bb.split()] for bb in bbox]
img_path = "img_testing_gen/imgs/blob3.png" # Specify the image path here
imgsz = 32 # Specify the image size here (same as model input size)

def calc_bbox(bbox_list, imgsz):
    # Decode YOLOv5 format to bounding box coordinates (x1, x2, y1, y2)
    bbox_coord = []
    for bbox in bbox_list:
        cls_score, cx, cy, w, h = bbox
        if cls_score < 0:
            continue
        cx *= imgsz
        cy *= imgsz
        w *= imgsz
        h *= imgsz
        
        x1 = cx - w / 2
        y1 = cy - h / 2
        x2 = cx + w / 2
        y2 = cy + h / 2
        bbox_coord.append([x1, y1, x2, y2, cls_score])
        print(f"Class: {cls_score}, BBox: ({x1:.2f}, {y1:.2f}, {x2:.2f}, {y2:.2f})")
        
    return bbox_coord
        
def plot_bbox(bbox_coord, img_path):
    # Draw bounding box on image
    img = cv2.imread(img_path)
    for bbox in bbox_coord:
        x1, y1, x2, y2, cls_score = bbox
        cv2.rectangle(img, (int(x1), int(y1)), (int(x2), int(y2)), (0, 255, 0), 1)
        # cv2.putText(img, f"{cls_score:.2f}", (int(x1), int(y1) - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
        
    # resize image
    img = cv2.resize(img, (imgsz*5, imgsz*5))
        
    # save file
    cv2.imwrite("output_image.png", img)

if __name__ == "__main__":
    bbox_coord = calc_bbox(bbox_list, imgsz)
    plot_bbox(bbox_coord, img_path)