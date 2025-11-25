CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror
SOURCES = s21_common.c
OBJECTS = $(SOURCES:.c=.o)
CPPCHECK_FLAGS = --enable=all --suppress=missingIncludeSystem --suppress=checkersReport --suppress=unusedFunction --suppress=unusedStructMember
CLANG_FORMAT = clang-format
CLANG_FORMAT_STYLE = ../../materials/linters/.clang-format

all: $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

cppcheck:
	cppcheck $(CPPCHECK_FLAGS) $(SOURCES) *.h

style:
	$(CLANG_FORMAT) -n -style=file:$(CLANG_FORMAT_STYLE) $(SOURCES) *.h

format:
	$(CLANG_FORMAT) -i -style=file:$(CLANG_FORMAT_STYLE) $(SOURCES) *.h

check: style cppcheck

clean:
	rm -f $(OBJECTS)

.PHONY: all clean cppcheck style format check