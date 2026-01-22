import numpy as np
from PIL import Image
from io import BytesIO
import display_api
import os.path as path

# Open the file in binary mode
with open(path.normpath("source/examples/ex-image.jpeg"), "rb") as f:
    data = f.read()

pix_format = "RGBA"
image = Image.open(BytesIO(data))
image = image.convert(pix_format)

new_size = (520, 520)
image = image.resize(new_size)

cmd = display_api.cmd_clear_screen_t()
cmd.on = display_api.BLACK

display_api.display_clear_screen(
    cmd, 
    display_api.DISPLAY_WNO, 
    display_api.display_cb_verbose,
    "[PYTHON.CLEAR] : custom message\n"   
)

h = display_api.display_query_height()
w = display_api.display_query_width()

if (h | w) < 0:
    print("Ups something went wrong ...")
    exit(1)

print("resolution %d - %d" % (h, w))

i = 0
inc = 50

while ((i + new_size[0]) < h):
    j = 0
    while ((j + new_size[1]) < w):
        cmd = display_api.cmd_draw_image_t()
        cmd.x = j
        cmd.y = i
        cmd.width = new_size[1]
        cmd.height = new_size[0]
        cmd.order = display_api.ORDER_BGRA

        image_array = np.array(image, dtype=np.uint8)

        # order rgba
        image_array = (
            (image_array[..., 3].astype(np.uint32) << 24) |
            (image_array[..., 2].astype(np.uint32) << 16) |
            (image_array[..., 1].astype(np.uint32) << 8) |
            image_array[..., 0].astype(np.uint32)
        )

        cmd.set_buffer_u32(image_array, display_api.ENDIAN_LITTLE)

        display_api.display_draw_image(
            cmd,
            display_api.DISPLAY_WNO,
            None,
            "[PYTHON.DRAW] : custom message\n")
        
        j += inc

    i += inc
        