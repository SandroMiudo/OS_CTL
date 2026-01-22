# normally you would input the context to the llm, or other ai models,
# but for this example we've prompted open-ai to generate a 128x128 resolution image
# which should include the tools we've provided and output it into the instructions.txt

# here the agent will only act as scanner which scanns for the tools used
# and calls the api

import json
import display_api
import os.path as path
import os
import time

def scan_and_draw(file_path: str):
    cmd = display_api.cmd_clear_screen_t()
    cmd.on = display_api.BLACK

    display_api.display_clear_screen(
        cmd, 
        display_api.DISPLAY_WNO, 
        display_api.display_cb_verbose,
        "[PYTHON.CLEAR] : custom message\n"   
    )

    with open(file_path, "r") as f:
        lines = f.readlines()

    i = 0
    while i < len(lines):
        line = lines[i].strip()

        # Only trigger on the marker
        if line == "#x_display":
            if i + 1 < len(lines):
                json_line = lines[i + 1].strip()

                try:
                    data = json.loads(json_line)

                    cmd = display_api.cmd_set_pixel_t()
                    cmd.x = int(data["offset_x"])
                    cmd.y = int(data["offset_y"])
                    cmd.color = int(data["color"], 16)

                    display_api.display_set_pixel(
                        cmd, 
                        display_api.DISPLAY_WNO,
                        display_api.display_cb_non,
                        "")
                    
                except Exception as e:
                    print(f"Failed to parse pixel command at line {i+1}: {e}")

                # time.sleep(0.01) 
                i += 2
                continue

        i += 1

if __name__ == "__main__":
    scan_and_draw(path.join(os.getcwd(), "source", "examples", "agent", "instructions.txt"))