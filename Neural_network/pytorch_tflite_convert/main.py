import subprocess

# onnx2tf -i ./model/yolov5.onnx -o ./model/output -oiqt
model_path = "./model/yolov5.onnx"
output_path = "./model/output"

subprocess.run([
    "onnx2tf", 
    "-i", model_path, 
    "-o", output_path, 
    "-oiqt"
])