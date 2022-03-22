/*  Author: Michael Trani
    March 2022       */

#include"p3.h"
#include"config.h"

void catch_sigterm();
void sigterm_handler(int signum, siginfo_t* info, void* ptr);


// Shared mem vars
int shmid_ticketNumber;
int* ticketNumber_ptr;
int shmid_choosing;
int* choosing_ptr;

std::string error_message;
std::ofstream logfile;

pid_t parental;
pid_t children;
int cprocess;

// for args
std::string timeIn, processnum;
int maxslaves;

int main(int argc, char* argv[]) {

    error_message = argv[0];
    error_message.erase(0, 2);        // removes annoying ./ at start
    error_message += "::ERROR: ";
    std::string logname = "logs/logfile.";

    srand((unsigned int)time(NULL));
    int timerLock = 1 + rand() % 5;  // used for both waiting periods

    // input fed from master
    int option;
    while ((option = getopt(argc, argv, "i:t:n:")) != -1) {
        switch (option)
        {
        case 'i':   // pid
            processnum = optarg;
            logname += processnum;
            break;

        case 't':   // time in
            timeIn = optarg;
            break;

        case 'n':   // how many processes requested
            maxslaves = atoi(optarg);
            break;

        default:
            break;
        }
    }

    //system("mkdir logs");
    logfile.open(logname);
    catch_sigterm();

    sem_t* sem;

    if ((sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0777, 1) )== SEM_FAILED) {
        error_message += "sem_open";
        perror(error_message.c_str());
        exit(-1);
    }

    // Log small details
    logfile << "Execution Time: " << timeIn << std::endl;
    logfile << "Process Number: " << processnum << std::endl;
    logfile << "This program waits: " << timerLock << " seconds.\n";

    // Shared Memory block
    shmkey = ftok("./slave", 246811);   // ##### Key 1
    shmid_choosing = shmget(shmkey, sizeof(choosing_ptr), 0777 | IPC_CREAT);
    choosing_ptr = (int*)shmat(shmid_choosing, NULL, 0);

    shmkey = ftok("./slave", 246812);   // #### Key #2
    shmid_ticketNumber = shmget(shmkey, sizeof(ticketNumber_ptr), 0777 | IPC_CREAT);
    ticketNumber_ptr = (int*)shmat(shmid_ticketNumber, NULL, 0);

    int shmid = shmget(SHMKEY, STR_SZ, 0777);   // get process id from parent
    if (shmid == -1) {
        std::cerr << "Slave: " << logname << " Error in shmget cint. " << std::endl;
        logfile << "Slave: " << logname << " Error in shmget cint. " << std::endl;
        exit(1);
    }

    int* cint = (int*)(shmat(shmid, 0, 0));  // read
    logfile << "Program number: " << *cint << std::endl;

    parental = getppid();
    children = getpid();
    cprocess = (int)(children - parental);
    int maximum = 0;

    for (int i = 0; i < PROCESS_RUNNING_MAX; i++) // Limit how many programs at a time
    {
        //execute code to enter critical section; log file this
        choosing_ptr[(cprocess - 1)] = 1;
        logfile << "Entering bakery\n";
        for (int h = 0; h < maxslaves; h++) {

            if ((ticketNumber_ptr[h]) > maximum)
                maximum = (ticketNumber_ptr[h]);
        }

        ticketNumber_ptr[(cprocess - 1)] = 1 + maximum;
        //std:: cout << "Slave Process:" << cprocess << " Ticket# "<< ticketNumber_ptr[(cprocess - 1)] << std::endl;
        logfile << timeFunction() << "  Ticket# " << ticketNumber_ptr[(cprocess - 1)] << std::endl;
        choosing_ptr[(cprocess - 1)] = 0;

        for (int j = 0; j < maxslaves; j++)
        {
            while (choosing_ptr[j] == 1) {
                ; // do nothing, just wait it out to flag choosing
            }

            while ((ticketNumber_ptr[j] != 0) && (ticketNumber_ptr[j] < ticketNumber_ptr[(cprocess - 1)])) {
                ; // do nothing, just wait it out to get ticket number
            }
        }


        //CRITICAL SECTION!!
        std::ofstream primary;
        primary.open("cstest", std::ios_base::app);
        sleep(timerLock);
        logfile << "Writing to log within Critical Section\n";
        primary << timeFunction() << " Queue " << ticketNumber_ptr[(cprocess - 1)] << "  File modified by " << processnum << std::endl;
        primary.close();
        sleep(timerLock);

        // closing critical section. setting ticket to 0
        ticketNumber_ptr[cprocess - 1] = 0;
    }

    shmdt(shared_num_ptr);
    shmdt(choosing_ptr);
    shmdt(ticketNumber_ptr);
    shmctl(shmid_shared_num, IPC_RMID, NULL);
    shmctl(shmid_choosing, IPC_RMID, NULL);
    shmctl(shmid_ticketNumber, IPC_RMID, NULL);

    logfile << "Completion Time: " << timeFunction() << "\n\n";
    logfile.close();

    return 0;

}


void sigterm_handler(int signum, siginfo_t* info, void* ptr) {
    // detaching and deleting shared memory
    shmdt(shared_num_ptr);
    shmdt(choosing_ptr);
    shmdt(ticketNumber_ptr);
    shmctl(shmid_shared_num, IPC_RMID, NULL);
    shmctl(shmid_choosing, IPC_RMID, NULL);
    shmctl(shmid_ticketNumber, IPC_RMID, NULL);

    logfile << "##### TERMINATED BY SIGNAL\n\n";
    logfile.close();
    exit(0);
}


void catch_sigterm() {
    static struct sigaction _sigact;
    memset(&_sigact, 0, sizeof(_sigact));
    _sigact.sa_sigaction = sigterm_handler;
    _sigact.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &_sigact, NULL);
}





/*
The slave just writes the message into the file inside the critical section.

We want to have some log messages to see that the process is behaving appropriately and it does follow the guidance required for critical section.
If a process starts to execute code to enter the critical section, it must print a message to that effect in its log file.
It will be a good idea to include the time when that happens.
Also, indicate the time in log file when the process actually enters and exits the critical section.
Within the critical section, wait for a random number of seconds (in the range [1,5]) before you write into the file, and then, wait for another [1,5] seconds before leaving the critical section.
For each child process, tweak the code so that the process requests and enters the critical section at most five times.


The bakery algorithm requires you to specify the number of processes in the system.

*/