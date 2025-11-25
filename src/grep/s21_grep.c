#include "s21_grep.h"

void init_flags(grep_flags *flags) {
  flags->ignore_case = 0;
  flags->invert_match = 0;
  flags->count = 0;
  flags->files_with_matches = 0;
  flags->line_number = 0;
  flags->no_filename = 0;
  flags->suppress_errors = 0;
  flags->file_patterns = 0;
  flags->only_matching = 0;
  flags->multiple_files = 0;
  flags->pattern_count = 0;

  for (int i = 0; i < MAX_PATTERNS; i++) {
    flags->patterns[i] = NULL;
  }
}

void validate_flags(const grep_flags *flags, error_handler_t *error_handler) {
  if (flags->pattern_count == 0 && !flags->file_patterns) {
    set_error(error_handler, "pattern missing");
  }

  if (flags->pattern_count >= MAX_PATTERNS) {
    set_error(error_handler, "too many patterns");
  }
}

int handle_ef_flags(int argc, char *argv[], int *i, grep_flags *flags,
                    error_handler_t *error_handler) {
  int result = 1;

  if (*i + 1 >= argc) {
    set_error(error_handler, "option requires an argument -- '%c'",
              argv[*i][1]);
    result = 0;
  } else if (argv[*i][1] == 'e') {
    flags->patterns[flags->pattern_count++] = argv[++(*i)];
  } else if (argv[*i][1] == 'f') {
    flags->file_patterns = 1;
    load_patterns_from_file(flags, argv[++(*i)], error_handler);
  }

  return result;
}

int parse_short_flags(const char *arg, grep_flags *flags, int argc,
                      char *argv[], int *index,
                      error_handler_t *error_handler) {
  int result = 1;

  for (size_t j = 1; arg[j] != '\0' && result; j++) {
    switch (arg[j]) {
      case 'i':
        flags->ignore_case = 1;
        break;
      case 'v':
        flags->invert_match = 1;
        break;
      case 'c':
        flags->count = 1;
        break;
      case 'l':
        flags->files_with_matches = 1;
        break;
      case 'n':
        flags->line_number = 1;
        break;
      case 'h':
        flags->no_filename = 1;
        break;
      case 's':
        flags->suppress_errors = 1;
        break;
      case 'o':
        flags->only_matching = 1;
        break;
      case 'e':
      case 'f':
        result = handle_ef_flags(argc, argv, index, flags, error_handler);
        break;
      default:
        set_error(error_handler, "invalid option -- '%c'", arg[j]);
        result = 0;
        break;
    }
  }

  return result;
}

int parse_arguments(int argc, char *argv[], grep_flags *flags, char *files[],
                    error_handler_t *error_handler) {
  int file_count = 0;
  int i = 1;

  while (i < argc && !error_handler->has_error) {
    if (argv[i][0] == '-') {
      if (strcmp(argv[i], "-e") == 0) {
        if (!handle_ef_flags(argc, argv, &i, flags, error_handler)) {
          break;
        }
      } else if (strcmp(argv[i], "-f") == 0) {
        if (!handle_ef_flags(argc, argv, &i, flags, error_handler)) {
          break;
        }
      } else {
        if (!parse_short_flags(argv[i], flags, argc, argv, &i, error_handler)) {
          break;
        }
      }
    } else {
      if (flags->pattern_count == 0 && !flags->file_patterns) {
        flags->patterns[flags->pattern_count++] = argv[i];
      } else if (file_count < MAX_FILES) {
        files[file_count++] = argv[i];
      }
    }
    i++;
  }

  if (!error_handler->has_error) {
    validate_flags(flags, error_handler);
  }

  if (file_count > 1) {
    flags->multiple_files = 1;
  }

  return error_handler->has_error ? -1 : file_count;
}

void load_patterns_from_file(grep_flags *flags, const char *filename,
                             error_handler_t *error_handler) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    if (!flags->suppress_errors) {
      set_error(error_handler, "%s: No such file or directory", filename);
    }
    return;
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, file)) != -1 &&
         flags->pattern_count < MAX_PATTERNS) {
    if (line[read - 1] == '\n') {
      line[read - 1] = '\0';
    }
    if (strlen(line) > 0) {
      flags->patterns[flags->pattern_count++] = strdup(line);
    }
  }

  free(line);
  fclose(file);
}

void compile_regex(regex_t *regex, const char *pattern, int ignore_case,
                   error_handler_t *error_handler) {
  int flags = REG_EXTENDED;
  if (ignore_case) flags |= REG_ICASE;

  int ret = regcomp(regex, pattern, flags);
  if (ret != 0) {
    char error_buf[100];
    regerror(ret, regex, error_buf, sizeof(error_buf));
    set_error(error_handler, "invalid regular expression: %s", error_buf);
  }
}

void print_match(const char *filename, int line_num, const char *line,
                 const grep_flags *flags) {
  int should_print = 1;

  if (flags->files_with_matches || flags->count) {
    should_print = 0;
  }

  if (should_print) {
    if (flags->multiple_files && !flags->no_filename) {
      printf("%s:", filename);
    }

    if (flags->line_number) {
      printf("%d:", line_num);
    }

    printf("%s", line);
    if (line[strlen(line) - 1] != '\n') {
      printf("\n");
    }
  }
}

void print_count(const char *filename, int count, const grep_flags *flags) {
  if (flags->multiple_files && !flags->no_filename) {
    printf("%s:", filename);
  }
  printf("%d\n", count);
}

void print_only_matching(const char *filename, int line_num, const char *line,
                         regex_t *regexes, int pattern_count,
                         const grep_flags *flags) {
  if (flags->files_with_matches) {
    return;
  }

  regmatch_t match;
  const char *current_line = line;

  while (1) {
    int found = 0;
    for (int p = 0; p < pattern_count && !found; p++) {
      if (regexec(&regexes[p], current_line, 1, &match, 0) == 0 &&
          match.rm_so != -1) {
        found = 1;

        if (flags->multiple_files && !flags->no_filename) {
          printf("%s:", filename);
        }

        if (flags->line_number) {
          printf("%d:", line_num);
        }

        int match_length = match.rm_eo - match.rm_so;
        printf("%.*s\n", match_length, current_line + match.rm_so);

        current_line += match.rm_eo;

        if (*current_line == '\0' || *current_line == '\n') {
          break;
        }
      }
    }
    if (!found) break;
  }
}

void process_single_file(FILE *file, const char *filename, regex_t *regexes,
                         int pattern_count, const grep_flags *flags) {
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int line_num = 0;
  int match_count = 0;
  int file_has_match = 0;

  while ((read = getline(&line, &len, file)) != -1) {
    line_num++;

    int match = 0;
    for (int p = 0; p < pattern_count && !match; p++) {
      if (regexec(&regexes[p], line, 0, NULL, 0) == 0) {
        match = 1;
      }
    }

    if (flags->invert_match) {
      match = !match;
    }

    if (match) {
      match_count++;
      file_has_match = 1;

      if (flags->files_with_matches) {
        break;
      }

      if (flags->only_matching && !flags->invert_match) {
        print_only_matching(filename, line_num, line, regexes, pattern_count,
                            flags);
      } else if (!flags->count && !flags->files_with_matches) {
        print_match(filename, line_num, line, flags);
      }
    }
  }

  if (flags->files_with_matches && file_has_match) {
    printf("%s\n", filename);
  }

  if (flags->count && !flags->files_with_matches) {
    print_count(filename, match_count, flags);
  }

  free(line);
}

void process_files(grep_flags *flags, char *files[], int file_count) {
  regex_t regexes[MAX_PATTERNS];
  error_handler_t error_handler;
  init_error_handler(&error_handler);

  for (int p = 0; p < flags->pattern_count && !error_handler.has_error; p++) {
    compile_regex(&regexes[p], flags->patterns[p], flags->ignore_case,
                  &error_handler);
  }

  if (error_handler.has_error) {
    fprintf(stderr, "s21_grep: %s\n", error_handler.error_message);
    return;
  }

  if (file_count == 0) {
    process_single_file(stdin, "(standard input)", regexes,
                        flags->pattern_count, flags);
  } else {
    for (int i = 0; i < file_count; i++) {
      FILE *file = fopen(files[i], "r");
      if (file) {
        process_single_file(file, files[i], regexes, flags->pattern_count,
                            flags);
        fclose(file);
      } else if (!flags->suppress_errors) {
        fprintf(stderr, "s21_grep: %s: No such file or directory\n", files[i]);
      }
    }
  }

  for (int p = 0; p < flags->pattern_count; p++) {
    regfree(&regexes[p]);
    if (flags->file_patterns && flags->patterns[p]) {
      free(flags->patterns[p]);
    }
  }
}

int main(int argc, char *argv[]) {
  int exit_code = 0;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s [OPTIONS] PATTERN [FILE...]\n", argv[0]);
    exit_code = 1;
  } else {
    grep_flags flags;
    error_handler_t error_handler;
    char *files[MAX_FILES];

    init_flags(&flags);
    init_error_handler(&error_handler);

    int file_count = parse_arguments(argc, argv, &flags, files, &error_handler);

    if (error_handler.has_error) {
      fprintf(stderr, "s21_grep: %s\n", error_handler.error_message);
      fprintf(stderr, "Usage: %s [OPTIONS] PATTERN [FILE...]\n", argv[0]);
      exit_code = 1;
    } else {
      process_files(&flags, files, file_count);
      exit_code = 0;
    }
  }

  return exit_code;
}