#ifndef S21_GREP_H
#define S21_GREP_H

#include <ctype.h>
#include <regex.h>

#include "../common/s21_common.h"

typedef struct {
  int ignore_case;
  int invert_match;
  int count;
  int files_with_matches;
  int line_number;
  int no_filename;
  int suppress_errors;
  int file_patterns;
  int only_matching;
  int multiple_files;
  char *patterns[MAX_PATTERNS];
  int pattern_count;
} grep_flags;

// Инициализация и валидация
void init_flags(grep_flags *flags);
void validate_flags(const grep_flags *flags, error_handler_t *error_handler);

// Парсинг аргументов
int parse_arguments(int argc, char *argv[], grep_flags *flags, char *files[],
                    error_handler_t *error_handler);

// Обработка файлов
void process_files(grep_flags *flags, char *files[], int file_count);
void load_patterns_from_file(grep_flags *flags, const char *filename,
                             error_handler_t *error_handler);

// Вспомогательные функции
void compile_regex(regex_t *regex, const char *pattern, int ignore_case,
                   error_handler_t *error_handler);
void print_match(const char *filename, int line_num, const char *line,
                 const grep_flags *flags);
void print_count(const char *filename, int count, const grep_flags *flags);

#endif