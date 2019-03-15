CC=/opt/iot-devkit/1.7.2/sysroots/x86_64-pokysdk-linux/usr/bin/i586-poky-linux/i586-poky-linux-gcc
	
CC_1=gcc

EXECUTABLE = task.o

galileo:
	$(CC) -pthread -Wall -o $(EXECUTABLE) task.c

all:
	$(CC_1) -pthread -Wall -o $(EXECUTABLE) task.c
	
trace:
	sudo trace-cmd record -e sched_switch -e signal taskset 0x01 ./task.o n

clean:
	
	rm -f *.o
	rm -f $(EXECUTABLE) 