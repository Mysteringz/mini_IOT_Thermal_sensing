"""
This file is for generating TinyEngine code for running object detection model
"""
import sys
import os

import tensorflow as tf
from types import SimpleNamespace

sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from code_generator.CodeGenerator import CodeGenerator
from code_generator.detection_utils import detectionUtils
from code_generator.GeneralMemoryScheduler import GeneralMemoryScheduler
from code_generator.InputResizer import InputResizer
from code_generator.TfliteConvertor import TfliteConvertor

# Specify your tflite model and config path here
tflite_path = "../assets/tflite/yolov5_full_integer_quant.tflite"
detection_conf = "../assets/config/yolov5_detection_config.json"

life_cycle_path = "./lifecycle.png"

# Specify your input size here
h = 32
w = 32

# Parse the TFLite model
tf_convertor = TfliteConvertor(tflite_path)
tf_convertor.parseOperatorInfo()
layer = tf_convertor.layer

# Resize input dimensions
resizer = InputResizer(layer)
resizer.inputResize(int(h), int(w))

# Specify code generation options here
outTable = []
use_inplace = True

# Memory scheduling   
memory_scheduler = GeneralMemoryScheduler(
    layer,
    False,
    False,
    outputTables=outTable,
    inplace=use_inplace,
    mem_visual_path=life_cycle_path,
    VisaulizeTrainable=False, # disable for code gen
)
memory_scheduler.USE_INPLACE = use_inplace
memory_scheduler.allocateMemory()
memory_scheduler.dumpLayerIndex()

# Initialize instance for generating post processing code
detection = detectionUtils(layer, detection_conf)

# Generate TinyEngine code
code_generator = CodeGenerator(
    memsche=memory_scheduler,
    inplace=memory_scheduler.USE_INPLACE,
    unsigned_input=False,
    patch_params=None,
    FP_output=False,
    profile_mode=False,
    fp_requantize=True,
    tflite_op=False,
    dummy_address=False,
    outputTables=outTable,
    detectionUtils=detection,
)

code_generator.codeGeneration()

print(memory_scheduler.buffers["input_output"])
