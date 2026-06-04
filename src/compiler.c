#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file) {
        perror(path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        perror(path);
        fclose(file);
        return NULL;
    }
    long size = ftell(file);
    if (size < 0) {
        perror(path);
        fclose(file);
        return NULL;
    }
    rewind(file);

    char *buffer = xmalloc((size_t)size + 1);
    size_t read = fread(buffer, 1, (size_t)size, file);
    if (read != (size_t)size && ferror(file)) {
        perror(path);
        free(buffer);
        fclose(file);
        return NULL;
    }
    buffer[read] = '\0';
    fclose(file);
    return buffer;
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

static int line_ends_with_colon(const char *line) {
    size_t len = strlen(line);
    return len > 0 && line[len - 1] == ':';
}

static int find_label(const LabelList *labels, const char *name, size_t *index) {
    for (size_t i = 0; i < labels->len; i++) {
        if (strcmp(labels->items[i].name, name) == 0) {
            *index = labels->items[i].index;
            return 1;
        }
    }
    return 0;
}

char *compile_mlog_in_dir(const char *source_code, const char *current_dir) {
    int ok = 1;
    StringList source = preprocess(source_code, current_dir, &ok);
    if (!ok) {
        list_free(&source);
        return NULL;
    }

    StringList cleaned = {0};
    LabelList labels = {0};
    size_t idx = 0;

    for (size_t i = 0; i < source.len; i++) {
        if (line_ends_with_colon(source.items[i])) {
            char *label = xstrndup(source.items[i], strlen(source.items[i]) - 1);
            char *trimmed = trim_copy(label);
            free(label);
            label_push(&labels, trimmed, idx);
        } else {
            list_push(&cleaned, xstrdup(source.items[i]));
            idx++;
        }
    }

    char *compiled = NULL;
    size_t len = 0;
    size_t cap = 0;
    for (size_t i = 0; i < cleaned.len; i++) {
        char *line = NULL;
        StringList parts = split_tokens(cleaned.items[i]);

        if (parts.len > 1 && strcmp(parts.items[0], "jump") == 0) {
            size_t target_index = 0;
            if (find_label(&labels, parts.items[1], &target_index)) {
                char target[32];
                snprintf(target, sizeof(target), "%zu", target_index);
                free(parts.items[1]);
                parts.items[1] = xstrdup(target);
            }
            line = join_tokens(&parts);
        } else {
            line = xstrdup(cleaned.items[i]);
        }

        if (i > 0) {
            builder_append(&compiled, &len, &cap, "\n");
        }
        builder_append(&compiled, &len, &cap, line);
        free(line);
        list_free(&parts);
    }

    if (!compiled) {
        compiled = xstrdup("");
    }

    list_free(&source);
    list_free(&cleaned);
    labels_free(&labels);
    return compiled;
}

char *compile_mlog(const char *source_code) {
    return compile_mlog_in_dir(source_code, ".");
}

char *compile_mlog_file(const char *main_file_path, const char **lib_paths, size_t lib_count) {
    char *main_content = read_file(main_file_path);
    if (!main_content) {
        return NULL;
    }

    char *full_source = NULL;
    size_t len = 0;
    size_t cap = 0;
    for (size_t i = 0; i < lib_count; i++) {
        builder_append(&full_source, &len, &cap, "#include \"");
        builder_append(&full_source, &len, &cap, lib_paths[i]);
        builder_append(&full_source, &len, &cap, "\"\n");
    }
    builder_append(&full_source, &len, &cap, main_content);

    char *base_dir = dirname_copy(main_file_path);
    char *compiled = compile_mlog_in_dir(full_source, base_dir);

    free(base_dir);
    free(full_source);
    free(main_content);
    return compiled;
}
