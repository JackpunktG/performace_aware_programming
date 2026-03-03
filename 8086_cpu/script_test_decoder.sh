#!/bin/bash

PASS=0
FAIL=0

run_test() {
    local ASM_FILE=$1

    if [ ! -f "$ASM_FILE" ]; then
        echo "Error: '$ASM_FILE' not found"
        exit 1
    fi

    BASE=$(basename "$ASM_FILE" .asm)
    DIR=$(dirname "$ASM_FILE")

    ORIGINAL="$DIR/${BASE}_original.bin"
    DISASSEMBLED="$DIR/${BASE}_disassembled.asm"
    REASSEMBLED="$DIR/${BASE}_reassembled.bin"

    echo "Testing: $ASM_FILE"

    # assemble original
    nasm -f bin "$ASM_FILE" -o "$ORIGINAL"

    # run through your disassembler
    ./out "$ORIGINAL" > "$DISASSEMBLED"

    # reassemble the disassembled output
    nasm -f bin "$DISASSEMBLED" -o "$REASSEMBLED"

    # compare the two binaries
    diff "$ORIGINAL" "$REASSEMBLED" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "  PASS - $BASE binaries are identical"
        PASS=$((PASS + 1))
    else
        echo "  FAIL - $BASE binaries differ"
        FAIL=$((FAIL + 1))
    fi
     # cleanup
    rm -f "$ORIGINAL" "$REASSEMBLED" "$DISASSEMBLED"
}

if [ $# -eq 0 ]; then
    # no arg - default to tests folder
    if [ ! -d "tests" ]; then
        echo "Error: no argument given and 'tests' folder not found"
        exit 1
    fi

    for ASM_FILE in tests/*.asm; do
        if [ ! -f "$ASM_FILE" ]; then
            echo "No .asm files found in 'tests'"
            exit 1
        fi
        run_test "$ASM_FILE"
    done

else
    # arg given - test only that file
    run_test "$1"
fi

echo ""
echo "Results: $PASS passed, $FAIL failed"
