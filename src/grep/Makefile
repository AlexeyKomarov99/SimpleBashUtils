CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror
TARGET = s21_grep
SOURCES = s21_grep.c
OBJECTS = $(SOURCES:.c=.o)
COMMON_DIR = ../common
COMMON_OBJECT = $(COMMON_DIR)/s21_common.o
CPPCHECK_GREP_FLAGS = --enable=all --suppress=missingIncludeSystem --suppress=checkersReport --suppress=normalCheckLevelMaxBranches --suppress=unusedStructMember
CPPCHECK_COMMON_FLAGS = --enable=all --suppress=missingIncludeSystem --suppress=checkersReport --suppress=unusedFunction --suppress=unusedStructMember
CLANG_FORMAT = clang-format
CLANG_FORMAT_STYLE = ../../materials/linters/.clang-format

all: $(TARGET)

$(TARGET): $(OBJECTS) $(COMMON_OBJECT)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(COMMON_OBJECT):
	cd $(COMMON_DIR) && $(MAKE)

sanitize: CFLAGS += -fsanitize=address -g
sanitize: $(OBJECTS) $(COMMON_OBJECT)
	$(CC) $(CFLAGS) -o $(TARGET)_sanitize $^
	@echo "Sanitizer build complete: ./$(TARGET)_sanitize"

cppcheck:
	cppcheck $(CPPCHECK_GREP_FLAGS) $(SOURCES) *.h
	cd $(COMMON_DIR) && cppcheck $(CPPCHECK_COMMON_FLAGS) $(COMMON_DIR)/*.c $(COMMON_DIR)/*.h

style:
	$(CLANG_FORMAT) -n -style=file:$(CLANG_FORMAT_STYLE) $(SOURCES) *.h
	cd $(COMMON_DIR) && $(MAKE) style

format:
	$(CLANG_FORMAT) -i -style=file:$(CLANG_FORMAT_STYLE) $(SOURCES) *.h
	cd $(COMMON_DIR) && $(MAKE) format

check: style cppcheck

build: check $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET)_sanitize
	cd $(COMMON_DIR) && $(MAKE) clean

.PHONY: all clean cppcheck style format check build sanitize