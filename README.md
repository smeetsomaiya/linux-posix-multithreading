# Real-time Task Models in Linux POSIX

Input Files Required : task.c
                       task.h
                       input.txt
                       Makefile

To compile the program


Open the Linux terminal and navigate to the directory using cd/path r 
To compile for Galileo:

	make galileo

To compile for other x86 architecture:
	
	 make all
 
make command will create object file task.o
Run the object file using 

	./task.o

To run on kernelshark

	 make trace
