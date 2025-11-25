#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

S21_GREP="../src/grep/s21_grep"
SYSTEM_GREP="grep"

SINGLE_FLAGS=("-i" "-v" "-c" "-l" "-n" "-h" "-s" "-o")
PATTERNS=("test" "hello" "pattern" "line")
TEST_FILES=("test1.txt" "test2.txt" "test3.txt")

success=0
fail=0
total=0

create_test_files() {
    echo "=== Создание тестовых файлов для grep ==="

    cat > test1.txt << 'EOF'
test line 1
TEST line 2
another line
test pattern here
hello world
line with TEST in middle
EOF

    cat > test2.txt << 'EOF'
second file
test in second file
HELLO there
no match here
another test line
EOF

    cat > test3.txt << 'EOF'
third file content
TEST UPPERCASE
test lowercase
Mixed Test Case
last line
EOF

    echo "Тестовые файлы созданы: test1.txt, test2.txt, test3.txt"
    echo "========================================"
    echo ""
}

compare_outputs() {
    local flags="$1"
    local pattern="$2"
    local files="$3"
    local test_num="$4"

    echo -e "${YELLOW}Тест $test_num: grep $flags '$pattern' $files${NC}"

    $SYSTEM_GREP $flags $pattern $files > "system_output.txt" 2>/dev/null

    $S21_GREP $flags $pattern $files > "s21_output.txt" 2>/dev/null

    if diff -q system_output.txt s21_output.txt > /dev/null; then
        echo -e "${GREEN}✓ УСПЕХ${NC}"
        ((success++))
    else
        echo -e "${RED}✗ ОШИБКА${NC}"
        echo "Различия:"
        diff -u system_output.txt s21_output.txt | head -10
        ((fail++))
    fi
    ((total++))
    echo "---"
}

run_tests() {
    echo "=== Запуск тестирования grep ==="
    echo ""
    local test_counter=1

    echo "=== Базовый поиск grep ==="
    for pattern in "${PATTERNS[@]}"; do
        echo "Паттерн ${pattern}"
        for file in "${TEST_FILES[@]}"; do
            compare_outputs "" "$pattern" "$file" "$test_counter"
            ((test_counter++))
        done
    done
    echo ""

    echo "=== Одиночные флаги grep ==="
    for flag in "${SINGLE_FLAGS[@]}"; do
        echo "Тестирование флага ${flag}"
        for pattern in "${PATTERNS[@]}"; do
            compare_outputs "$flag" "$pattern" "test1.txt" "test2.txt" "$test_counter"
            ((test_counter++))
        done
    done
    echo ""

    echo "=== Комбинация флагов grep ==="
    compare_outputs "-in" "test" "test1.txt test2.txt" "$test_counter"
    ((test_counter++))
    compare_outputs "-vc" "test" "test1.txt" "$test_counter"
    ((test_counter++))
    compare_outputs "-lh" "hello" "test1.txt test2.txt" "$test_counter"
    ((test_counter++))
    echo ""

    echo "=== Тестирование нескольких файлов ==="
    compare_outputs "-n" "test" "test1.txt test2.txt test3.txt" "$test_counter"
    ((test_counter++))
    echo ""
}

cleanup() {
    rm -f system_output.txt s21_output.txt
    rm -f test1.txt test2.txt test3.txt 2>/dev/null
    echo "Временные файлы удалены"
}

main() {
    if ! command -v "$SYSTEM_GREP" &> /dev/null; then
        echo -e "${RED}Ошибка: Системная утилита grep не найдена${NC}"
        exit 1
    fi

    if [ ! -f "$S21_GREP" ]; then
        echo -e "${RED}Ошибка: Утилита $S21_GREP не найдена"
        echo "Соберите утилиту с помощью make перед запуском тестов"
        exit 1
    fi

    create_test_files
    run_tests
    cleanup

    echo "=========================="
    echo -e "${GREEN}УСПЕШНЫХ ТЕСТОВ: $success${NC}"
    echo -e "${RED}НЕУДАЧНЫХ ТЕСТОВ: $fail${NC}"
    echo -e "ВСЕГО ТЕСТОВ: $total"
    echo "=========================="

}

main