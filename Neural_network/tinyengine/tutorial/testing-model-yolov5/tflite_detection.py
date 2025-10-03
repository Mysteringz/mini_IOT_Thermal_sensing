"""
This script is a playground for debugging TensorFlow Lite models.
"""
import sys
import os
import numpy as np
from PIL import Image 
import tensorflow as tf

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "../..")))
from code_generator.TfliteConvertor import TfliteConvertor

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "../..")))
from mcunet.mcunet.model_zoo import download_tflite

w = 32
h = 32

def convert_img_to_list(file_name):
    # Converting image file to int8 array
    img = Image.open(f'img_testing_gen/imgs/{file_name}').convert("RGB").resize((32, 32))
    img_arr = np.array(img).astype(np.int16) - 128
    print("Image shape:", img_arr.shape)
    return img_arr.flatten().tolist()
 
class TFModel:
    # This class is to test TFLite Model Inference
    def __init__(self, model_path, layers):
        self.interpreter = tf.lite.Interpreter(model_path=model_path, experimental_preserve_all_tensors=True)
        self.interpreter.allocate_tensors() # pre-plan tensor allocation to optimize inference
        self.layers = layers
        self.tensor_details = self.interpreter.get_tensor_details()
        self.tensor_map = {t['index']: t for t in self.tensor_details}
        
    def print_all_layers(self):
        # Print all tensors information (tensor of input and output of each layer)
        for tensor in self.tensor_details:
            print(f"Tensor Name: {tensor['name']}, Index: {tensor['index']}, Shape: {tensor['shape']}, Type: {tensor['dtype']}")
            
        # Print all layers information
        for i, layer in enumerate(self.layers):
            # print(f"[Layer {i}] Op={type(layer).__name__} {layer.params}")  # Prints the class name (e.g., Conv2d, AvgPool2d)
            param = layer.params if hasattr(layer, 'params') else {}
            out_idx = param.get("output_idx")
            if out_idx is None:
                continue
            
            print(f"[Layer {i}] Op={param.get('op', 'Unknown')}, Output Tensor Index={out_idx}, Params={param}")
        
    def print_layer_info(self, layer_index):
        # Print layer information for a specific layer
        for i, layer in enumerate(self.layers):
            # print(f"[Layer {i}] Op={type(layer).__name__} {layer.params}")  # Prints the class name (e.g., Conv2d, AvgPool2d)
            param = layer.params if hasattr(layer, 'params') else {}
            out_idx = param.get("output_idx")
            if i != layer_index:
                continue
            if out_idx is None:
                continue
            
            return param

    def debug(self, input_data, layer_index):
        # For debugging inference
        
        # retrieve metadata about tensor input details
        input_details = self.interpreter.get_input_details()
        
        # set value of the input tensors
        for data, detail in zip(input_data, input_details):
            self.interpreter.set_tensor(detail["index"], data)

        # execute the inference
        self.interpreter.invoke()
        
        # print details of final output tensors
        output_idx = []
        for tensor in self.tensor_details:
            if tensor['name'] in ["PartitionedCall:0", "PartitionedCall:1", "PartitionedCall:2"]:
                output_idx.append(tensor['index'])
                print(f"{tensor['name']} ({tensor['index']}): {tensor['quantization']}")

        # Write value of specified output layer
        print("==== Layer-by-Layer Output ====")
        with open("layer_output.txt", "w") as f:
            np.set_printoptions(threshold=np.inf)
            for i, layer in enumerate(self.layers):
                param = layer.params if hasattr(layer, 'params') else {}
                out_idx = param.get("output_idx")
                if out_idx is None:
                    continue
                if out_idx not in output_idx:
                    continue
                
                print(f"==== Layer {i} Output ====")
                for key, value in param.items():
                    print(f"{key}: {value}")
                
                try:
                    out_tensor = self.interpreter.get_tensor(out_idx)
                    # print(f"[Layer {i}] Op={param['op']}, Output Tensor Index={out_idx}, Shape={out_tensor.shape}")
                    tensor_str = np.array2string(out_tensor, threshold=np.inf, separator=', ', max_line_width=1000)
                    f.write(f"[Layer {i}] Op={param['op']}, Shape={out_tensor.shape}\n")
                    f.write(tensor_str + "\n\n")

                except Exception as e:
                    print(f"[Layer {i}] Failed to read tensor {out_idx}: {e}")
                    continue

    
def init_debugger():
    # Create Instance of TFLite class
    tflite_path = ('../../assets/yolov5_full_integer_quant.tflite')

    parser = TfliteConvertor(tflite_path)
    parser.parseOperatorInfo()

    debugger = TFModel(model_path=tflite_path, layers=parser.layer)
    
    return debugger
    
if __name__ == "__main__":
    layer_index = 0
    file_name = 'blob4.png' # Specify the image file name here
    img_arr_1D = np.array(convert_img_to_list(file_name), dtype=np.int8)
    input_data = img_arr_1D.reshape((1, h, w, 3))
    
    debugger = init_debugger()
    debugger.debug([input_data], layer_index)