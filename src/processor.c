#include "compiler.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define fullpath _fullpath
#else
#include <limits.h>
#include <unistd.h>
#define fullpath realpath
#endif

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return NULL;
    }
    rewind(file);

    char *buffer = xmalloc((size_t)size + 1);
    size_t read = fread(buffer, 1, (size_t)size, file);
    if (read != (size_t)size && ferror(file)) {
        free(buffer);
        fclose(file);
        return NULL;
    }
    buffer[read] = '\0';
    fclose(file);
    return buffer;
}

static char *absolute_path(const char *path) {
#ifdef _WIN32
    char *resolved = fullpath(NULL, path, 0);
    return resolved ? resolved : xstrdup(path);
#else
    char resolved[PATH_MAX];
    return fullpath(path, resolved) ? xstrdup(resolved) : xstrdup(path);
#endif
}

static char *join_path(const char *dir, const char *name) {
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(name);
    int needs_sep = dir_len > 0 && dir[dir_len - 1] != '/' && dir[dir_len - 1] != '\\';
    char *path = xmalloc(dir_len + (needs_sep ? 1 : 0) + name_len + 1);

    memcpy(path, dir, dir_len);
    if (needs_sep) {
        path[dir_len++] = '\\';
    }
    memcpy(path + dir_len, name, name_len + 1);
    return path;
}

static char *dirname_copy(const char *path) {
    const char *last_slash = strrchr(path, '/');
    const char *last_backslash = strrchr(path, '\\');
    const char *last = last_slash > last_backslash ? last_slash : last_backslash;

    if (!last) {
        return xstrdup(".");
    }
    if (last == path) {
        return xstrndup(path, 1);
    }
    return xstrndup(path, (size_t)(last - path));
}

static int string_list_contains(const StringList *list, const char *item) {
    for (size_t i = 0; i < list->len; i++) {
        if (strcmp(list->items[i], item) == 0) {
            return 1;
        }
    }
    return 0;
}

static int parse_include(const char *line, char **include_file) {
    const char *p = line;
    while (isspace((unsigned char)*p)) {
        p++;
    }
    if (!starts_with(p, "#include")) {
        return 0;
    }
    p += strlen("#include");
    if (!isspace((unsigned char)*p)) {
        return 0;
    }
    while (isspace((unsigned char)*p)) {
        p++;
    }

    char close = '\0';
    if (*p == '"' || *p == '\'') {
        close = *p;
    } else if (*p == '<') {
        close = '>';
    } else {
        return 0;
    }
    p++;

    const char *start = p;
    while (*p && *p != close) {
        p++;
    }
    if (*p != close || p == start) {
        return 0;
    }

    *include_file = xstrndup(start, (size_t)(p - start));
    return 1;
}

static char *resolve_includes(const char *source, const char *current_dir, StringList *visited_files, int *ok) {
    StringList lines = split_lines(source);
    char *resolved = NULL;
    size_t len = 0;
    size_t cap = 0;

    for (size_t i = 0; i < lines.len && *ok; i++) {
        char *include_file = NULL;
        if (parse_include(lines.items[i], &include_file)) {
            char *joined = join_path(current_dir, include_file);
            char *full_path = absolute_path(joined);

            if (!string_list_contains(visited_files, full_path)) {
                char *include_content = read_file(full_path);
                if (!include_content) {
                    fprintf(stderr, "preprocessor error: could not find library '%s' at '%s'\n", include_file, full_path);
                    *ok = 0;
                } else {
                    list_push(visited_files, xstrdup(full_path));
                    char *include_dir = dirname_copy(full_path);
                    char *included = resolve_includes(include_content, include_dir, visited_files, ok);
                    if (*ok) {
                        if (len > 0) {
                            builder_append(&resolved, &len, &cap, "\n");
                        }
                        builder_append(&resolved, &len, &cap, included);
                    }
                    free(included);
                    free(include_dir);
                    free(include_content);
                }
            }

            free(full_path);
            free(joined);
            free(include_file);
        } else {
            if (len > 0) {
                builder_append(&resolved, &len, &cap, "\n");
            }
            builder_append(&resolved, &len, &cap, lines.items[i]);
        }
    }

    list_free(&lines);
    if (!resolved) {
        resolved = xstrdup("");
    }
    return resolved;
}

static const char *find_object_macro(const MacroList *macros, const char *name) {
    for (size_t i = 0; i < macros->len; i++) {
        if (strcmp(macros->items[i].name, name) == 0) {
            return macros->items[i].value;
        }
    }
    return NULL;
}

static StringList split_args(const char *text) {
    StringList args = {0};
    const char *start = text;
    for (const char *p = text; ; p++) {
        if (*p == ',' || *p == '\0') {
            char *arg = trim_copy_range(start, p);
            if (arg[0] != '\0') {
                list_push(&args, arg);
            } else {
                free(arg);
            }
            if (*p == '\0') {
                break;
            }
            start = p + 1;
        }
    }
    return args;
}

static void collect_macros(const StringList *raw_lines, MacroList *macros, FuncMacroList *func_macros) {
    for (size_t i = 0; i < raw_lines->len; i++) {
        char *stripped = trim_copy(raw_lines->items[i]);
        if (!starts_with(stripped, "#define")) {
            free(stripped);
            continue;
        }

        const char *p = stripped + strlen("#define");
        while (isspace((unsigned char)*p)) {
            p++;
        }

        const char *name_start = p;
        while (word_char(*p)) {
            p++;
        }
        if (p == name_start) {
            free(stripped);
            continue;
        }

        char *name = xstrndup(name_start, (size_t)(p - name_start));
        if (*p == '(') {
            const char *args_start = p + 1;
            const char *args_end = strchr(args_start, ')');
            if (!args_end) {
                free(name);
                free(stripped);
                continue;
            }
            p = args_end + 1;
            if (!isspace((unsigned char)*p)) {
                free(name);
                free(stripped);
                continue;
            }
            while (isspace((unsigned char)*p)) {
                p++;
            }
            FuncMacro macro = {0};
            char *args_text = xstrndup(args_start, (size_t)(args_end - args_start));
            macro.name = name;
            macro.args = split_args(args_text);
            free(args_text);
            macro.body = trim_copy(p);
            func_macro_push(func_macros, macro);
        } else {
            if (!isspace((unsigned char)*p)) {
                free(name);
                free(stripped);
                continue;
            }
            while (isspace((unsigned char)*p)) {
                p++;
            }
            if (*p != '\0') {
                macro_push(macros, name, trim_copy(p));
            } else {
                free(name);
            }
        }
        free(stripped);
    }
}

static char *strip_comments(const char *line) {
    const char *hash = strchr(line, '#');
    return trim_copy_range(line, hash ? hash : line + strlen(line));
}

static char *replace_word(const char *body, const char *param, const char *value) {
    char *out = NULL;
    size_t len = 0;
    size_t cap = 0;
    size_t param_len = strlen(param);

    for (const char *p = body; *p; ) {
        int left_ok = (p == body) || !word_char(*(p - 1));
        int right_ok = strncmp(p, param, param_len) == 0 && !word_char(p[param_len]);
        if (left_ok && right_ok) {
            builder_append_re_sub_value(&out, &len, &cap, value);
            p += param_len;
        } else {
            builder_append_n(&out, &len, &cap, p, 1);
            p++;
        }
    }

    if (!out) {
        out = xstrdup("");
    }
    return out;
}

static char *expand_one_func_call(const char *line, const FuncMacro *macro) {
    char *out = NULL;
    size_t len = 0;
    size_t cap = 0;
    size_t name_len = strlen(macro->name);

    for (const char *p = line; *p; ) {
        int left_ok = (p == line) || !word_char(*(p - 1));
        int match = left_ok && strncmp(p, macro->name, name_len) == 0 && p[name_len] == '(';
        if (!match) {
            builder_append_n(&out, &len, &cap, p, 1);
            p++;
            continue;
        }

        const char *arg_start = p + name_len + 1;
        const char *arg_end = strchr(arg_start, ')');
        if (!arg_end) {
            builder_append_n(&out, &len, &cap, p, 1);
            p++;
            continue;
        }

        char *arg_text = xstrndup(arg_start, (size_t)(arg_end - arg_start));
        StringList call_args = split_args(arg_text);
        free(arg_text);

        if (call_args.len != macro->args.len) {
            fprintf(stderr, "macro %s: expected %zu args, got %zu\n", macro->name, macro->args.len, call_args.len);
            list_free(&call_args);
            free(out);
            return NULL;
        }

        char *body = xstrdup(macro->body);
        for (size_t i = 0; i < macro->args.len; i++) {
            char *next = replace_word(body, macro->args.items[i], call_args.items[i]);
            free(body);
            body = next;
        }

        builder_append(&out, &len, &cap, body);
        free(body);
        list_free(&call_args);
        p = arg_end + 1;
    }

    if (!out) {
        out = xstrdup("");
    }
    return out;
}

static char *expand_function_macros(const char *line, const FuncMacroList *func_macros) {
    char *current = xstrdup(line);
    for (size_t i = 0; i < func_macros->len; i++) {
        char *next = expand_one_func_call(current, &func_macros->items[i]);
        free(current);
        if (!next) {
            return NULL;
        }
        current = next;
    }
    return current;
}

static char *expand_object_macros(const char *line, const MacroList *macros) {
    char *out = NULL;
    size_t len = 0;
    size_t cap = 0;
    int first = 1;
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
        char *token = xstrndup(start, (size_t)(p - start));
        const char *replacement = find_object_macro(macros, token);
        if (!first) {
            builder_append(&out, &len, &cap, " ");
        }
        builder_append(&out, &len, &cap, replacement ? replacement : token);
        first = 0;
        free(token);
    }

    if (!out) {
        out = xstrdup("");
    }
    return out;
}

static void split_multiline_into(const char *text, StringList *result_lines) {
    char *expanded = NULL;
    size_t len = 0;
    size_t cap = 0;
    for (const char *p = text; *p; p++) {
        if (p[0] == '\\' && p[1] == 'n') {
            builder_append(&expanded, &len, &cap, "\n");
            p++;
        } else {
            builder_append_n(&expanded, &len, &cap, p, 1);
        }
    }
    if (!expanded) {
        expanded = xstrdup("");
    }

    const char *start = expanded;
    for (const char *p = expanded; ; p++) {
        if (*p == '\n' || *p == '\0') {
            char *sub = trim_copy_range(start, p);
            if (sub[0] != '\0') {
                list_push(result_lines, sub);
            } else {
                free(sub);
            }
            if (*p == '\0') {
                break;
            }
            start = p + 1;
        }
    }
    free(expanded);
}

StringList preprocess(const char *source_code, const char *current_dir, int *ok) {
    StringList visited_files = {0};
    char *expanded_source = resolve_includes(source_code, current_dir ? current_dir : ".", &visited_files, ok);
    list_free(&visited_files);
    if (!*ok) {
        free(expanded_source);
        return (StringList){0};
    }

    StringList raw_lines = split_lines(expanded_source);
    MacroList macros = {0};
    FuncMacroList func_macros = {0};
    StringList result_lines = {0};

    free(expanded_source);

    collect_macros(&raw_lines, &macros, &func_macros);

    for (size_t i = 0; i < raw_lines.len; i++) {
        char *stripped = trim_copy(raw_lines.items[i]);
        if (starts_with(stripped, "#define")) {
            free(stripped);
            continue;
        }
        free(stripped);

        char *clean = strip_comments(raw_lines.items[i]);
        if (clean[0] == '\0') {
            free(clean);
            continue;
        }

        char *func_expanded = expand_function_macros(clean, &func_macros);
        free(clean);
        if (!func_expanded) {
            continue;
        }
        char *object_expanded = expand_object_macros(func_expanded, &macros);
        free(func_expanded);
        split_multiline_into(object_expanded, &result_lines);
        free(object_expanded);
    }

    list_free(&raw_lines);
    macros_free(&macros);
    func_macros_free(&func_macros);
    return result_lines;
}
