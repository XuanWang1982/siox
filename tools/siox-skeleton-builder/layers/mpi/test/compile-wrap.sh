#!/bin/sh

OUTPUT_FILE=mpi-wrapper.c
alias sioxmpicc="mpicc -L/home/siox-devel/install-xuan/siox/lib64 -lsiox-monitoring-siox-ll -lboost_regex -L/home/siox-devel/install-tools/boost_1_53_0/lib -lboost_system -lboost_serialization -lboost_thread -lboost_program_options -L/home/siox-devel/install-tools/gcc-4.7.3/lib64 -L/homesiox-devel/install-tools/gcc-4.7.3/lib -lstdc++"

alias sioxmpirun="SIOX_CONFIGURATION_PROVIDER_ENTRY_POINT=/home/sioxaagu/siox/src/monitoring/low-level-c/test/siox.conf mpirun --mca io ompio"

echo 'Generating' $OUTPUT_FILE
sleep 2
python ../../../siox-skeleton-builder.py -s wrap -t ../../../template.py -o $OUTPUT_FILE ../mpi-wrapper.h > wrap-options.txt

echo 'Compiling' $OUTPUT_FILE
sleep 2
sioxmpicc $OUTPUT_FILE -c -fPIC -std=c99 -I ../../../low-level-C-interface `pkg-config --cflags glib-2.0`
sioxmpicc write_gpfs_file.c -c

echo 'Appending the output of the wrapper to compile the program'
sleep 2
sioxmpicc write_gpfs_file.o mpi-wrapper.o -o write_gpfs_file_wrapper

echo 'Running wrapped code'
sioxmpirun -np 1 write_gpfs_file_wripper

#mpicc ${OUTPUT_FILE%%.c}.o -o libsiox-mpi-wrapper.so -shared -ldl `pkg-config --libs glib-2.0`