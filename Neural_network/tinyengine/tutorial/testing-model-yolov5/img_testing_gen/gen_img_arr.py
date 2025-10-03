"""
This file is for generating image array in uint8_t format from image file 
to be imported to Model Inference Testing
"""

from PIL import Image 
import numpy as np
import os

from utils import LoadImages, letterbox

# Create Directory to store images and image arrays
if not os.path.exists('imgs'):
    os.makedirs('imgs')
if not os.path.exists('imgs_arr'):
    os.makedirs('imgs_arr')
    
"""
Image Requirement
- Image size: 32x32 pixels
- Image format: RGB (3 channels)
- Pixel valiues: float_32
"""
object_name = 'blob1'  # Specify the object name here
file_name = f'imgs/{object_name}.png' # Specify the image file name here


def convert_img_to_list(file_name):
    # Convert grayscale img to RGB and resize to 32x32
    img = Image.open(file_name).convert("RGB").resize((32, 32))
    img_arr = np.array(img).astype(np.int16)
    
    return img_arr.flatten().tolist()

def convert_img_to_yolov5_list(source, imgsz, stride, pt, vid_stride):
    # Imitate YOLOv5 image loading and processing by using its LoadImages function and letterbox
    dataset = LoadImages(source, img_size=imgsz, stride=stride, auto=pt, vid_stride=vid_stride)
    path, im, im0s, vid_cap, s = next(iter(dataset))
    # im = torch.from_numpy(im)
    return im
    

def write_img_to_file(file_name, object_name):
    img_arr_1D = convert_img_to_list(file_name)
        
    # Write image array to .c file
    with open(f'imgs_arr/{object_name}.c', 'w') as f:
        f.write(f"const signed char {object_name}[] = {{\n")
        for i, val in enumerate(img_arr_1D):
            f.write(f"{val}")
            if i < len(img_arr_1D) - 1:
                f.write(", ")
            if (i + 1) % 48 == 0:
                f.write("\n")
            
        f.write("};\n")
    
if __name__ == "__main__":
    write_img_to_file(file_name, object_name)