#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void todo_free_string(char* s);

void todo_init(const char* data_dir);

char* todo_snapshot_json(void);
char* todo_counts_label(void);

void todo_add(const char* text);
void todo_set_done(size_t index, int done);
void todo_remove_at(size_t index);
void todo_move_item(size_t from, size_t to);
void todo_update_text(size_t index, const char* text);

void todo_import_markdown(const char* content);
char* todo_export_markdown(void);
/** 成功返回 null，失败返回错误信息 */
char* todo_import_json(const char* json);

#ifdef __cplusplus
}
#endif
