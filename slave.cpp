/*  Author: Michael Trani
    February 2022       */

#include"p2.h"


int main(int argc, char* argv[]){   

    srand(time(0));
    int timerLock = 1 + rand() % 5;  // used for waiting period
    std::string programName = argv[0];  // perror msg
    std::string logname = "logs/logfile."; 
    std::string timeIn, processnum;
    int option;

    while ((option = getopt(argc, argv, "i:t:")) != -1) {
        switch (option)
        {
            case 'i':
                processnum = whitespaceRemover(optarg);
                logname += processnum;
                break;

            case 't':
                timeIn = whitespaceRemover(optarg); 

            default:
                break;
        }
    }


    std::ofstream logfile(logname);
    std::ofstream primary("cstest");
    logfile << "Execution Time: " << timeIn << std::endl;
    logfile << "This program waits: " << timerLock << " seconds.\n";

    sleep(5);
    int shmid = shmget(SHMKEY, STR_SZ, 0777);   // string
    if (shmid == -1){
        std::cerr << "Slave: " << logname << " Error in shmget cint. " << std::endl;
        logfile << "Slave: " << logname << " Error in shmget cint. " << std::endl;
        exit(1);
    }

    int* cint = (int*)(shmat(shmid, 0, 0));  // read
    logfile << "ticket number: " << *cint << std::endl;

    /*
    for ( i = 0; i < 5; i++ )
    {
        execute code to enter critical section; log file this
        sleep(timerLock);
        primary << timeFunction() <<  " Queue " << ticketnumber << "File modified by " << processnum << std::endl;
        sleep(timerLock);
        execute code to exit from critical section;
    }
    
    */



    logfile << "Completion Time: " << timeFunction() << std::endl;;
    logfile.close();
    return 0;
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