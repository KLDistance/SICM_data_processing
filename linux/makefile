bin/test1:src/main.c obj/csv_ops.o obj/parallel_processing.o
	gcc src/main.c -o bin/test1 obj/csv_ops.o obj/parallel_processing.o -Iinclude -lOpenCL -lpthread

obj/csv_ops.o:src/csv_ops.c
	gcc -c src/csv_ops.c -o obj/csv_ops.o -Iinclude

obj/parallel_processing.o:src/parallel_processing.c
	gcc -c src/parallel_processing.c -o obj/parallel_processing.o -Iinclude