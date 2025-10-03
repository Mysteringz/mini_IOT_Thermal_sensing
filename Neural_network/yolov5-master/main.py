import os
import torch 
import train
import export

device = torch.device("cuda")

# Specify your model and dataset yaml path here
model_path = f"./models/yolov5_modified.yaml"
dataset_yaml = os.path.join(os.path.dirname(__file__), 'datasets', 'coco.yaml')

# Automatically get the latest training run ID
id = 0
PATH = os.getcwd()
for root, dirnames, filenames in os.walk(os.path.join(PATH, 'runs', 'train')):
    if len(dirnames) > 0:
        id += 1

weights_path = f"./runs/train/exp{id}/weights/best.pt"
export_model = ("onnx")

# Specify training parameters here
train_params = {
    "cfg": model_path,
    "data": dataset_yaml,
    "epochs": 500,
    "batch_size": 512,
    "imgsz": 32,
    "weights": ''
}

# Specify export parameters here
export_params = {
    "data": dataset_yaml,
    "imgsz": (32, 32),
    "weights": weights_path,
    # "device": device,
    # "include": export_model
}

# Train and export the model
train.run(**train_params)
export.run(**export_params)
