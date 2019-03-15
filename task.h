#define MOUSE_DEV "/dev/input/mice" //File to read mouse values

#define LOG_FILE "log.txt" //Log.txt output file
FILE *log_file;

int exit_flag = 0;

// Structures to read input from the file
struct task_details_struct
{
	int num_tasks;
	int max_exec_time;
};

struct task_params_struct
{
	int periodic; //0 for periodic, 1 for aperiodic
	int priority;
	int period;
	int min_value;
	int max_value;
	int mouse_event_type; // Only for aperiodic events, 0 otherwise
};

//Thread-activation wait variables
pthread_mutex_t activation_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t activation_cv = PTHREAD_COND_INITIALIZER;

cpu_set_t cpuset;

sem_t mutex; //Semaphore to wait for activation

//Semaphore to post mouse events
sem_t left_mouse;
sem_t right_mouse;

// Function to read CPU time stamp counter
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

// Signal to exit the threads
sigset_t set;
