#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

S21_CAT="../src/cat/s21_cat"
SYSTEM_CAT="cat"

SINGLE_FLAGS=("-n" "-b" "-e" "-E" "-s" "-t" "-T" "-v")
GNU_FLAGS=("--number" "--number-nonblank" "--squeeze-blank")

success=0
fail=0
total=0

create_test_files() {
    echo "=== Создание тестовых файлов ==="
    
    cat > all_flags_test.txt << 'EOF'
    First normal line
Second line with text

Empty line above
Line with	tabs	here
    Line with leading spaces
Line with trailing spaces:     
Mixed    spaces	and	tabs

Multiple empty lines above


Back to normal after empties
Line with special chars: 	
Line with DEL:  here
Last normal line
EOF

    touch empty.txt

    echo "Single line of text" > single_line.txt

    echo "Созданые тестовые файлы: all_flags_test.txt, empty.txt, single_line.txt"
    echo "========================================"
    echo ""
}

compare_outputs() {
    local flags="$1"
    local file="$2"
    local test_num="$3"

    echo -e "${YELLOW}Тест $test_num: cat $flags $file"

    $SYSTEM_CAT $flags "$file" > system_output.txt 2>/dev/null

    $S21_CAT $flags "$file" > s21_output.txt 2>/dev/null

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
    echo "========================================"
}

run_tests() {

    echo "=== Тестирование одиночных флагов cat ==="
    local test_counter=1

    for flag in "${SINGLE_FLAGS[@]}"; do
        for file in "all_flags_test.txt" "empty.txt" "single_line.txt"; do
            if [ -f "$file" ]; then
                compare_outputs "$flag" "$file" "$test_counter"
                ((test_counter++))
            fi
        done
    done
    echo ""

    echo "=== Тестирование без флагов ==="
    for file in "all_flags_test.txt" "empty.txt" "single_line.txt"; do
        if [ -f "$file" ]; then
            compare_outputs "" "$file" "$test_counter"
            ((test_counter++))
        fi
    done
    echo ""

    echo "=== Тестирование нескольких файлов ==="
    compare_outputs "-n" "all_flags_test.txt single_line.txt" "$test_counter"
    ((test_counter++))
    compare_outputs "-b" "all_flags_test.txt empty.txt single_line.txt" "$test_counter"
    ((test_counter++))
    compare_outputs "-e" "all_flags_test.txt empty.txt single_line.txt" "$test_counter"
    ((test_counter++))
    echo ""

    echo "=== Тестирование GNU флагов ==="
    for flag in "${GNU_FLAGS[@]}"; do
        for file in "all_flags_test.txt" "empty.txt" "single_line.txt"; do
            if [ -f "$file" ]; then
                compare_outputs "$flag" "$file" "$test_counter"
                ((test_counter++))
            fi
        done
    done
    echo ""
}

cleanup() {
    rm -f system_output.txt s21_output.txt
    rm -f all_flags_test.txt empty.txt single_line.txt special_chars.txt 2>/dev/null
    echo "Временные файлы удалены"
}

main() {

    if ! command -v "$SYSTEM_CAT" &> /dev/null; then
        echo -e "${RED}Ошибка: Системная утилита cat не найдена${NC}"
        exit 1
    fi

    if [ ! -f "$S21_CAT" ]; then
        echo -e "${RED}Ошибка: Утилита $S21_CAT не найдена"
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