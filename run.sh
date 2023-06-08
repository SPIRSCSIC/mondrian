#!/usr/bin/env bash

dataset=${DATASET:-adults}

build () {
    echo "Building Mondrian"
    rm -rf build
    mkdir build
    cd build && cmake .. && make && cd - > /dev/null
}

run () {
    ./build/mondrian -f datasets/${dataset}.csv -o ${dataset}_anonymized.csv \
        && echo -e "\nFile ${dataset}_anonymized.csv generated in current directory"
}

test_k () {
    local lower
    local upper
    local step
    if [ ! -z "$1" ]; then
        lower=$1
    else
        lower=5
    fi
    if [ ! -z "$2" ]; then
        upper=$2
    else
        upper=105
    fi
    if [ ! -z "$3" ]; then
        step=$3
    else
        step=5
    fi
    echo "Running tests with variable K"
    for k in $(seq $lower $step $upper); do
        ./build/mondrian -f datasets/${dataset}.csv -k $k -r
        echo
    done
}

test_qi () {
    cp datasets/${dataset}.csv datasets/${dataset}.qi.csv
    local qi="$(sed -n '2p' datasets/${dataset}.qi.csv)"
    OLDIFS=$IFS
    IFS=', ' read -ra parts <<< "$qi"
    IFS=$OLDIFS
    local output="${parts[0]}"
    echo "Running tests with variable QIDs length"
    for ((i=0; i<${#parts[@]}; i++)); do
        if [ $i -eq 0 ]; then
            output=${output}
        else
            output="${output},${parts[i]}"
        fi
        sed -i "2s/.*/$output/" datasets/${dataset}.qi.csv
        echo "QIDs: $(( i + 1 ))"
        ./build/mondrian -f datasets/${dataset}.qi.csv -r
        echo
    done
    rm datasets/${dataset}.qi.csv
}

test_data () {
    local size="$(wc datasets/${dataset}.data.csv)"
    local batch
    local num_test
    if [ ! -z "$1" ]; then
        batch=$1
    else
        batch=5000
    fi
    if [ ! -z "$2" ]; then
        num_test=$2
    else
        num_test=6
    fi
    echo "Running tests with variable dataset size"
    for ((i=1; i<=$num_test; i++)); do
        if [ $i -eq 1 ]; then
            sed -n "1,$(( i * batch )) p" datasets/${dataset}.csv > datasets/${dataset}.data.csv
        else
            sed -n "$(( ( ($i - 1) * batch ) + 1 )),$(( i * batch )) p" datasets/${dataset}.csv >> datasets/${dataset}.data.csv
        fi
        echo "Dataset size: $(wc -l < datasets/${dataset}.data.csv)"
        ./build/mondrian -f datasets/${dataset}.data.csv -r
        echo
    done
    rm datasets/${dataset}.data.csv
}

if [ "$1" = "build" ]; then
    build
elif [ "$1" = "run" ]; then
    run
elif [ "$1" = "test" ]; then
    if [ ! -z "$2" ]; then
        if [ "$2" = "k" ]; then
            test_k $3 $4 $5
        elif [ "$2" = "qi" ]; then
            test_qi
        elif [ "$2" = "data" ]; then
            test_data $3 $4
        fi
    else
        test_k
        test_qi
        test_data
    fi
elif [ "$1" = "-h" ]; then
    echo "Usage: ./run.sh CMD ARGS"
    echo
    echo "Optional commands:"
    echo "  build: build the project"
    echo "  run: run the project"
    echo "  test: run some tests"
    echo "  -h: show this help"
    echo
    echo "Optional arguments:"
    echo "  (run) -r: force build the project"
    echo "  (test) k [lower [upper [step]]]: test with variable K. Default: lower=5, upper=105, step=5"
    echo "  (test) qi: test with variable QIDs length"
    echo "  (test) data [batch [num_test]]: test with variable dataset size. Default: batch=5000, num_test=6"
    echo
    echo "Environment variables:"
    echo "  DATASET: dataset name. Default: adults. Example: DATASET=informs ./run.sh"
else
    if [ ! -d "build" ] || [ "$1" = "-r" ]; then
        build
        echo
    fi
    run
fi
