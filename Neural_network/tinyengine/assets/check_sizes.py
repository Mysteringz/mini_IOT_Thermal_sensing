import os
import argparse

# To run, use the command: python check_sizes.py --model <file_path>

parser = argparse.ArgumentParser(description="Check the size of TFLite models.")
parser.add_argument("--model", type=str, help="Path to the TFLite model file.")
args = parser.parse_args()

print(os.path.getsize(args.model) / 1024 / 1024, "MB")