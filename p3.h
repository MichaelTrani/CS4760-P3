/*  Author: Michael Trani
    February 2022       */
#ifndef P2_H
#define P2_H
#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <semaphore.h>
#include <fcntl.h>

#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <signal.h>

#define SHMKEY  859047
#define STR_SZ  sizeof ( std::string )
#define INT_SZ  sizeof ( int )
#define PERM (S_IRUSR | S_IWUSR)
#define SEMAPHORE_NAME "/semaphore"


    //shared memory attempt #3 :p
key_t shmkey;
int shmid_shared_num;
int* shared_num_ptr;

//Grabs time
char* timeFunction() { // Grabs current time and outputs hour/min/sec
    time_t current_sec = time(0);
    int length = 9;
    std::string formatted_time = "%H:%M:%S";

    struct tm* local = localtime(&current_sec);

    char* output = new char[length];
    strftime(output, length, formatted_time.c_str(), local);
    return output;
}

// Removes whitespace - no longer needed
std::string whitespaceRemover(std::string modifyME) { // This removes an annoying whitespace fed into the program.
    remove(modifyME.begin(), modifyME.end(), ' ');

    return modifyME;
}


#endif

/*
    std::time_t now = time(0); // get time
    char* stringnow = ctime(&now);
    std::cout << "time caught: " << stringnow << std::endl; // outputs full date
    std::cout << timeFunction() << std::endl;

*/
