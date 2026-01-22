import numpy as np
from PIL import Image
from io import BytesIO
import display_api
import os.path as path
import time

# Open the file in binary mode
with open(path.normpath("source/examples/ex-image.jpeg"), "rb") as f:
    data = f.read()          # read all bytes

pix_format = "RGBA"

# Load image into PIL
image = Image.open(BytesIO(data))
image = image.convert(pix_format)

new_size = (1080, 1080)
image = image.resize(new_size)

image_array = np.array(image, dtype=np.uint8)

# order rgba
image_array = (
    (image_array[..., 3].astype(np.uint32) << 24) |
    (image_array[..., 2].astype(np.uint32) << 16) |
    (image_array[..., 1].astype(np.uint32) << 8) |
    image_array[..., 0].astype(np.uint32)
)

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

time.sleep(1) 

offset = (300, 0)

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
cmd.order = display_api.ORDER_BGRA

cmd.set_buffer_u32(image_array, display_api.ENDIAN_LITTLE)

display_api.display_draw_image(
    cmd, 
    display_api.DISPLAY_WNO,
    None,
    "[PYTHON.DRAW] : custom message\n")