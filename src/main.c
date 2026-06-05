#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

char *read_file(const char *path) {
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

    char *buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fprintf(stderr, "out of memory\n");
        fclose(file);
        return NULL;
    }

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

int write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "w");
    if (!file) {
        perror(path);
        return 0;
    }
    fputs(text, file);
    if (fclose(file) != 0) {
        perror(path);
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int exit_code = 1;

    char *source = NULL;
    char *compiled_code = NULL;
    char *result_from_file = NULL;
    const char **lib_paths = NULL;

    setlocale(LC_ALL, "en_US.UTF-8");

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input.mlog> <output.mlog> [library.mlog ...]\n", argv[0]);
        goto cleanup;
    }

    source = read_file(argv[1]);
    if (!source) goto cleanup;

    printf("--- Source Code ---\n");
    printf("%s", source);

    if (source[0] != '\0' && source[strlen(source) - 1] != '\n') {
        printf("\n");
    }

    static const char *default_libs[] = {
        "./libs/math.mlog",
        "./libs/display.mlog"
    };

    size_t default_count =
        sizeof(default_libs) / sizeof(default_libs[0]);

    size_t user_count =
        argc > 3 ? (size_t)(argc - 3) : 0;

    size_t lib_count = default_count + user_count;

    lib_paths = malloc(lib_count * sizeof(*lib_paths));
    if (!lib_paths) {
        fprintf(stderr, "out of memory\n");
        goto cleanup;
    }

    memcpy(lib_paths, default_libs, default_count * sizeof(*lib_paths));

    if (user_count > 0) {
        memcpy(lib_paths + default_count, argv + 3, user_count * sizeof(*lib_paths));
    }

    compiled_code = compile_mlog_file(argv[1], lib_paths, lib_count);

    if (!compiled_code) {
        goto cleanup;
    }

    printf("\n--- Compiled Code ---\n");
    printf("%s\n", compiled_code);

    if (!write_file(argv[2], compiled_code)) {
        goto cleanup;
    }

    result_from_file = read_file(argv[2]);
    if (!result_from_file) {
        goto cleanup;
    }

    printf("\n--- Output File ---\n");
    printf("%s\n", result_from_file);

    exit_code = 0;

cleanup:
    free(lib_paths);
    free(result_from_file);
    free(compiled_code);
    free(source);

    return exit_code;
}