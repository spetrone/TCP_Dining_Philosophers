/*This is the implementation file for the functions used by the
clients (philosophers in the dining philosophers problem)

Author: Stephanie Petrone
Date: November 12, 2022
Last modified: november 12, 2022
*/

#include "client.h"

int runClient(int portL, int portR)
{
	
	int sockidL; //socket to connect to server for left hand
	int sockidR; //socket to connect to server for right hand
    struct sockaddr_in servAddrL; //server address for left hand
	struct sockaddr_in servAddrR; //server address for right hand
	char inputBuf[MAXLEN]; //buffer to sending strings
	char recBuf[MAXLEN]; //buffer for receiving echo
	int msgLength = 0; //length of message to send
	int count = 0; //number of bytes with send() and recv()
	int philEatCount = 0; //calculating how much philsosopher has eaten
	int leftFork = 0; //determines if the philospher has the left fork
	int rightFork = 0; //determines if the philosopher has the right fork
	int doneEating = 0; //used to determine if it is time for philosopher to think
	clock_t deadlockTime = 0; //deadlock time is counted in seconds


 	//set server address information for left hand
	memset(&servAddrL, '\0', sizeof(servAddrL));//initialize struct
	servAddrL.sin_family = AF_INET;
	servAddrL.sin_port = htons(portL);
	servAddrL.sin_addr.s_addr = inet_addr("127.0.0.1");

	//set server address information for right hand
	memset(&servAddrR, '\0', sizeof(servAddrR));//initialize struct
	servAddrR.sin_family = AF_INET;
	servAddrR.sin_port = htons(portR);
	servAddrR.sin_addr.s_addr = inet_addr("127.0.0.1");

	//create socket for left hand
	if((sockidL = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Error with socket(): \n");
		return -1;
	}

	//create socket for right hand
	if((sockidR = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Error with socket(): \n");
		return -1;
	}

	//connect to server for left hand
	if(connect(sockidL, (struct sockaddr *)&servAddrL, sizeof(servAddrL)) != 0)
	{
		perror("Error with connect(): \n");
		return -1;
	}

	//connect to server for right hand
	if(connect(sockidR, (struct sockaddr *)&servAddrR, sizeof(servAddrR)) != 0)
	{
		perror("Error with connect(): \n");
		return -1;
	}



	//dining philosophers problem, this is a philosopher's hand

	//loop until eaten enought or so much time has passed (deadlock?)
	clock_t start = clock(); //start time for philosopher trying to complete its process
	clock_t max = DEADLOCK_MAX * CLOCKS_PER_SEC;
	while (philEatCount < PHIL_EAT_REQ && (deadlockTime < max)) {

		
		//while the philosopher is not done eating (they are waiting)
		while (!doneEating && deadlockTime < max) {
			
			
			//wait for forks
			//left fork first, request from the fork server
			while(!leftFork && deadlockTime < max) {


				printf("\nmax is: %li\n", max);
				deadlockTime += ((clock() - start)); //add time to count towards deadlock
				printf("Deadlock time: %li,\n", deadlockTime);

				memset(inputBuf, '\0', sizeof(inputBuf)); //set buffer to 0
				strcpy(inputBuf, "request");
				msgLength = strlen(inputBuf); 

				if((count = send(sockidL, inputBuf, msgLength, 0)) != msgLength)
				{
					perror("Error with send(): \n");
					return -1;
				}
				else
					printf("requesting left fork ***\n");

				//receive echo
				memset(recBuf, '\0', sizeof(recBuf)); //reset buffer
				count = recv(sockidL, recBuf, sizeof(recBuf), 0);

				if (strcmp(recBuf, "given") == 0 ) {
					leftFork = 1;
				} //wil exit loop, otherwise will keep looping until it is given

			}
			//then picks up the right fork

			if(!rightFork) {
				memset(inputBuf, '\0', sizeof(inputBuf)); //set buffer to 0
				strcpy(inputBuf, "request");
				msgLength = strlen(inputBuf); 

				if((count = send(sockidR, inputBuf, msgLength, 0)) != msgLength)
				{
					perror("Error with send(): \n");
					return -1;
				}
				else
					printf("requesting right fork ***\n");

				//receive echo
				memset(recBuf, '\0', sizeof(recBuf)); //reset buffer
				count = recv(sockidR, recBuf, sizeof(recBuf), 0);
				printf("*** MESSAGE from fork server: %s \n", recBuf);

				if (strcmp(recBuf, "given") == 0 ) {
					rightFork = 1;
				} //will  eat at next step, otherwise put forks both back down
				else {
					
					leftFork = 0;
					rightFork = 0;

					//release the forks, right not taken so only return left
					memset(inputBuf, '\0', sizeof(inputBuf)); //set buffer to 0
					strcpy(inputBuf, "return");
					msgLength = strlen(inputBuf); 

					if((count = send(sockidL, inputBuf, msgLength, 0)) != msgLength)
					{
						perror("Error with send(): returning left fork\n");
						return -1;
					}
					else
						printf("returning left fork ***\n");

				}

			}

			//eat when they have both forks, each time they eat they have the forks for 2 second and 
			//increment eating amount by 
			if (rightFork == 1 && leftFork == 1) {
				printf("\n\nPHILOSOPHER HAS BOTH FORKS, EATING\n\n");
				philEatCount +=30 ;
				deadlockTime = 0; //not in deadlock, reset it to 0
				sleep(2); //eating time
				leftFork = 0;
				rightFork = 0;
				doneEating = 1; //set flag, time to think

				//release the forks, right then left
				memset(inputBuf, '\0', sizeof(inputBuf)); //set buffer to 0
				strcpy(inputBuf, "return");
				msgLength = strlen(inputBuf); 

				//communicate with server that philosopher client is returning the right fork
				if((count = send(sockidR, inputBuf, msgLength, 0)) != msgLength)
				{
					perror("Error with send(): returning right fork\n");
					return -1;
				}
				else
					printf("returning right fork ***\n");

				//communicate with server that philosopher client is returning the left fork
				memset(inputBuf, '\0', sizeof(inputBuf)); //set buffer to 0
				strcpy(inputBuf, "return");
				msgLength = strlen(inputBuf); 

				if((count = send(sockidL, inputBuf, msgLength, 0)) != msgLength)
				{
					perror("Error with send(): returning left fork\n");
					return -1;
				}
				else
					printf("returning left fork ***\n");
			}
		}

			//think only when done eating
			if(doneEating) {
				sleep(3); //think
				printf("\nPHILOSOPHER IS THINKING\n");
				//now philosopher needs to wait again, set flag
				doneEating = 0;
			}
	}
	
	//close left
 	close(sockidL); 

	//close right
	close(sockidR);

	if (deadlockTime > DEADLOCK_MAX) {
		printf("\n\n***********EXITED DUE TO DEADLOCK**************\n\n");
	}
	else {
		printf("\n\n*************** PHILOSOPHER IS DONE ***************\n\n");
	}
	

    return 0;

}