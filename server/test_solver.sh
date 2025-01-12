#!/bin/bash

PROGRAM="solver.c"
EXECUTABLE="solver"

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
# Integer numbers count:
["5+5"]=10.000000
["80-2"]=78.000000
["2*30"]=60.000000
["2/100"]=0.020000
["12+2+10"]=24.000000
["9-10-5"]=-6.000000
["1*20*3"]=60.000000
["60/2/3"]=10.000000
["1/10"]=0.100000
["0/2"]=0.000000
# Fractional numbers count:
["23.02-1.3+22.2222"]=43.942200  
["34.22*22*38.000001"]=28607.920753  
["34.9990001+222.22332"]=257.222320  
["3.34/3.02-22.22"]=-21.114040  
["99-33.22/1.0"]=65.780000  
["1 / 2.0"]=0.500000  
["1.0/2.0"]=0.500000  
["1.0/2"]=0.500000  
["10.2 / 2.009"]=5.077153  
["0.00000/10.1"]=0.000000
["20.2/(3- 3.00001)"]=-2019999.999987  
["2.1 - 2.1 + 0.2"]=0.200000  
["(8.0+2.0) / 2.0"]=5.000000
# Division by zero error:
["1/0"]="WA"  
["2.1/0"]="WA"  
["100.002 / 0"]="WA"  
["20.1/(2-2)"]="WA"  
["20/(4-4)"]="WA"  
["20/(4.0-4)"]="WA"  
["20/(4-4.0)"]="WA"  
["(20.1-3.4)/0.0"]="WA"
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
