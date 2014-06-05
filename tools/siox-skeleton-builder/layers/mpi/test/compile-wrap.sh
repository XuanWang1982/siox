#!/bin/sh

OUTPUT_FILE=mpi-wrapper.c

WRAP_MPI_FUNCTIONS=-Wl,--wrap="MPI_Init",--wrap="MPI_Init_thread",--wrap="MPI_Finalize",--wrap="MPI_File_open",--wrap="MPI_File_close",--wrap="MPI_File_delete",--wrap="MPI_File_set_size",--wrap="MPI_File_preallocate",--wrap="MPI_File_get_size",--wrap="MPI_File_get_group",--wrap="MPI_File_get_amode",--wrap="MPI_File_set_info",--wrap="MPI_File_get_info",--wrap="MPI_File_set_view",--wrap="MPI_File_get_view",--wrap="MPI_File_read_at",--wrap="MPI_File_read_at_all",--wrap="MPI_File_write_at",--wrap="MPI_File_write_at_all",--wrap="MPI_File_iread_at",--wrap="MPI_File_iwrite_at",--wrap="MPI_File_read",--wrap="MPI_File_read_all",--wrap="MPI_File_write",--wrap="MPI_File_write_all",--wrap="MPI_File_iread",--wrap="MPI_File_iwrite",--wrap="MPI_File_seek",--wrap="MPI_File_get_position",--wrap="MPI_File_get_byte_offset",--wrap="MPI_File_read_shared",--wrap="MPI_File_write_shared",--wrap="MPI_File_iread_shared",--wrap="MPI_File_iwrite_shared",--wrap="MPI_File_read_ordered",--wrap="MPI_File_write_ordered",--wrap="MPI_File_seek_shared",--wrap="MPI_File_get_position_shared",--wrap="MPI_File_read_at_all_begin",--wrap="MPI_File_read_at_all_end",--wrap="MPI_File_write_at_all_begin",--wrap="MPI_File_write_at_all_end",--wrap="MPI_File_read_all_begin",--wrap="MPI_File_read_all_end",--wrap="MPI_File_write_all_begin",--wrap="MPI_File_write_all_end",--wrap="MPI_File_read_ordered_begin",--wrap="MPI_File_read_ordered_end",--wrap="MPI_File_write_ordered_begin",--wrap="MPI_File_write_ordered_end",--wrap="MPI_File_get_type_extent",--wrap="MPI_File_set_atomicity",--wrap="MPI_File_get_atomicity",--wrap="MPI_File_sync"

alias sioxmpicc="mpicc -L/home/siox-devel/install-xuan/siox/lib64 -lsiox-monitoring-siox-ll -lboost_regex -L/home/siox-devel/install-tools/boost_1_53_0/lib -lboost_system -lboost_serialization -lboost_thread -lboost_program_options -L/home/siox-devel/install-tools/gcc-4.7.3/lib64 -L/home/siox-devel/install-tools/gcc-4.7.3/lib -lstdc++"

alias sioxmpirun="SIOX_CONFIGURATION_PROVIDER_ENTRY_POINT=/home/sioxaagu/siox/src/monitoring/low-level-c/test/siox.conf mpirun --mca io ompio"

echo 'Generating' $OUTPUT_FILE
sleep 1
python ../../../siox-skeleton-builder.py -s wrap -t ../../../template.py -o $OUTPUT_FILE ../mpi-wrapper.h > wrap-options.txt

echo 'Compiling' $OUTPUT_FILE
sleep 1
sioxmpicc $OUTPUT_FILE -g -c -fPIC -std=c99 -I ../../../low-level-C-interface `pkg-config --cflags glib-2.0`
sioxmpicc write_gpfs_file.c -c

echo 'Appending the output of the wrapper to compile the program'
sleep 1
sioxmpicc write_gpfs_file.o mpi-wrapper.o -g -o write_gpfs_file_wrapper $WRAP_MPI_FUNCTIONS

echo 'Running wrapped code'
sioxmpirun -np 1 write_gpfs_file_wrapper

#mpicc ${OUTPUT_FILE%%.c}.o -o libsiox-mpi-wrapper.so -shared -ldl `pkg-config --libs glib-2.0`