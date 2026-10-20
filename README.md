# 墨迹清单 Rust + Qt

Windows 桌面待办：**Qt 6 Widgets** + **Rust** 存储与 Markdown/JSON 逻辑。

- 与 `todo-taro` / `todo-miniapp` / `todo-android` 数据字段一致（`todo_taro_todos.json`）
- 筛选、上移/下移、导出剪贴板、文件/粘贴导入

## 依赖

- Qt 6.8（默认 `../.qt/6.8.2/msvc2022_64`）
- Rust + CMake 3.21+ + MSVC 2022

## 构建

```powershell
cmake -B build -DCMAKE_PREFIX_PATH=D:\codes\soft\.qt\6.8.2\msvc2022_64
cmake --build build --config Release
```

数据目录：`%AppData%/<组织>/todo-rust-qt/` 下的 `todo_taro_todos.json`。
