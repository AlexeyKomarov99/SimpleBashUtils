#ifndef S21_CAT_H
#define S21_CAT_H

#include "../common/s21_common.h"

typedef struct {
  int number_nonblank;
  int show_ends;
  int number;
  int squeeze_blank;
  int show_tabs;
  int show_nonprinting;
  int show_nonprinting_tabs;
  int T_flag_only;
} cat_flags;

void init_flags(cat_flags *flags);
void resolve_flag_conflicts(cat_flags *flags);
int parse_arguments(int argc, char *argv[], cat_flags *flags, char *files[],
                    error_handler_t *error_handler);
void process_files(const char *const files[], int file_count, cat_flags *flags);
void process_single_file(FILE *file, const cat_flags *flags);
void print_nonprinting(unsigned char c);
void process_line_content(const char *line, ssize_t len,
                          const cat_flags *flags);

#endif