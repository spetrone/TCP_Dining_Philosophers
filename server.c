/*This is the implementation file for the functions used by the
servers (forks) in the dining philosophers problem
Author: Stephanie Petrone
Date: November 12, 2022
Last modified: november 12, 2022
*/
#include "server.h"

#define NUM_CON 2 //number of connections serviced by this


int runServer(int port)
{
    unsigned short listen_port = port;
	struct sockaddr_in clientAddr; //holds client address of sock
	int addrLen; //holds lenght of client adress
	int count = 0 ; //bytes received wirth recv()
	int connectSock; //new socket with connection
	int listenSock; //socket for listen()
	char buf[MAXLEN]; //buffer for receiving 
	char inputBuf[MAXLEN]; //buffer to sending strings
	int msgLength = 0; //length of message to send
	struct sockaddr_in serverSockAddr;
	char IP_str[16]; //for holding string of IP addr (up tp 16 chars)
	int completeCon = 0; //counter of number of connections hich have been completed
	pid_t pid; //for child processes with
	

	int status; //holds status of child process when parent waiting
	pid_t pidS; //for slave process pid when testing if they are finished from parent
	int slaveProcCount = 0; //used for counting completed slave processes to close listening socket

	//named semaphore information
	char semName[8] = "sem";
	char semNumStr[5];
	int locktest = -1; //used for testing semaphore

	//create the named semaphore,  named SEM9xxx with 9xxx being the port number for the server
	sprintf(semNumStr, "%d", port);
	strcat(semName, semNumStr);
	printf("SEMAPHORE NAME: %s", semName);
	sem_t *mutex = sem_open(semName, O_CREAT | O_EXCL, 0644, 1);

	//test if there were errors in creation of semaphore
    if (mutex == SEM_FAILED) {
        perror("sem_open(3) error");
        exit(1);
    }

	//Close the semaphore not using it in the parent process 
    if (sem_close(mutex) < 0) {
        perror("sem_close(3) failed");
        //ignore sem_unlink(3) errors
        sem_unlink(semName);
        exit(1);
    } 

	//create socket for listening at the port number passed to the function
	listenSock = socket(PF_INET, SOCK_STREAM, 0);

    //set address for the listening
	memset(&serverSockAddr, '0', sizeof(serverSockAddr));
	serverSockAddr.sin_family = AF_INET;
	serverSockAddr.sin_port = htons(listen_port);
	serverSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	

	printf( "Here and pid val is: %d\n", pid);
	//bind socket
	if( bind(listenSock, (struct sockaddr* )&serverSockAddr, sizeof(serverSockAddr)) == -1)
	{
        perror("Error with bind(): ");
		exit(1);
	}
	else
	{
		//ip to string for printing
		memset(IP_str, '\0', sizeof(IP_str));
		strcpy(IP_str, inet_ntoa(serverSockAddr.sin_addr));
		
		printf("\nlistenSock binded to port %u\n", listen_port);
		printf("\nIP address: %s\n\n", IP_str); 
	}


	//listen for connection
	if (listen(listenSock, 10) == -1)
	{
		perror("Error with listen(): ");
		return -1;
	}

	//print status
	printf("\nServer listening on port: %u...\n", listen_port);

    
	//iteratively accept connection, one for each philosopher serviced
	while(completeCon < NUM_CON)
	{
		//accept new connection
		if(( connectSock = accept(listenSock,(struct sockaddr *) &clientAddr, &addrLen)) == -1)
		{
			perror("Error with accept(): ");
			return -1;
		}
		else {
		
			printf("\nSuccesfully connected to client socketfd: %d,\n\n", connectSock);

			printf("...............Spawning slave process................\n");

			if ((pid = fork()) < 0) {
				perror("Error spawning slave process");
				exit(1);
			}

			if (pid == 0) { //within each child process

			//open semaphore in child
			mutex = sem_open(semName, O_RDWR);
			if (mutex == SEM_FAILED) {
				perror("child sem_open(3) failed");
				exit(EXIT_FAILURE);
			}

			sem_post(mutex); //start with an unlocked mutex

			//receive first message then loop for more receiving
			memset(buf, '\0', sizeof(buf)); //clear buffer
			if((count = recv(connectSock, buf, sizeof(buf), 0)) < 0)
			{
				perror("error with first recv(): ");
				return -1;
			}

			//Run as a server process, one for each philosopher serviced, sharing the fork
			while (count > 0) {

				if((strcmp(buf, "request")) == 0) {

					//test avilability of mutex/fork
					if ((sem_trywait(mutex)) < 0)
					printf("\n\n\nIN trywait failed\n\n");
						//printf("*****LOCKTEST: %d on mutex\n", locktest);
						if (errno == EAGAIN){
						printf("\n\nlocked!\n\n");
						memset(inputBuf, '\0', sizeof(buf)); //clear buffer
						strcpy(inputBuf, "locked"); //the message to the client about status of fork
						msgLength = strlen(inputBuf);
					}
					else {
						memset(inputBuf, '\0', sizeof(buf)); //clear buffer
						strcpy(inputBuf, "given"); //the message to the client about status of fork
						msgLength = strlen(inputBuf);
					}

					//testing mutex
					/*else if (locktest == -1){
						perror("Error with mutex in server doing trywait()");
						exit(1);
					} */

				}
				else if((strcmp(buf, "return")) == 0) {
					memset(inputBuf, '\0', sizeof(buf)); //clear buffer
					strcpy(inputBuf, "fork_returned");
					msgLength = strlen(inputBuf);
					sem_post(mutex);

				} 

				//send message about whether fork is taken or not
				if ((send(connectSock, inputBuf, msgLength, 0)) != msgLength)
				{
					perror("Error with send(): ");
				}

				//receive next message
				memset(buf, '\0', sizeof(inputBuf)); //reset buffer
				count = recv(connectSock, buf, sizeof(buf), 0); 

			}
					

			//close client socket 
			close(connectSock);

			if (sem_close(mutex) < 0)
        		perror("sem_close(3) failed");

			exit(0); //exit child process
				
			}
			else { //in parent
				completeCon++; //manage number of connections (used for waiting on # of child processes)
				close(connectSock); //doesn't use socket, so close it
			}
		}	
	}

	/* Wait for children to exit. */
	slaveProcCount = completeCon;
	while (slaveProcCount > 0) {
 	pidS = wait(&status);
  	printf("Slave process with PID %ld exited with status 0x%x.\n", (long)pidS, status);
  	--slaveProcCount; 
	}
	
	//unlink semaphore
	if (sem_unlink(semName) < 0)
    perror("sem_unlink(3) failed");

	//all slave processes must have completed (from while loop above)
	//close the listening socket
	close(listenSock);

}