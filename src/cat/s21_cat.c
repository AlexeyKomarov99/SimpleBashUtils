#include "s21_cat.h"

#include <string.h>

void init_flags(cat_flags *flags) {
  flags->number_nonblank = 0;
  flags->show_ends = 0;
  flags->number = 0;
  flags->squeeze_blank = 0;
  flags->show_tabs = 0;
  flags->show_nonprinting = 0;
  flags->show_nonprinting_tabs = 0;
  flags->T_flag_only = 0;
}

void resolve_flag_conflicts(cat_flags *flags) {
  if (flags->number_nonblank) {
    flags->number = 0;
  }

  if (flags->show_nonprinting_tabs && !flags->T_flag_only) {
    flags->show_nonprinting = 1;
  }
}

int parse_gnu_flag(const char *flag_str, cat_flags *flags,
                   error_handler_t *error_handler) {
  int result = 1;

  if (strcmp(flag_str, "number-nonblank") == 0) {
    flags->number_nonblank = 1;
  } else if (strcmp(flag_str, "number") == 0) {
    flags->number = 1;
  } else if (strcmp(flag_str, "squeeze-blank") == 0) {
    flags->squeeze_blank = 1;
  } else if (strcmp(flag_str, "show-ends") == 0) {
    flags->show_ends = 1;
    flags->show_nonprinting = 1;
  } else if (strcmp(flag_str, "show-tabs") == 0) {
    flags->show_nonprinting_tabs = 1;
    flags->show_nonprinting = 1;
  } else if (strcmp(flag_str, "show-nonprinting") == 0) {
    flags->show_nonprinting = 1;
  } else {
    set_error(error_handler, "unrecognized option '--%s'", flag_str);
    result = 0;
  }

  return result;
}

int parse_short_flags(const char *flag_str, cat_flags *flags,
                      error_handler_t *error_handler) {
  int result = 1;

  for (size_t i = 0; flag_str[i] != '\0' && result; i++) {
    switch (flag_str[i]) {
      case 'b':
        flags->number_nonblank = 1;
        break;
      case 'e':
        flags->show_ends = 1;
        flags->show_nonprinting = 1;
        break;
      case 'E':
        flags->show_ends = 1;
        break;
      case 'n':
        flags->number = 1;
        break;
      case 's':
        flags->squeeze_blank = 1;
        break;
      case 't':
        flags->show_nonprinting_tabs = 1;
        flags->show_nonprinting = 1;
        break;
      case 'T':
        flags->show_nonprinting_tabs = 1;
        flags->T_flag_only = 1;
        break;
      case 'v':
        flags->show_nonprinting = 1;
        break;
      default:
        set_error(error_handler, "invalid option -- '%s'", &flag_str[i]);
        result = 0;
        break;
    }
  }

  return result;
}

int parse_single_flag(const char *arg, cat_flags *flags,
                      error_handler_t *error_handler) {
  int result = 1;

  if (arg[1] == '-') {
    result = parse_gnu_flag(arg + 2, flags, error_handler);
  } else {
    result = parse_short_flags(arg + 1, flags, error_handler);
  }

  return result;
}

int parse_arguments(int argc, char *argv[], cat_flags *flags, char *files[],
                    error_handler_t *error_handler) {
  int file_count = 0;

  for (int i = 1; i < argc && !error_handler->has_error; i++) {
    if (argv[i][0] == '\0') {
      continue;
    }

    if (argv[i][0] == '-') {
      if (!parse_single_flag(argv[i], flags, error_handler)) {
        break;
      }
    } else {
      if (file_count < MAX_FILES) {
        files[file_count++] = argv[i];
      }
    }
  }

  return error_handler->has_error ? -1 : file_count;
}

void print_nonprinting(unsigned char c) {
  if (c < 32 && c != 9 && c != 10) {
    printf("^%c", c + 64);
  } else if (c == 127) {
    printf("^?");
  } else if (c > 127 && c < 160) {
    printf("M-^%c", c - 64);
  } else if (c >= 160 && c < 255) {
    printf("M-%c", c - 128);
  } else if (c == 255) {
    printf("M-^?");
  } else {
    putchar(c);
  }
}

void process_line_content(const char *line, ssize_t len,
                          const cat_flags *flags) {
  for (ssize_t i = 0; i < len; i++) {
    unsigned char c = line[i];

    if ((flags->show_nonprinting_tabs || flags->T_flag_only) && c == '\t') {
      printf("^I");
      continue;
    }

    if (flags->show_ends && c == '\n') {
      printf("$\n");
      continue;
    }

    if (flags->show_nonprinting && !flags->T_flag_only && c != '\n') {
      print_nonprinting(c);
    } else {
      putchar(c);
    }
  }
}

void process_single_file(FILE *file, const cat_flags *flags) {
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int line_number = 1;
  int consecutive_blank = 0;

  while ((read = getline(&line, &len, file)) != -1) {
    int is_blank = (read <= 1 || (line[0] == '\n'));

    if (flags->squeeze_blank && is_blank) {
      consecutive_blank++;
      if (consecutive_blank > 1) {
        continue;
      }
    } else {
      consecutive_blank = 0;
    }

    if (flags->number || (flags->number_nonblank && !is_blank)) {
      printf("%6d\t", line_number++);
    }

    process_line_content(line, read, flags);
  }

  free(line);
}

void process_files(const char *const files[], int file_count,
                   cat_flags *flags) {
  resolve_flag_conflicts(flags);

  if (file_count == 0) {
    process_single_file(stdin, flags);
    return;
  }

  for (int i = 0; i < file_count; i++) {
    FILE *file = fopen(files[i], "r");
    if (file) {
      process_single_file(file, flags);
      fclose(file);
    } else {
      fprintf(stderr, "s21_cat: %s: No such file or directory\n", files[i]);
    }
  }
}

int main(int argc, char *argv[]) {
  cat_flags flags;
  error_handler_t error_handler;
  char *files[MAX_FILES];
  int exit_code = 0;

  init_flags(&flags);
  init_error_handler(&error_handler);

  int file_count = parse_arguments(argc, argv, &flags, files, &error_handler);

  if (error_handler.has_error) {
    fprintf(stderr, "s21_cat: %s\n", error_handler.error_message);
    exit_code = 1;
  } else {
    process_files((const char *const *)files, file_count, &flags);
  }

  return exit_code;
}