import numpy as np
from PIL import Image
from io import BytesIO
import display_api
import os.path as path
import ctypes

# Open the file in binary mode
with open(path.normpath("source/examples/ex-image.jpeg"), "rb") as f:
    data = f.read()          # read all bytes

# Load image into PIL
image = Image.open(BytesIO(data))

new_size = (256, 256)
image = image.resize(new_size, Image.ANTIALIAS)

# Convert to NumPy array
image_array = np.array(image)

print("Image mode:", image.mode)
print("Array shape:", image_array.shape)
print("Data type:", image_array.dtype)

offset = (10, 10)

cmd = display_api.cmd_draw_image_t()
cmd.x = offset[0]
cmd.y = offset[1]
cmd.width = new_size[1]
cmd.height = new_size[0]
cmd.buffer = image_array

h = display_api.display_query_height()
w = display_api.display_query_width()

print(f"Resolution : {w}x{h}")

display_api.display_draw_image(
    cmd, 
    display_api.DISPLAY_WNO,
    display_api.display_cb_verbose,
    "[PYTHON.DRAW] : custom message\n")