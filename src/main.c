#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

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

static int write_file(const char *path, const char *text) {
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
    setlocale(LC_ALL, "en_US.UTF-8");
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input.mlog> <output.mlog> [library.mlog ...]\n", argv[0]);
        return 1;
    }

    char *source = read_file(argv[1]);
    if (!source) {
        return 1;
    }

    printf("--- Исходный код ---\n");
    printf("%s", source);
    if (source[0] != '\0' && source[0] != '\n') {
        printf("\n");
    }

    const char **lib_paths = argc > 3 ? (const char **)&argv[3] : NULL;
    size_t lib_count = argc > 3 ? (size_t)(argc - 3) : 0;
    char *compiled_code = compile_mlog_file(argv[1], lib_paths, lib_count);
    if (!compiled_code) {
        free(source);
        return 1;
    }

    printf("--- Результат компиляции ---\n");
    printf("%s\n", compiled_code);

    if (!write_file(argv[2], compiled_code)) {
        free(compiled_code);
        free(source);
        return 1;
    }

    char *result_from_file = read_file(argv[2]);
    if (!result_from_file) {
        free(compiled_code);
        free(source);
        return 1;
    }

    printf("\n--- Проверка записанного файла ---\n");
    printf("%s\n", result_from_file);

    free(result_from_file);
    free(compiled_code);
    free(source);
    return 0;
}
