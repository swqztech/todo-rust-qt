use serde::{Deserialize, Serialize};
use std::fs;
use std::path::PathBuf;

const STORAGE_KEY: &str = "todo_taro_todos";

#[derive(Clone, Serialize, Deserialize)]
pub struct TodoItem {
    #[serde(rename = "Id")]
    pub id: u32,
    #[serde(rename = "Text")]
    pub text: String,
    #[serde(rename = "Done")]
    pub done: bool,
}

pub struct TodoStore {
    path: PathBuf,
    items: Vec<TodoItem>,
    next_id: u32,
}

impl TodoStore {
    pub fn new(path: PathBuf) -> Self {
        let mut s = Self {
            path,
            items: Vec::new(),
            next_id: 1,
        };
        s.load();
        s
    }

    fn recompute_next_id(&mut self) {
        self.next_id = 1;
        for item in &self.items {
            if item.id >= self.next_id {
                self.next_id = item.id + 1;
            }
        }
    }

    pub fn load(&mut self) {
        self.items.clear();
        if !self.path.exists() {
            self.next_id = 1;
            return;
        }
        let raw = fs::read_to_string(&self.path).unwrap_or_default();
        if raw.trim().is_empty() {
            self.next_id = 1;
            return;
        }
        if let Ok(arr) = serde_json::from_str::<Vec<TodoItem>>(&raw) {
            self.items = arr;
            self.recompute_next_id();
        } else {
            self.next_id = 1;
        }
    }

    fn save(&self) {
        if let Some(parent) = self.path.parent() {
            let _ = fs::create_dir_all(parent);
        }
        let json = serde_json::to_string_pretty(&self.items).unwrap_or_else(|_| "[]".into());
        let _ = fs::write(&self.path, json);
    }

    pub fn snapshot_json(&self) -> String {
        serde_json::to_string(&self.items).unwrap_or_else(|_| "[]".into())
    }

    pub fn counts_label(&self) -> String {
        let left = self.items.iter().filter(|i| !i.done).count();
        let done = self.items.iter().filter(|i| i.done).count();
        format!("还剩 {} · 已完成 {}", left, done)
    }

    pub fn add(&mut self, text: &str) {
        let t = text.trim();
        if t.is_empty() {
            return;
        }
        self.items.push(TodoItem {
            id: self.next_id,
            text: t.to_string(),
            done: false,
        });
        self.next_id += 1;
        self.save();
    }

    fn add_done(&mut self, text: &str) {
        let t = text.trim();
        if t.is_empty() {
            return;
        }
        self.items.push(TodoItem {
            id: self.next_id,
            text: t.to_string(),
            done: true,
        });
        self.next_id += 1;
        self.save();
    }

    pub fn set_done(&mut self, index: usize, done: bool) {
        if index >= self.items.len() || self.items[index].done == done {
            return;
        }
        self.items[index].done = done;
        self.save();
    }

    pub fn remove_at(&mut self, index: usize) {
        if index < self.items.len() {
            self.items.remove(index);
            self.save();
        }
    }

    pub fn move_item(&mut self, from: usize, to: usize) {
        if from >= self.items.len() || to >= self.items.len() || from == to {
            return;
        }
        let row = self.items.remove(from);
        self.items.insert(to, row);
        self.save();
    }

    pub fn update_text(&mut self, index: usize, text: &str) {
        let t = text.trim();
        if t.is_empty() || index >= self.items.len() {
            return;
        }
        self.items[index].text = t.to_string();
        self.save();
    }

    pub fn import_markdown(&mut self, content: &str) {
        for raw in content.lines() {
            let line = raw.trim();
            if line.is_empty() {
                continue;
            }
            let lower = line.to_lowercase();
            if lower.starts_with("- [x]") {
                self.add_done(line[5..].trim());
            } else if line.starts_with("- [ ]") {
                self.add(line[5..].trim());
            } else {
                self.add(line);
            }
        }
    }

    pub fn export_markdown(&self) -> String {
        self.items
            .iter()
            .map(|i| {
                if i.done {
                    format!("- [x] {}", i.text)
                } else {
                    format!("- [ ] {}", i.text)
                }
            })
            .collect::<Vec<_>>()
            .join("\n")
    }

    pub fn replace_from_json(&mut self, json_text: &str) -> Result<(), String> {
        let arr: Vec<TodoItem> = serde_json::from_str(json_text).map_err(|e| e.to_string())?;
        self.items = arr;
        self.recompute_next_id();
        self.save();
        Ok(())
    }

    pub fn storage_key() -> &'static str {
        STORAGE_KEY
    }
}
