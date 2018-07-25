bin/sicm_data_processing:src/main.c obj/csv_ops.o obj/parallel_processing.o
	gcc src/main.c -o bin/sicm_data_processing obj/csv_ops.o obj/parallel_processing.o -Iinclude -lOpenCL -lpthread -lm

obj/csv_ops.o:src/csv_ops.c
	gcc -c src/csv_ops.c -o obj/csv_ops.o -Iinclude

obj/parallel_processing.o:src/parallel_processing.c
	gcc -c src/parallel_processing.c -o obj/parallel_processing.o -Iinclude