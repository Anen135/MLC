#ifndef MINDUSTRY_COMPILER_H
#define MINDUSTRY_COMPILER_H

#include <stddef.h>

typedef struct {
    char **items;
    size_t len;
    size_t cap;
} StringList;

typedef struct {
    char *name;
    char *value;
} Macro;

typedef struct {
    Macro *items;
    size_t len;
    size_t cap;
} MacroList;

typedef struct {
    char *name;
    StringList args;
    char *body;
} FuncMacro;

typedef struct {
    FuncMacro *items;
    size_t len;
    size_t cap;
} FuncMacroList;

typedef struct {
    char *name;
    size_t index;
} Label;

typedef struct {
    Label *items;
    size_t len;
    size_t cap;
} LabelList;

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
char *xstrdup(const char *text);
char *xstrndup(const char *start, size_t len);

void list_push(StringList *list, char *item);
void list_free(StringList *list);

void macro_push(MacroList *list, char *name, char *value);
void macros_free(MacroList *list);

void func_macro_push(FuncMacroList *list, FuncMacro macro);
void func_macros_free(FuncMacroList *list);

void label_push(LabelList *list, char *name, size_t index);
void labels_free(LabelList *list);

void builder_append(char **buf, size_t *len, size_t *cap, const char *text);
void builder_append_n(char **buf, size_t *len, size_t *cap, const char *text, size_t n);
void builder_append_re_sub_value(char **buf, size_t *len, size_t *cap, const char *text);

char *trim_copy_range(const char *start, const char *end);
char *trim_copy(const char *text);
int starts_with(const char *text, const char *prefix);
int word_char(char c);

StringList split_lines(const char *source);
StringList split_tokens(const char *line);
char *join_tokens(const StringList *tokens);

StringList preprocess(const char *source_code, const char *current_dir, int *ok);
char *compile_mlog(const char *source_code);
char *compile_mlog_in_dir(const char *source_code, const char *current_dir);
char *compile_mlog_file(const char *main_file_path, const char **lib_paths, size_t lib_count);

#endif
