import numpy as np
from PIL import Image
from io import BytesIO
import display_api
import os.path as path
import time

# Open the file in binary mode
with open(path.normpath("source/examples/ex-image.jpeg"), "rb") as f:
    data = f.read()          # read all bytes

# Load image into PIL
image = Image.open(BytesIO(data))

new_size = (1920, 1080)
image = image.resize(new_size, Image.ANTIALIAS)

# Convert to NumPy array
image_array = np.array(image, dtype=np.uint32)

print("Image mode:", image.mode)
print("Array shape:", image_array.shape)
print("Data type:", image_array.dtype)

cmd = display_api.cmd_clear_screen_t()
cmd.on = display_api.BLACK

display_api.display_clear_screen(
    cmd, 
    display_api.DISPLAY_WNO, 
    display_api.display_cb_verbose,
    "[PYTHON.CLEAR] : custom message\n"   
)

time.sleep(3) 

offset = (0, 0)

h = display_api.display_query_height()
w = display_api.display_query_width()

if (h | w) < 0:
    print("Ups something went wrong ...")
    exit(1)

print(f"Resolution : {w}x{h}")

cmd = display_api.cmd_draw_image_t()
cmd.x = offset[0]
cmd.y = offset[1]
cmd.width = new_size[1]
cmd.height = new_size[0]

cmd.set_buffer(image_array)

display_api.display_draw_image(
    cmd, 
    display_api.DISPLAY_WNO,
    None,
    "[PYTHON.DRAW] : custom message\n")