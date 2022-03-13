/*  Author: Michael Trani
    March 2022       */
#include"p3.h"
#include"config.h"

pid_t waitreturn;	// for waiting on process to end

void child(int);
void parent(int);

std::string path = "./slave";
std::string slave_args = "-i ";
std::string slave_time = "-t ";

struct Shared_Space* SM;

int main(int argc, char* argv[]){

    std::string programName = argv[0];

    int option;
    int user_time = DEFAULT_TIME;
    int total_processes = 20;

    while ((option = getopt(argc, argv, "t:n:")) != -1) {
        switch (option)
        {
            case 't':
                 user_time = atoi(optarg);
                 break;
            case 'n':
                total_processes = atoi(optarg);
                if (total_processes > PROCESS_MAX) {
                    total_processes = PROCESS_MAX;
                    std::cout << "WARNING: process count(n) override: 20 Maximum.\n";
                }
                else
                    total_processes = atoi(optarg);
                break;
            default:
                break;
        }
    }

    time_t start_time = time(NULL);      // Creating time variables to use timer
    time_t current_time = time(NULL);
    pid_t activeChildren[PROCESS_MAX];  // Storage for active programs
    int process_counter = 0;

    for(int i = total_processes; i > 0; i--){



        int status = 0;
        switch (fork()) {
            case -1:
                std::cout << "Failed to fork" << std::endl;
                return (1);
                //do a perror

            case 0:
                activeChildren[i] = getpid();
                process_counter++;
                child(activeChildren[i]);
                break;

            default:
                parent(i);
                waitreturn = wait(&status);
                status = 0;
                break;
        }


    }

    return 0;
}

void parent(int temp){
    // Get shared memory segment identifier
    int shmid = shmget(SHMKEY, STR_SZ, 0777 | IPC_CREAT); // STRING
    if (shmid == -1){
        std::cout << "Parent: ... Error in shmget ..." << std::endl;
        exit(1);
    }

    // Get the pointer to shared block
    char* paddr = (char*)(shmat(shmid, 0, 0));
    int* pint = (int*)(paddr);  

    
    /* Write into the shared area. */
    *pint = temp;             
    sleep(2);
}


void child(int slave_pid){
    slave_args += std::to_string(slave_pid);
    slave_time += timeFunction();
    
    execl(path.c_str(),"slave", slave_args.c_str(), slave_time.c_str(), (char*)0);

    std::cout << "ERROR: excel failed to execute.\n" << std::endl;
}
