#include "s21_common.h"

#include <stdarg.h>
#include <sys/stat.h>

void init_error_handler(error_handler_t *handler) {
  handler->has_error = 0;
  handler->error_message[0] = '\0';
}

void set_error(error_handler_t *handler, const char *format, ...) {
  handler->has_error = 1;
  va_list args;
  va_start(args, format);
  vsnprintf(handler->error_message, sizeof(handler->error_message), format,
            args);
  va_end(args);
}

void clear_error(error_handler_t *handler) {
  handler->has_error = 0;
  handler->error_message[0] = '\0';
}

int is_file_exists(const char *filename) {
  struct stat buffer;
  return (stat(filename, &buffer) == 0);
}

void safe_string_copy(char *dest, const char *src, size_t dest_size) {
  if (dest_size > 0) {
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
  }
}