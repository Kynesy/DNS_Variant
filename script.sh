#!/bin/bash
export LC_NUMERIC="C"

MAX_RUNS=50
M_FLAG=0
C_FLAG=0

usage() {
    local script_name=$1
    echo 
    echo "NAME"
    echo "    $script_name - Utility script for measuring the performance and correctness check."
    echo
    echo "SYNOPSIS"
    echo "    $script_name [-b] [-d] [-n SIZE -m | -n SIZE -c -p PROC]"
    echo
    echo "DESCRIPTION"
    echo "    $script_name can be used to build the project, clean the project, measure the performance of the dnsVariant algorithm,"
    echo "    and check the correctness."
    echo "    Options -b and -d must be used alone."
    echo "    Options -n and -m or -n, -c and -p must be used together."
    echo
    echo "OPTIONS"
    echo "    -b"
    echo "        Build the project. It must be used alone."
    echo
    echo "    -d"
    echo "        Clean the compiled files, input files and output files. It must be used alone."
    echo
    echo "    -n SIZE"
    echo "        Specifies the size of the matrix. This argument is mandatory when using the -m or -c option."
    echo
    echo "    -m"
    echo "        Runs the algorithm MAX_RUNS times and saves the measurements in a CSV file inside the measurements folder."
    echo "        It requires the -n option. The -c option is not allowed."
    echo "        The CSV file will contain the following columns: Processor number, Input Time, Computation Time."
    echo
    echo "    -c"
    echo "        It check the correctness of the dnsVariant algorithm. It requires the -n and -p options."
    echo "        The -m option is not allowed."
    echo "        It requires the existence of matrixA.bin and matrixB.bin files. Those can be generate with"
    echo "        the command ./generateMatrix [SIZE]."
    echo
    echo "    -p PROC"
    echo "        Specifies the number of processes. It must be a multiple of SIZE^2, comprised between SIZE^2 and SIZE^3."
    echo "        This argument is mandatory when using the -c option."
    echo
    echo "EXAMPLES"
    echo
    echo "        $script_name -b"
    echo "            Build the project."
    echo
    echo "        $script_name -d"
    echo "            Purge the project."
    echo
    echo "        $script_name -n 6 -m"
    echo "            Measure the performance of the dnsVariant algorithm with matrix size 6."
    echo
    echo "        $script_name -n 6 -c -p 72"
    echo "            Check the correctness of the dnsVariant algorithm with matrix size 6 and 72 processes."
    echo
    echo "AUTHOR"
    echo "    Cezar Narcis Culcea"
    exit 1
}

build(){
    make clean
    make all
    exit 0
}

clean(){
    make clean
    rm -f matrix*
    exit 0
}

generateInput(){
    echo "Matrix SIZE: ${SIZE}*${SIZE}"
    make all > /dev/null
    ./generateMatrix $SIZE
}

measure(){
    timestamp=$(date +%d%m%Y_%H%M)
    csv_file="./measurements/SIZE${SIZE}_${timestamp}.csv"
    echo "Measurements matrix SIZE $SIZE * $SIZE" >> $csv_file
    echo "Proc;Input Time;Calc Time" >> $csv_file

    #################### Sequential Measurements ###################
    echo "-------------------------------------------------------------------------------------------------------"
    echo "Sequential Algorithm Measurements (Avg. on $MAX_RUNS runs)"
    inputTime_seq=0
    calcTime_seq=0

    for i in $(seq 1 $MAX_RUNS); do
        output=$(./seqMatrixMultiply $SIZE)
        time1=$(echo $output | awk '{print $1}')
        time2=$(echo $output | awk '{print $2}')

        inputTime_seq=$(echo "$inputTime_seq + $time1" | bc -l)
        calcTime_seq=$(echo "$calcTime_seq + $time2" | bc -l)

    done

    inputTime_seq=$(echo "$inputTime_seq / $MAX_RUNS" | bc -l)
    calcTime_seq=$(echo "$calcTime_seq / $MAX_RUNS" | bc -l)
    printf "\tInput Time: %10.6f  -  Calc Time: %10.6f\n" $inputTime_seq $calcTime_seq

    printf "%d;%10.6f;%10.6f\n" 1 $inputTime_seq $calcTime_seq >> $csv_file

    #################### Parallel Measurements ###################  
    echo "-------------------------------------------------------------------------------------------------------"
    echo "Parallel Algorithm Measurements (Avg. on $MAX_RUNS runs)"

    for i in $(seq 1 $SIZE); do
        if (( SIZE % i == 0 )); then
            proc=$((SIZE*SIZE*i))
            inputTime_par=0
            calTime_par=0

            printf "\tProcs number: %d = %d*%d*%d\t--> " $proc $SIZE $SIZE $i 

            for j in $(seq 1 $MAX_RUNS); do
                output=$(mpirun --oversubscribe -n $proc ./dnsVariant $SIZE)
                time1=$(echo $output | awk '{print $1}')
                time2=$(echo $output | awk '{print $2}')

                inputTime_par=$(echo "$inputTime_par + $time1" | bc -l)
                calTime_par=$(echo "$calTime_par + $time2" | bc -l)

            done

            inputTime_par=$(echo "$inputTime_par / $MAX_RUNS" | bc -l)
            calTime_par=$(echo "$calTime_par / $MAX_RUNS" | bc -l)
            printf "Input Time: %10.6f  -  Calc Time: %10.6f\n" $inputTime_par $calTime_par

            printf "%d;%10.6f;%10.6f\n" $proc $inputTime_par $calTime_par >> $csv_file
        fi
    done

    exit 0
}

check() {
    if [ ! -e "matrixA.bin" ] || [ ! -e "matrixB.bin" ]; then
        echo "Error: matrixA.bin or matrixB.bin does not exist. Please provide input matrices."
        exit 1
    fi

    if (( ($PROC % ($SIZE * $SIZE) != 0) || ($PROC < ($SIZE * $SIZE)) ||  ($PROC > ($SIZE * $SIZE * $SIZE)) )); then
        echo "Error: Number of processes must be a multiple of $SIZE^2, comprised between $SIZE^2 and $SIZE^3."
        exit 1
    fi

    echo "-------------------------------------------------------------------------------------------------------"
    printf "Correctness check (Matrix size: %d * %d)\n" $SIZE $SIZE

    printf "\t Sequential computation ... "
    ./seqMatrixMultiply $SIZE > /dev/null
    printf "done\n"

    printf "\t Parallel computation ... "   
    mpirun --oversubscribe -n $PROC ./dnsVariant $SIZE > /dev/null
    printf "done\n"

    digestSequential=$(md5sum matrixC_sequential.bin | awk '{print $1}')
    digestParallel=$(md5sum matrixC_dnsVariant.bin | awk '{print $1}')

    if [ "$digestSequential" = "$digestParallel" ]; then
        echo -e '\033[1mMatrices C are equal\033[0m'
    else
        echo -e '\033[1mMatrices C are not equal\033[0m'
    fi

    printf "Results saved in matrixC_sequential.bin and matrixC_dnsVariant.bin\n"

    exit 0
}

while getopts ":bdn:mcp:" opt; do
    case ${opt} in
        b )
            build
            ;;
        d )
            clean
            ;;
        n )
            SIZE=$OPTARG
            ;;
        p )
            PROC=$OPTARG
            ;;
        m )
            M_FLAG=1
            ;;
        c )
            C_FLAG=1
            ;;
        :) printf "Missing argument for -%s\n" "$OPTARG" >&2; usage
            ;;
        \?) printf "Illegal option: -%s\n" "$OPTARG" >&2; usage
            ;;
    esac
done
shift $((OPTIND -1))

# Check if options are valid
if [[ -z $SIZE ]]; then
    echo "Error: -n SIZE is required"
    usage "$0"
fi

# Check if -m and -c are used together
if [[ $M_FLAG -eq $C_FLAG ]]; then
    echo "Error: One of the options -m or -c must be used"
    usage "$0"
fi

# Check if -n is used with -c or -m
if [[ -n $SIZE && ($M_FLAG -eq 1) ]]; then
    generateInput
    measure
fi


if [[ -n $SIZE && ($C_FLAG -eq 1) ]]; then
    if [[ -z $PROC ]]; then
        echo "Error: -p PROC is required"
        usage "$0"
    fi
    check
fi

