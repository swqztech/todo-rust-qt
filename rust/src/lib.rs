mod todo_store;

use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::path::PathBuf;
use std::sync::Mutex;

use todo_store::TodoStore;

static STORE: Mutex<Option<TodoStore>> = Mutex::new(None);

fn store_mut() -> std::sync::MutexGuard<'static, Option<TodoStore>> {
    STORE.lock().unwrap()
}

#[no_mangle]
pub extern "C" fn todo_free_string(s: *mut c_char) {
    if s.is_null() {
        return;
    }
    unsafe {
        drop(CString::from_raw(s));
    }
}

fn to_c_string(s: String) -> *mut c_char {
    CString::new(s).unwrap_or_default().into_raw()
}

#[no_mangle]
pub extern "C" fn todo_init(data_dir: *const c_char) {
    let dir = if data_dir.is_null() {
        PathBuf::from(".")
    } else {
        unsafe {
            CStr::from_ptr(data_dir)
                .to_str()
                .map(PathBuf::from)
                .unwrap_or_else(|_| PathBuf::from("."))
        }
    };
    let path = dir.join(format!("{}.json", todo_store::TodoStore::storage_key()));
    *store_mut() = Some(TodoStore::new(path));
}

#[no_mangle]
pub extern "C" fn todo_snapshot_json() -> *mut c_char {
    let guard = store_mut();
    let s = guard
        .as_ref()
        .map(|st| st.snapshot_json())
        .unwrap_or_else(|| "[]".into());
    to_c_string(s)
}

#[no_mangle]
pub extern "C" fn todo_counts_label() -> *mut c_char {
    let guard = store_mut();
    let s = guard
        .as_ref()
        .map(|st| st.counts_label())
        .unwrap_or_else(|| "".into());
    to_c_string(s)
}

#[no_mangle]
pub extern "C" fn todo_add(text: *const c_char) {
    if text.is_null() {
        return;
    }
    let t = unsafe { CStr::from_ptr(text).to_string_lossy() };
    if let Some(st) = store_mut().as_mut() {
        st.add(&t);
    }
}

#[no_mangle]
pub extern "C" fn todo_set_done(index: usize, done: i32) {
    if let Some(st) = store_mut().as_mut() {
        st.set_done(index, done != 0);
    }
}

#[no_mangle]
pub extern "C" fn todo_remove_at(index: usize) {
    if let Some(st) = store_mut().as_mut() {
        st.remove_at(index);
    }
}

#[no_mangle]
pub extern "C" fn todo_move_item(from: usize, to: usize) {
    if let Some(st) = store_mut().as_mut() {
        st.move_item(from, to);
    }
}

#[no_mangle]
pub extern "C" fn todo_update_text(index: usize, text: *const c_char) {
    if text.is_null() {
        return;
    }
    let t = unsafe { CStr::from_ptr(text).to_string_lossy() };
    if let Some(st) = store_mut().as_mut() {
        st.update_text(index, &t);
    }
}

#[no_mangle]
pub extern "C" fn todo_import_markdown(content: *const c_char) {
    if content.is_null() {
        return;
    }
    let t = unsafe { CStr::from_ptr(content).to_string_lossy() };
    if let Some(st) = store_mut().as_mut() {
        st.import_markdown(&t);
    }
}

#[no_mangle]
pub extern "C" fn todo_export_markdown() -> *mut c_char {
    let guard = store_mut();
    let s = guard
        .as_ref()
        .map(|st| st.export_markdown())
        .unwrap_or_else(|| "".into());
    to_c_string(s)
}

#[no_mangle]
pub extern "C" fn todo_import_json(json: *const c_char) -> *mut c_char {
    if json.is_null() {
        return to_c_string("空内容".into());
    }
    let t = unsafe { CStr::from_ptr(json).to_string_lossy() };
    let mut guard = store_mut();
    if let Some(st) = guard.as_mut() {
        match st.replace_from_json(&t) {
            Ok(()) => return std::ptr::null_mut(),
            Err(e) => return to_c_string(e),
        }
    }
    to_c_string("未初始化".into())
}
