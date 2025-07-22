# Converts from pytorch model to a clean onnx model
# Problem with the pytorch exporter is that the output onnx model is very messy,
# This a standalone converter fixes that issue

import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.onnx

# Copy over your PyTorch model class here
class SimpleCNN(nn.Module):
    def __init__(self):
        super(SimpleCNN, self).__init__()
        # Input: 40x40x3
        self.conv1 = nn.Conv2d(3, 16, kernel_size=3, padding=1)  # 40x40x16
        self.pool1 = nn.AvgPool2d(2, 2)                           # 20x20x16
        
        # Replace 20x20 kernel with this sequence:
        self.conv2 = nn.Conv2d(16, 32, kernel_size=3, padding=1)  # 20x20x32
        self.pool2 = nn.AvgPool2d(2, 2)                           # 10x10x32
        self.conv3 = nn.Conv2d(32, 64, kernel_size=3, padding=1)  # 10x10x64
        self.pool3 = nn.AvgPool2d(2, 2)                           # 5x5x64
        self.conv4 = nn.Conv2d(64, 128, kernel_size=3, padding=1) # 5x5x128
        self.pool4 = nn.AvgPool2d(5, 5)                           # 1x1x128 # Pooling to reduce spatial dim length
        
        # Final projection to 2 channels
        self.final_conv = nn.Conv2d(128, 2, kernel_size=1)     # 1x1x2 # Acts like an FC layer through Point-wise convolution

    def forward(self, x):
        x = F.relu(self.conv1(x))
        x = self.pool1(x)
        
        x = F.relu(self.conv2(x))
        x = self.pool2(x)
        x = F.relu(self.conv3(x))
        x = self.pool3(x)
        x = F.relu(self.conv4(x))
        x = self.pool4(x)
        
        x = self.final_conv(x)  # [batch, 6400, 1, 1]
        x = x.squeeze(-1).squeeze(-1)  # [batch, 6400] # Removes spatial dimensions entirely
        return x

def convert_to_onnx(pytorch_model, input_shape, onnx_filename, weights_path=None):
    """
    Convert a PyTorch model to ONNX format
    
    Args:
        pytorch_model: The PyTorch model to convert
        input_shape: Tuple describing the input shape (e.g., (1, 3, 40, 40))
        onnx_filename: Output filename for the ONNX model
    """
    # Create a dummy input with the specified shape
    sample_input = (torch.randn(*input_shape),)

    # Load weights
    if weights_path is not None:
        pytorch_model.load_state_dict(torch.load(weights_path, weights_only=True))
        print(f"Loaded weights from {weights_path}")

    pytorch_model.eval()
    
    # Export model with metadata
    torch.onnx.export(
        pytorch_model,               # model being run
        sample_input,                # model input
        onnx_filename,               # output file name
        export_params=True,          # store the trained parameter weights
        opset_version=11,            # ONNX opset version to use
        do_constant_folding=True,    # whether to execute constant folding
        input_names=['input'],       # model input names
        output_names=['output'],     # model output names
        dynamic_axes={
            'input': {0: 'batch_size'},  # variable length axes
            'output': {0: 'batch_size'}
        }
    )
    
    print(f"Model successfully converted to ONNX and saved as {onnx_filename}")

if __name__ == "__main__":
    # Create a simple model
    model = SimpleCNN()
    
    # Set the model to evaluation mode (important for models with dropout/batchnorm)
    model.eval()
    
    # Define the input shape (batch_size, channels, height, width)
    input_shape = (1, 3, 40, 40)
    onnx_filename = "/Users/lzf/InnowingSRA/pytorch_simple/simple_model/model99.onnx" # This is the output dir and filename
    weight_path = '/Users/lzf/InnowingSRA/pytorch_simple/simple_model/model_weights.pth' # Where the pytorch weights are stored
    
    # Convert the model to ONNX
    convert_to_onnx(model, input_shape, onnx_filename, weight_path)
