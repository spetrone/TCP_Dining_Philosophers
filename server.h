/*This is the header file for the functions used by the
server in the dining philosophers problem

Author: Stephanie Petrone
Date: November 12, 2022
Last modified: november 12, 2022
*/

#include <stdio.h>
#include <stdio.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <errno.h> //for semaphore EAGAIN
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */

#define MAXLEN 128  //max byte size of message


#ifndef SERVER_H
#define SERVER_H



int runServer(int);

#endif