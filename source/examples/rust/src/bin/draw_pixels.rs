use display_ffi::*;

fn main() {
    draw_diagonal_pattern(500, 500, 200, 200, display_flags_t_DISPLAY_WNO, None);
}

pub fn draw_diagonal_pattern(
    width: u32,
    height: u32,
    offset_x: u32,
    offset_y: u32,
    flags: display_flags_t,
    cb: display_callback_f) {

    let mut cmd = cmd_clear_screen_t {
        on: (BLACK as u8)
    };

    clear_screen(&mut cmd, flags, None, cb);

    for i in 0..width.min(height) {
        let mut cmd = cmd_set_pixel_t {
            x: i + offset_x,
            y: i + offset_y,
            color: (i * 0x01010101) as u32, // pseudo-color gradient
        };

        set_pixel(&mut cmd, flags, None, cb);
    }
}