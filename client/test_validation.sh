#!/bin/bash

PROGRAM="validation.c"
EXECUTABLE="validation"

if [ ! -f "$PROGRAM" ]; then
    echo "Error: file $PROGRAM is not found."
    exit 1
fi

gcc -o $EXECUTABLE $PROGRAM
if [ $? -ne 0 ]; then
    echo "Compilation error."
    exit 1
fi
echo "Compilation successful."

declare -A TEST_CASES=(
# Brackets check (correct):
["2*3-1"]="OK"
["(2.21+3.0)*8.3"]="OK"
["(2+6)*2"]="OK"
["2-(8*2.9)"]="OK"
# Operations check (correct):
["8+2+3"]="OK"
["2-5-3"]="OK"
["2*3*4"]="OK"
["8/4/2"]="OK"
["3/2*7"]="OK"
["5-1+6"]="OK"
# Fractional numbers check (correct):
["3.2-4.3"]="OK"
["2.34/3.44"]="OK"
["2.0+5.1*8.9"]="OK"
["3.01+2.45+6.89"]="OK"
["4.000+1.999+4.343"]="OK"
["2.12345/(3.93560-1.88992)"]="OK"
# Wrong brackets positions (incorrect):
["(2.0+2.0"]="WA"  
[")2.0-2.0("]="WA"  
[")2.001-9"]="WA"  
["2.0+2)"]="WA"  
["(2.3/2.0("]="WA"  
[")2.3/2.3)"]="WA"
["(2.2+2.2+2.2)"]="WA"  
["2.2+2.2-2.2)"]="WA"  
["(2.2+1.1+3.3"]="WA"
["3.3+(3.3-3.3"]="WA"  
["3+)3.3-3.3)"]="WA"  
["3.0-((2.2+2.2)"]="WA"  
["3.3+(2.3+9))"]="WA"  
["(3.3+3.3))+2"]="WA"  
["((3.4+2.3)+2"]="WA"  
["(3.3+3.3)"]="WA"  
["(3+3)"]="WA"
["(4+4+4+4)"]="WA"
# Wrong acceptable-symbols positions (incorrect):
["2--2"]="WA"  
["2..0-2"]="WA"  
[".2-0"]="WA"
["5.2.+2"]="WA"  
["5.2.4+3.3"]="WA"
["3.3--3.3"]="WA"  
["3++3"]="WA"  
["8+3..3"]="WA"  
["8+8.9+3..3"]="WA"  
["0.3//2"]="WA"  
["2**8"]="WA"  
["(3+3.3)--0"]="WA"  
["8++(3.3-2.2)"]="WA"  
["8.2+(2.2)"]="WA"  
["2.3+.0"]="WA"  
["4.444444-4."]="WA"  
["1-2++2"]="WA"  
["1++2-3"]="WA"  
["1..9+2.9"]="WA"  
["1.2+2..2"]="WA"  
["9.0+2.2+3..3"]="WA"  
["(8-2)++1"]="WA"  
["(3.3+3.3).-2.0"]="WA"  
["(8.0.+2.0)-1.1"]="WA"  
["(22-2)+2+"]="WA"  
["+2.2+2.3"]="WA"  
["2.2.2"]="WA"  
["2.2.2+2.2.2"]="WA"
# Wrong amount of numbers (incorrect):
["4+4+4+4"]="WA"   
["4.0+(3.3+3.3+3.3)"]="WA"  
["(3.9+3.3-2.0)-3.3"]="WA" 
["1+2+3+4+5"]="WA"
["2.0"]="WA"
["2"]="WA"
# Using of non acceptable-symbols (incorrect):
["3.p+2.0"]="WA"  
["3.4+2.u"]="WA"  
["3.4+3.4+3.p"]="WA"  
["3.4+3.4+3.3p"]="WA"    
["5.5-2.2o2.2"]="WA"  
["5-2-o"]="WA"  
["o3.3+3.3o"]="WA"  
# Exceeding the limit of math task:
["1111111111111111111111111111111"]="WA"  # "1"*31
["1111111111111111111111111+       1"]="OK"  # length<=31 (no spaces)
["1111111111111111111+11111111111"]="OK"  # length==31
["11111111111111111111+11111111111"]="WA"  # length==32
)

run_test() {
    local input=$1
    local expected=$2

    result=$(echo "$input" | ./$EXECUTABLE)
    if [ "$result" == "$expected" ]; then
        echo "Correct. Test '$input' (Expected: $expected, Result: $result)"
    else
        echo "Incorrect. Test '$input' (Expected: $expected, Result: $result)"
    fi
}

echo "Starting tests..."
for input in "${!TEST_CASES[@]}"; do
    expected=${TEST_CASES[$input]}
    run_test "$input" "$expected"
done
