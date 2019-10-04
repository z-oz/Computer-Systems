/*-------------------------------------------------------------------------*
 *---                         					        				---*	
 *--- 			      mathServer.c                              		---*
 *---                                                                   ---*
 *---     This file defines a C program that gets file-sys commands     ---*
 *--- from client via a socket, executes those commands in their own    ---*
 *--- threads, and returns the corresponding output back to the         ---*
 *--- client.                                                           ---*
 *---                                                                   ---*
 *---  ----    ----    ----    ----    ----    ----    ----    ----     ---*
 *---                                                                   ---*
 *---  Version 1.0                                     Joseph Phillips  ---*
 *---                                                                   ---*
 *-------------------------------------------------------------------------*/
//Compile with:
//$ gcc mathServer.c -o mathServer -lpthread

//---Header file inclusion---//

#include "mathClientServer.h"
#include <errno.h> // For perror()
#include <pthread.h> // For pthread_create()


//---Definition of constants:---//

#define STD_OKAY_MSG "Okay"

#define STD_ERROR_MSG "Error doing operation"

#define STD_BYE_MSG "Good bye!"

#define THIS_PROGRAM_NAME "mathServer"

#define FILENAME_EXTENSION ".bc"

#define OUTPUT_FILENAME "out.txt"

#define ERROR_FILENAME "err.txt"

#define CALC_PROGNAME "/usr/bin/bc"

const int ERROR_FD = -1;

extern void* handleClient(void* vPtr);
extern void dirCommand(int fd);
extern void readCommand(int clientFd, int fileNum);
extern void writeCommand(int clientFd, int fileNum, char* text);
extern void deleteCommand(int clientFd, int fileNum);
extern void* calcCommand(int clientFd, int fileNum);

//---Definition of functions:---//

//  PURPOSE:  To run the server by 'accept()'-ing client requests from
//'listenFd' and doing them.
void doServer(int 		listenFd
			 ) 
{
  	//  I.  Application validiity check:

  	//  II.  Server clients:
  	pthread_t threadId;
  	pthread_attr_t threadAttr;
  	int threadCount = 0;

	// YOUR CODE HERE
  	int* iPtr;
  	pthread_attr_init(&threadAttr);
	
  	while (1)  
	{
		int  clientFd = accept(listenFd,NULL,NULL);
		printf("Thread %d starting.\n", threadCount);
		int* iPtr = (int*)calloc(2,sizeof(int*));
		iPtr[0] = clientFd;
		iPtr[1] = threadCount++;
		pthread_attr_setdetachstate(&threadAttr,PTHREAD_CREATE_DETACHED);
		pthread_create(&threadId,&threadAttr,handleClient,(void*)iPtr);
  	}
	pthread_attr_destroy(&threadAttr);
}

void* handleClient(void* vPtr) 
{
 	int* iPtr  = (int*)vPtr;
  	int clientFd  = iPtr[0];
  	int* threadId = &iPtr[1];
  	free(vPtr);

  	//  II.B.  Read command:
  	char  buffer[BUFFER_LEN];
  	char  command;
  	int  fileNum;
  	char text[BUFFER_LEN];
  	int shouldContinue= 1;
  	char *textPtr;

  	while  (shouldContinue)
    	{
			// text[0]	= '\0';
      		memset(buffer,'\0',BUFFER_LEN);
      		memset(text  ,'\0',BUFFER_LEN);
      		
			read(clientFd,buffer,BUFFER_LEN);
      		printf("Thread %d received: %s\n",*threadId,buffer);
      		sscanf(buffer,"%c %d \"%[^\"]\"",&command,&fileNum,text);

     		switch (command) 
			{
      			case DIR_CMD_CHAR:
        			dirCommand(clientFd);
        			break;
				case READ_CMD_CHAR:
        			readCommand(clientFd,fileNum);
        			break;
      			case WRITE_CMD_CHAR:
        			writeCommand(clientFd,fileNum, text);
        			break;
      			case DELETE_CMD_CHAR:
        			deleteCommand(clientFd,fileNum);
        			break;
     			case CALC_CMD_CHAR:
        			calcCommand(clientFd,fileNum);
        			break;
      			case QUIT_CMD_CHAR:
        			write(clientFd, STD_BYE_MSG, sizeof(STD_BYE_MSG));
					close(clientFd);
        			shouldContinue = 0;
        			printf("Thread %d quitting. \n",*threadId);
					exit(EXIT_SUCCESS);
        			break;
        	}
      	}
  	printf("Thread %d quitting. \n",*threadId);
  	return(NULL);
}

void dirCommand(int fd) 
{
  	DIR* dirPtr = opendir(".");
  	if (dirPtr == NULL) 
		write(fd,STD_ERROR_MSG,strlen(STD_ERROR_MSG));

  	struct    dirent*   entryPtr;
  	char buffer[BUFFER_LEN];
  	char*filename;
  	while ( (entryPtr = readdir(dirPtr)) != NULL )
    {
		filename = entryPtr->d_name;
		strncat(buffer,filename,BUFFER_LEN);
		strncat(buffer,"\n",BUFFER_LEN);
	}
  	closedir(dirPtr);
  	write(fd,buffer,BUFFER_LEN);
}

void readCommand(int clientFd, int fileNum) 
{
  	char fileName[BUFFER_LEN];
  	snprintf(fileName, BUFFER_LEN, "%d%s", fileNum, FILENAME_EXTENSION);
	fileNum = open(fileName, O_RDONLY, 0);

  	char buffer[BUFFER_LEN];
  	char line[BUFFER_LEN];
	int NumBytes;
  	if (fileNum < 0) 
    		write(clientFd,  STD_ERROR_MSG, sizeof(STD_ERROR_MSG));

	NumBytes = read(fileNum, buffer, BUFFER_LEN);
	write(clientFd, buffer, NumBytes);
  	close(fileNum);
}

void writeCommand(int clientFd, int  fileNum, char* text) 
{
  	char fileName[BUFFER_LEN];
  	snprintf(fileName, BUFFER_LEN, "%d%s", fileNum, FILENAME_EXTENSION);
	fileNum = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0660);

  	if (fileNum < 0) 	
    	write(clientFd, STD_ERROR_MSG, sizeof(STD_ERROR_MSG));
  	else 
	{
		write(fileNum,text, fileNum);
		write(clientFd,STD_OKAY_MSG,sizeof(STD_OKAY_MSG));
  	}
  	close(fileNum);
}

void deleteCommand(int clientFd, int fileNum) 
{
  	char fileName[BUFFER_LEN];
  	snprintf(fileName,BUFFER_LEN,"%d%s",fileNum,FILENAME_EXTENSION);

  	int status = unlink(fileName);
  	if (status < 0)
		write(clientFd,STD_ERROR_MSG,sizeof(STD_ERROR_MSG));
  	else 
		write(clientFd, STD_OKAY_MSG,sizeof(STD_OKAY_MSG));
}


void* calcCommand(int clientFd, int fileNum) 
{
  	pid_t childId = fork();
  	int status;
  	char buffer[BUFFER_LEN];

  	if (childId < 0) 
    	write(clientFd,STD_ERROR_MSG,sizeof(STD_ERROR_MSG)); 
  	else if (childId == 0) 
	{
		char fileName[BUFFER_LEN];
		snprintf(fileName,BUFFER_LEN,"%d%s",fileNum,FILENAME_EXTENSION);
		int inFd= open(fileName,O_RDONLY,0);
		int outFd= open(OUTPUT_FILENAME,O_WRONLY|O_CREAT|O_TRUNC,0660);
		int errFd= open(ERROR_FILENAME, O_WRONLY | O_CREAT | O_TRUNC,0660);

		if  ( (inFd < 0) || (outFd < 0) || (errFd < 0) )
		{
			fprintf(stderr,"Could not open one or more files\n");
			exit(EXIT_FAILURE);
		}
		close(0);
		dup(inFd);
		close(1);
		dup(outFd);
		close(2);
		dup(errFd);
		execl(CALC_PROGNAME,CALC_PROGNAME,NULL);
		fprintf(stderr,"CALC_PROGNAME failed to run \n");
		exit(EXIT_FAILURE);
  	}	

  	childId = wait(&status);
  	int file = open(OUTPUT_FILENAME,O_RDONLY,0);
  	int errorFd= open(ERROR_FILENAME, O_RDONLY,0);
  	char errorBuffer[BUFFER_LEN];

  	if (WIFEXITED(status)) 
	{
    		if (WEXITSTATUS(status) != EXIT_SUCCESS) 
      			write(clientFd, STD_ERROR_MSG,sizeof(STD_ERROR_MSG));
   			else 
			{
				memset(buffer,'\0',BUFFER_LEN);
      			int numBytes = read(file,buffer,BUFFER_LEN);
      			if (numBytes <= 0)
        			read(errorFd,buffer,BUFFER_LEN);
    		}
    		write(clientFd,buffer,BUFFER_LEN);
  	}
  	close(file);
  	close(errorFd);
  	return(EXIT_SUCCESS);
}

//  PURPOSE:  To decide a port number, either from the command line arguments
//	'argc' and 'argv[]', or by asking the user.  Returns port number.
int		getPortNum	(int	argc,
					 char*	argv[]
					)
{
  //  I.  Application validity check:

  //  II.  Get listening socket:
  int	portNum;
  if  (argc >= 2)
    portNum	= strtol(argv[1],NULL,0);
  else
  {
    char	buffer[BUFFER_LEN];
    printf("Port number to monopolize? ");
    fgets(buffer,BUFFER_LEN,stdin);
    portNum	= strtol(buffer,NULL,0);
  }
  //  III.  Finished:  
  return(portNum);
}


//  PURPOSE:  To attempt to create and return a file-descriptor for listening
//	to the OS telling this server when a client process has connect()-ed
//	to 'port'.  Returns that file-descriptor, or 'ERROR_FD' on failure.
int		getServerFileDescriptor(int		port)
{
  //  I.  Application validity check:

  //  II.  Attempt to get socket file descriptor and bind it to 'port':
  //  II.A.  Create a socket
  int socketDescriptor = socket(AF_INET, // AF_INET domain
			        SOCK_STREAM, // Reliable TCP
			        0);

  if  (socketDescriptor < 0)
  {
    perror(THIS_PROGRAM_NAME);
    return(ERROR_FD);
  }

  //  II.B.  Attempt to bind 'socketDescriptor' to 'port':
  //  II.B.1.  We'll fill in this datastruct
  struct sockaddr_in socketInfo;

  //  II.B.2.  Fill socketInfo with 0's
  memset(&socketInfo,'\0',sizeof(socketInfo));

  //  II.B.3.  Use TCP/IP:
  socketInfo.sin_family = AF_INET;

  //  II.B.4.  Tell port in network endian with htons()
  socketInfo.sin_port = htons(port);

  //  II.B.5.  Allow machine to connect to this service
  socketInfo.sin_addr.s_addr = INADDR_ANY;

  //  II.B.6.  Try to bind socket with port and other specifications
  int status = bind(socketDescriptor, // from socket()
		    		(struct sockaddr*)&socketInfo,
		    		sizeof(socketInfo)
		   			);

  if  (status < 0)
  {
    perror(THIS_PROGRAM_NAME);
    return(ERROR_FD);
  }

  //  II.B.6.  Set OS queue length:
  listen(socketDescriptor,5);

  //  III.  Finished:
  return(socketDescriptor);
}


int		main		(int	argc,
				 char*	argv[]
				)
{
  //  I.  Application validity check:

  //  II.  Do server:
  int 	      port	= getPortNum(argc,argv);
  int	      listenFd	= getServerFileDescriptor(port);
  int	      status	= EXIT_FAILURE;

  if  (listenFd >= 0)
  {
    doServer(listenFd);
    close(listenFd);
    status	= EXIT_SUCCESS;
  }

  //  III.  Finished:
  return(status);
}
