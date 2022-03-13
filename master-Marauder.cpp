/*  Author: Michael Trani
    February 2022       */
#include"p2.h"
#include"config.h"

pid_t waitreturn;      // for waiting on process to end
int shmid;

void child();
void parent(int);
void sigalrm_handler(int signum, siginfo_t* info, void* ptr);
void sigint_handler(int signum, siginfo_t* info, void* ptr);
void sigalrm_catcher();
void sigint_catcher();

std::string error_message;
std::string warning_message;

std::string path = "./slave";

pid_t* child_pid;
int active_process_counter = 1;
int total_processes = 5;



int main(int argc, char* argv[]) {
    error_message = argv[0];
    error_message.erase(0, 2);        // removes annoying ./ at start
    warning_message = error_message;
    warning_message += "--WARNING--";
    error_message += "::ERROR: ";

    // user args section
    int option;
    int user_time = DEFAULT_TIME;

    while ((option = getopt(argc, argv, "ht:n:")) != -1) {
        switch (option) {
        case 'h':
            std::cout << "-t ss\tTimeout in seconds\n" <<
                "-n uu\tNumber of total processes to run.\n";
            return 0;

        case 't':  // How long should it run before timing out - user input
            user_time = atoi(optarg);
            break;

        case 'n': // How many processes should it run - user input
            total_processes = atoi(optarg);
            if (total_processes > PROCESS_RUNNING_MAX) {
                total_processes = PROCESS_RUNNING_MAX;
                std::cout << "WARNING: process count(n) override: 20 Maximum.\n";
            }
            else
                total_processes = atoi(optarg);
            break;

        default:
            std::cout << "Unknown input\n";
            return 1;
        }
    }

    if (user_time <= 1 || total_processes < 1) {
        std::cout << error_message << "::Invalid input:\n\tTime must be greater than 1.\n\tProcesses must be more than 0.\n";
        return 1;
    }

    // Shared memory initialization section
   
    // Get shared memory segment identifier
    shmid = shmget(SHMKEY, STR_SZ, 0777 | IPC_CREAT); // STRING
    if (shmid == -1) {
        std::cout << "Parent: ... Error in shmget ..." << std::endl;
        exit(1);
    }

    // for signal handling
    shmkey = ftok("./master", 246810);   // ##### KEY 1
    shmid_shared_num = shmget(shmkey, sizeof(shared_num_ptr), 0777 | IPC_CREAT);
    shared_num_ptr = (int *)shmat(shmid_shared_num, NULL, 0);
    shared_num_ptr[0] = 0;

    char slave_max_stack[PROCESS_RUNNING_MAX];  // for writing to the buffer - for running
    int slave_inc = 5;  // 5 max processes at at time
    snprintf(slave_max_stack, PROCESS_RUNNING_MAX, "%i", slave_inc);

    char slave_max_to_run[PROCESS_COUNT_MAX];   // for writing to the buffer - for exe count
    snprintf(slave_max_to_run, PROCESS_COUNT_MAX, "% i", total_processes);

    // for PIDs
    if ((child_pid = (pid_t*)(malloc(total_processes * sizeof(pid_t)))) == NULL) {
        errno = ENOMEM; // out of memory error
        error_message += "Malloc total_processes out of memory.\n";
        perror(error_message.c_str());
        exit(1);
    }


    // Main loop:
    pid_t parental;

    // Signal catchers after shared mem established
    sigalrm_catcher();
    sigint_catcher();

    //get start time
    alarm(user_time);

    do {
        if (active_process_counter < PROCESS_COUNT_MAX) {
            switch (parental = fork()) {

            case -1:
                error_message += "::Failed to fork.\n";
                perror(error_message.c_str());
                return (1);

            case 0:
                child_pid[active_process_counter] = parental;
                child();
                break;

            default:
                parent(active_process_counter);
                break;
            }
            active_process_counter++;
        }


    } while (active_process_counter < (total_processes + 1));

    // Wait for everything to end.
    for (int i = 1; i < total_processes; i++) {
        int status;
        waitpid(child_pid[i], &status, 0);  // should match active_process_counter
    }

    free(child_pid);
    shmdt(shared_num_ptr);
    shmctl(shmid_shared_num, IPC_RMID, NULL);

    return 0;
}


// Parent Child functions
void parent(int temp) {
    // Get the pointer to shared block
    char* paddr = (char*)(shmat(shmid, 0, 0));
    int* pint = (int*)(paddr);

    /* Write into the shared area. */
    *pint = temp;
    //sleep(2);   // is this necessary?
}

void child() {

    pid_t temp = getpid();  

    std::string slave_pid_arg = std::to_string(temp);  // argument for slave - PID
    std::string slave_time = timeFunction();       // argument for slave - time
    std::string slave_max = std::to_string(total_processes);    // argument for total num of processes

    execl("./slave", "slave", "-i", slave_pid_arg.c_str(), "-t", slave_time.c_str(), "-n", slave_max.c_str(), (char*) NULL);
    
    // If we get to this point the call failed.
    error_message += "::excel failed to execute.\n";
    perror(error_message.c_str());
    std::cout << "excel failed to execute.\n" << std::endl;
}

// Signal handlers
void sigalrm_handler(int signum, siginfo_t* info, void* ptr) {
    // ignore other interrupt signals
    signal(SIGINT, SIG_IGN);

    std::cout << warning_message << " Child processes exceeded limit\n";

    // clean shared mem
    shmdt(shared_num_ptr);
    shmctl(shmid_shared_num, IPC_RMID, NULL);

    pid_t ctemp[total_processes]; // temporary processes for slaves

    for (int i = 0; i < total_processes; i++) {
        ctemp[i] = child_pid[i];
    }

    free(child_pid);

    // kill signal for slaves
    for (int j = 0; j < total_processes; j++) {
        kill(ctemp[j], SIGTERM);
    }

}

void sigint_handler(int signum, siginfo_t* info, void* ptr) {
    // prevents multiple interrupts
    signal(SIGINT, SIG_IGN);
    signal(SIGALRM, SIG_IGN);

    std::cout << warning_message << " Interrupt. Exiting.\n";

    // clean shared mem
    shmdt(shared_num_ptr);
    shmctl(shmid_shared_num, IPC_RMID, NULL);

    pid_t ctemp[total_processes]; // temporary processes for slaves

    for (int i = 0; i < total_processes; i++) {
        ctemp[i] = child_pid[i];
    }

    free(child_pid);

    // kill signal for slaves
    for (int j = 0; j < total_processes; j++) {
        kill(ctemp[j], SIGTERM);
    }
}

void sigalrm_catcher() {
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));

    _sigact.sa_sigaction = sigalrm_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGALRM, &_sigact, NULL);
}

void sigint_catcher() {
    static struct sigaction _sigact;

    memset(&_sigact, 0, sizeof(_sigact));

    _sigact.sa_sigaction = sigint_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGINT, &_sigact, NULL);
}