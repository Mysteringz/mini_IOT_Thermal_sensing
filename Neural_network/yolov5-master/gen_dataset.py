import os
import numpy as np
import cv2
import json
    
# Generate images for training
def generate_images(data_size, folder_path, data_type, max_blobs=10, img_size=32):
    # Create COCO format dictionary
    if not os.path.exists(f"{folder_path}/images/{data_type}"):
        os.makedirs(f"{folder_path}/images/{data_type}")
    if not os.path.exists(f"{folder_path}/labels/{data_type}"):
        os.makedirs(f"{folder_path}/labels/{data_type}")
        
    for i in range(data_size):
        
        with open(f"{folder_path}/labels/{data_type}/img{i}.txt", 'w') as f:
            
            # img size 32x32, with 24x32 for the actual image
            img = np.zeros((img_size, img_size), dtype=np.int8)
            num_of_human = np.random.randint(0, max_blobs+1)
            
            for _ in range(num_of_human):

                # Randomly generate a blob
                blob_size = np.random.randint(4, 7)
                # Use actual image size here
                x1, y1 = np.random.randint(0, 32-blob_size), np.random.randint(0, 24-blob_size)
                x2, y2 = x1+blob_size, y1+blob_size
                img[y1:y2, x1:x2] = np.random.uniform(28, 36, (blob_size, blob_size))
                
                f.write(f"{0} {(x1 + blob_size / 2) / img_size } {(y1 + blob_size / 2) / img_size} {blob_size / img_size} {blob_size / img_size}\n")  # category_id, x, y, w, h

            # Add background temperature
            background_temp = np.random.uniform(23, 28, (img_size, img_size))
            img[img == 0] = background_temp[img == 0]

            # Add noise
            # img += np.random.normal(0, 0.4, (24, 32))

            # Normalize image
            img = (img - 23) / (35 - 23)
            img_8int = (img * 255).astype(np.uint8)
            img_rgb = np.stack([img_8int]*3, axis=-1)
            
            if i == 0:
                print(img_rgb.shape)

            # Save image (scaled to 255)
            cv2.imwrite(f'{folder_path}/images/{data_type}/img{i}.png'.format(i), img_8int)

if __name__ == "__main__":
    
    # Create folder to save all data
    dataset_path = os.path.join(os.path.dirname(__file__), 'datasets')

    # Maximum number of blobs in one image
    max_blobs = 5
    
    if not os.path.exists(f"{dataset_path}/images"):
        os.makedirs(f"{dataset_path}/images")
    if not os.path.exists(f"{dataset_path}/labels"):
        os.makedirs(f"{dataset_path}/labels")
    
    # Training Dataset
    generate_images(1400, dataset_path, 'train', max_blobs)
    # Validation Dataset
    generate_images(300, dataset_path, 'val', max_blobs)
    # Testing Dataset
    generate_images(300, dataset_path, 'test', max_blobs)