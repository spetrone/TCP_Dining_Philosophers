
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


#define MAXLEN 128 //max size of message
#define PHIL_EAT_REQ 60
#define DEADLOCK_MAX 5 //program times out and exits after 10 seconds of deadlock

#ifndef CLIENT_H
#define CLIENT_H

int runClient(int, int);

#endif