#ifndef S21_COMMON_H
#define S21_COMMON_H

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATTERNS 100
#define MAX_FILES 100

// Структура для обработки ошибок
typedef struct {
  int has_error;
  char error_message[256];
} error_handler_t;

// Функции для работы с ошибками
void init_error_handler(error_handler_t *handler);
void set_error(error_handler_t *handler, const char *format, ...);
void clear_error(error_handler_t *handler);

// Вспомогательные функции
int is_file_exists(const char *filename);
void safe_string_copy(char *dest, const char *src, size_t dest_size);

#endif