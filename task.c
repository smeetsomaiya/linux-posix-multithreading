#define _GNU_SOURCE
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<semaphore.h>
#include<fcntl.h>
#include<signal.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <sys/time.h>
#include "task.h"

int SLEEP_PERIOD_MULTIPLIER = 1000;

void exit_signal_handler(int signum) {
   if(signum == SIGUSR1) {
    printf("Received exit signal %d\n", signum);
	sem_destroy(&left_mouse);
	sem_destroy(&right_mouse);
    exit_flag = 1;
	struct timespec stop;
			clock_gettime(CLOCK_REALTIME, &stop);
		printf("Main exit %ld secs timestamp %ld nsecs", stop.tv_sec, stop.tv_nsec);
	fprintf(log_file, "\nMain exit timestamp %ld secs %ld nsecs\n", stop.tv_sec, stop.tv_nsec);
	fclose(log_file);
    }
}

int generate_uniform_rand(int min, int max) {
    return ((rand()/RAND_MAX)*(max-min)+min);
}


void* periodic_task(void *arguments)
{	
	struct  task_params_struct *args = arguments;
		struct timespec start, stop;
    int err;

	printf("Waiting for activation %d\n", args -> priority);

	pthread_mutex_lock(&activation_mutex);

    err = pthread_cond_wait(&activation_cv, &activation_mutex);
    if(err!=0)
        printf("Periodic condition variable error %d\n",err);
		printf("Activated %d\n", args -> priority);
    pthread_mutex_unlock(&activation_mutex);

    while(exit_flag == 0) {

    err = pthread_sigmask(SIG_BLOCK,&set,NULL);
    if (err != 0)
        printf("\nSignal not masked:[%s]", strerror(err));
    else
        printf("\nSignal Masked\n");

    printf("Now executing %d\n", args -> priority);

   int i, j=0;
	printf("%d %d", args -> min_value, args-> max_value);
   int x = generate_uniform_rand(args -> min_value, args -> max_value);
   printf("Number of iterations is - %d\n", x);
		clock_gettime(CLOCK_REALTIME, &start);
   for(i = 0; i < x; i++) {
       j = j+i;
   }
	clock_gettime(CLOCK_REALTIME, &stop);
        fprintf(log_file,"Thread of priority %d\t",args -> priority);
        fprintf(log_file,"Number of iterations %d\t",x);
	fprintf(log_file, "Start time %ld\t", start.tv_nsec);
	fprintf(log_file, "Stop time %ld\t", stop.tv_nsec);
	fprintf(log_file, "Elapsed time %ld\n\n", (stop.tv_nsec - start.tv_nsec));

   int sleep_period = args -> period;
   printf("Sleep period %d\n", sleep_period);

   int  err = pthread_sigmask(SIG_UNBLOCK,&set,NULL);
   if (err != 0)
        printf("\nSignal ublock failed :[%s]", strerror(err));
    else
       printf("\nSignal unblocked\n");

   usleep(SLEEP_PERIOD_MULTIPLIER*args->period);
   }

    printf("pthread exiting %d\n", args->priority);

    pthread_exit(NULL);
    return NULL;
}

void* aperiodic_task(void *arguments) {
    struct  task_params_struct *args = arguments;
	struct timespec start, stop;
    int i,j = 0;

    while(exit_flag == 0) {
    printf("Wait for mouse events\n");

    if(args->mouse_event_type == 1) { //Left click event
        sem_wait(&left_mouse);
        printf("This is an aperiodic task for left mouse click\n");
    } else if(args->mouse_event_type == 2) { //Right click event
        sem_wait(&right_mouse);
        printf("This is an aperiodic task for right mouse click\n");
    }

    int x = generate_uniform_rand(args -> min_value, args -> max_value);
	printf("%d %d", args -> min_value, args-> max_value);
   printf("Number of iterations is - %d\n", x);
	clock_gettime(CLOCK_REALTIME, &start);
        for(i=0; i<x;i++) {
            j=j+i;
        }
	clock_gettime(CLOCK_REALTIME, &stop);
        fprintf(log_file,"Aperiodic task - thread of priority %d\t", args->priority);
	fprintf(log_file,"Number of iterations %d\t",x);
	fprintf(log_file, "Start time %ld\t", start.tv_nsec);
	fprintf(log_file, "Stop time %ld\t", stop.tv_nsec);
	fprintf(log_file, "Elapsed time %ld\n\n", (stop.tv_nsec - start.tv_nsec));
    }

    printf("pthread exiting %d\n", args->priority);
    pthread_exit(NULL);
    return NULL;
}

void* mouse_thread(void *arguments) {
    int fd, bytes, left, right, previous_left, previous_right;
    unsigned char data[1];

    const char *mDevice = MOUSE_DEV;

    // Open Mouse
    fd = open(mDevice, O_RDWR);
    if(fd == -1)
    {
        printf("ERROR Opening %s\n", mDevice);
		return NULL;
    }
	   printf("Mouse ready %s\n", mDevice);

    while(exit_flag == 0)
    {
    bytes = read(fd, data, sizeof(data));
    if(bytes > 0)
    {
        left = data[0] & 0x1;

        if(previous_left > left) {
//            sem_post(&left_mouse);
            printf("Left mouse click\n");
        }
        previous_left = left;

        right = data[0] & 0x2;

        if(previous_right > right) {
//            sem_post(&right_mouse);
            printf("Right mouse click\n");
        }
        previous_right = right;

        printf("left=%d, right=%d\n",left, right);
    }   
    }
    printf("Mouse thread exiting\n");
    pthread_exit(NULL);
    return NULL;
}

void create_mouse_thread() {
	pthread_t mouse_thread_id;
	CPU_ZERO(&cpuset);
	CPU_SET(0,&cpuset);
	
	//Initialize mouse semaphores in a locked state
    sem_init(&left_mouse, 0, 0);
    sem_init(&right_mouse, 0, 0);

    int err = pthread_create(&mouse_thread_id, NULL, &mouse_thread, NULL);
    if (err != 0) {
        printf("\ncan't create mouse thread :[%s]", strerror(err));
//        pthread_setaffinity_np(mouse_thread_id,sizeof(cpu_set_t),&cpuset);
    }
    else
        printf("\n Mouse Thread created successfully\n");
}

void exit_signal_init() {
   sigemptyset(&set);
   sigaddset(&set, SIGUSR1);
   signal(SIGUSR1, exit_signal_handler);
}

int main(void)
{
	struct timespec start;
    log_file = fopen(LOG_FILE, "w+");
	clock_gettime(CLOCK_REALTIME, &start);
	printf("Main begin timestamp %ld secs %ld nsecs", start.tv_sec, start.tv_nsec);
	fprintf(log_file, "Main begin timestamp %ld secs %ld nsecs\n\n", start.tv_sec, start.tv_nsec);

    //Pthread variables
    pthread_t tid[10];
    pthread_attr_t thread_attributes;

	// Variables to read input from the file
    FILE *file;
	int j, err;
    char *c = NULL;
    ssize_t nread;
    size_t len = 0;
	struct task_details_struct det;
    struct task_params_struct params[100];
    struct sched_param s_priority;

    /*
       Variables to set cpu affinity
    */
	CPU_ZERO(&cpuset);
	CPU_SET(0,&cpuset);


	exit_signal_init();
	
    file = fopen("input.txt", "r");
    if(file) {
    	int line_count = 1;
    	while((nread = getline(&c,&len,file)) != -1) {
	        if(line_count == 1) {
	        int tasks = atoi(strsep(&c, " "));
	        int total_period = atoi(strsep(&c, ";"));

	   		printf("\nNum of tasks %d\n",tasks);
	   		printf("Time period %d\n\n",total_period);

	        det.num_tasks = tasks;
	        det.max_exec_time = total_period;
	        }
	        else {
	        params[line_count-2].periodic = atoi(strsep(&c, " "));
	        params[line_count-2].priority = atoi(strsep(&c, " "));
	        params[line_count-2].period = atoi(strsep(&c, " "));
	       	params[line_count-2].min_value = atoi(strsep(&c, " "));
	        params[line_count-2].max_value = atoi(strsep(&c, " "));
	        params[line_count-2].mouse_event_type = atoi(strsep(&c, " "));
	        }
	        line_count++;
	    }
		fclose(file);
    }

	pthread_mutex_init(&activation_mutex, NULL);

    err = pthread_cond_init(&activation_cv,NULL);            // initialising condition variables
    if(err!=0)
        printf("Activation condition init failed %d\n",err);

	create_mouse_thread();


    for(j = 0; j < det.num_tasks; j++)
    {
    err = pthread_attr_init(&thread_attributes);
    if (err != 0)
        printf("\ncan't create thread attributes :[%s]", strerror(err));
    else
        printf("\nThread attributes created successfully\n");

    err = pthread_attr_setschedpolicy(&thread_attributes, SCHED_FIFO);
    if(err != 0) {
        printf("FIFO not set %d\n", err);
    }
    err = pthread_attr_getschedparam(&thread_attributes, &s_priority);
    if(err != 0) {
        printf("Get sched params failed %d\n", err);
    }
    printf("Initial priority is %d\n", s_priority.sched_priority);

    s_priority.sched_priority = params[j].priority;

    err = pthread_attr_setschedparam(&thread_attributes, &s_priority);
    if(err != 0) {
        printf("Set sched params failed %d\n", err);
    }

    err = pthread_attr_getschedparam(&thread_attributes, &s_priority);
    printf("Updated priority is %d\n", s_priority.sched_priority);

    printf("Thread period is %d\n", params[j].period);

    if(params[j].periodic == 1) {
        err = pthread_create(&(tid[j]), &thread_attributes, &aperiodic_task, (void *)&params[j]);
    } else {
    err = pthread_create(&(tid[j]), &thread_attributes, &periodic_task, (void *)&params[j]);
    }
    if (err != 0) {
        printf("\ncan't create thread :[%s]", strerror(err));
        pthread_setaffinity_np(tid[j],sizeof(cpu_set_t),&cpuset);
    }
    else
        printf("\n Thread created successfully\n");
    }

	pthread_cond_broadcast(&activation_cv);

    usleep(SLEEP_PERIOD_MULTIPLIER*3000);

    kill(getpid(), SIGUSR1);

    exit(0);
}
