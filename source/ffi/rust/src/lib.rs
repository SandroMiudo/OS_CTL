use std::ffi::CString;
use std::os::raw::{c_char, c_int};

mod bindings;

pub use bindings::*; // your bindgen module

fn with_c_string<F>(msg: Option<&str>, f: F)
where F: FnOnce(*mut c_char) {
    let msg_str = msg.unwrap_or("");
    let c_msg = CString::new(msg_str).expect("Message contains null byte");
    f(c_msg.as_ptr() as *mut c_char);
    // CString is dropped here automatically, so Rust frees it
}

pub fn draw_image(
    cmd: &mut cmd_draw_image_t,
    flags: display_flags_t,
    msg: Option<&str>,
    cb: display_callback_f) {
    with_c_string(msg, |msg_ptr| unsafe {
        display_draw_image(cmd, flags, cb, msg_ptr);
    });
}

pub fn set_pixel(
    cmd: &mut cmd_set_pixel_t,
    flags: display_flags_t,
    msg: Option<&str>,
    cb: display_callback_f) {
    with_c_string(msg, |msg_ptr| unsafe {
        display_set_pixel(cmd, flags, cb, msg_ptr);
    });
}

pub fn clear_screen(
    cmd: &mut cmd_clear_screen_t,
    flags: display_flags_t,
    msg: Option<&str>,
    cb: display_callback_f) {
    with_c_string(msg, |msg_ptr| unsafe {
        display_clear_screen(cmd, flags, cb, msg_ptr);
    });
}

pub fn fill_screen(
    cmd: &mut cmd_fill_screen_t,
    flags: display_flags_t,
    msg: Option<&str>,
    cb: display_callback_f) {
    with_c_string(msg, |msg_ptr| unsafe {
        display_fill_screen(cmd, flags, cb, msg_ptr);
    });
}