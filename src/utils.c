#include "compiler.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *xmalloc(size_t size) {
    void *ptr = malloc(size ? size : 1);
    if (!ptr) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return ptr;
}

void *xrealloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size ? size : 1);
    if (!new_ptr) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return new_ptr;
}

char *xstrdup(const char *text) {
    size_t len = strlen(text);
    char *copy = xmalloc(len + 1);
    memcpy(copy, text, len + 1);
    return copy;
}

char *xstrndup(const char *start, size_t len) {
    char *copy = xmalloc(len + 1);
    memcpy(copy, start, len);
    copy[len] = '\0';
    return copy;
}

void list_push(StringList *list, char *item) {
    if (list->len == list->cap) {
        list->cap = list->cap ? list->cap * 2 : 8;
        list->items = xrealloc(list->items, list->cap * sizeof(*list->items));
    }
    list->items[list->len++] = item;
}

void list_free(StringList *list) {
    for (size_t i = 0; i < list->len; i++) {
        free(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->len = 0;
    list->cap = 0;
}

void macro_push(MacroList *list, char *name, char *value) {
    if (list->len == list->cap) {
        list->cap = list->cap ? list->cap * 2 : 8;
        list->items = xrealloc(list->items, list->cap * sizeof(*list->items));
    }
    list->items[list->len++] = (Macro){name, value};
}

void macros_free(MacroList *list) {
    for (size_t i = 0; i < list->len; i++) {
        free(list->items[i].name);
        free(list->items[i].value);
    }
    free(list->items);
}

void func_macro_push(FuncMacroList *list, FuncMacro macro) {
    if (list->len == list->cap) {
        list->cap = list->cap ? list->cap * 2 : 8;
        list->items = xrealloc(list->items, list->cap * sizeof(*list->items));
    }
    list->items[list->len++] = macro;
}

void func_macros_free(FuncMacroList *list) {
    for (size_t i = 0; i < list->len; i++) {
        free(list->items[i].name);
        list_free(&list->items[i].args);
        free(list->items[i].body);
    }
    free(list->items);
}

void label_push(LabelList *list, char *name, size_t index) {
    if (list->len == list->cap) {
        list->cap = list->cap ? list->cap * 2 : 8;
        list->items = xrealloc(list->items, list->cap * sizeof(*list->items));
    }
    list->items[list->len++] = (Label){name, index};
}

void labels_free(LabelList *list) {
    for (size_t i = 0; i < list->len; i++) {
        free(list->items[i].name);
    }
    free(list->items);
}

void builder_append(char **buf, size_t *len, size_t *cap, const char *text) {
    size_t add = strlen(text);
    if (*len + add + 1 > *cap) {
        while (*len + add + 1 > *cap) {
            *cap = *cap ? *cap * 2 : 64;
        }
        *buf = xrealloc(*buf, *cap);
    }
    memcpy(*buf + *len, text, add);
    *len += add;
    (*buf)[*len] = '\0';
}

void builder_append_n(char **buf, size_t *len, size_t *cap, const char *text, size_t n) {
    if (*len + n + 1 > *cap) {
        while (*len + n + 1 > *cap) {
            *cap = *cap ? *cap * 2 : 64;
        }
        *buf = xrealloc(*buf, *cap);
    }
    memcpy(*buf + *len, text, n);
    *len += n;
    (*buf)[*len] = '\0';
}

void builder_append_re_sub_value(char **buf, size_t *len, size_t *cap, const char *text) {
    for (const char *p = text; *p; p++) {
        if (p[0] == '\\' && p[1] == 'n') {
            builder_append(buf, len, cap, "\n");
            p++;
        } else {
            builder_append_n(buf, len, cap, p, 1);
        }
    }
}

char *trim_copy_range(const char *start, const char *end) {
    while (start < end && isspace((unsigned char)*start)) {
        start++;
    }
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }
    return xstrndup(start, (size_t)(end - start));
}

char *trim_copy(const char *text) {
    return trim_copy_range(text, text + strlen(text));
}

int starts_with(const char *text, const char *prefix) {
    return strncmp(text, prefix, strlen(prefix)) == 0;
}

int word_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

StringList split_lines(const char *source) {
    StringList lines = {0};
    const char *start = source;
    for (const char *p = source; ; p++) {
        if (*p == '\n' || *p == '\0') {
            const char *end = p;
            if (end > start && *(end - 1) == '\r') {
                end--;
            }
            list_push(&lines, xstrndup(start, (size_t)(end - start)));
            if (*p == '\0') {
                break;
            }
            start = p + 1;
        }
    }
    return lines;
}

StringList split_tokens(const char *line) {
    StringList tokens = {0};
    const char *p = line;
    while (*p) {
        while (isspace((unsigned char)*p)) {
            p++;
        }
        if (*p == '\0') {
            break;
        }
        const char *start = p;
        while (*p && !isspace((unsigned char)*p)) {
            p++;
        }
        list_push(&tokens, xstrndup(start, (size_t)(p - start)));
    }
    return tokens;
}

char *join_tokens(const StringList *tokens) {
    char *out = NULL;
    size_t len = 0;
    size_t cap = 0;
    for (size_t i = 0; i < tokens->len; i++) {
        if (i > 0) {
            builder_append(&out, &len, &cap, " ");
        }
        builder_append(&out, &len, &cap, tokens->items[i]);
    }
    if (!out) {
        out = xstrdup("");
    }
    return out;
}
