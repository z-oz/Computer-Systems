/*-------------------------------------------------------------------------*
 *---									---*
 *---		launcher.c						---*
 *---									---*
 *---	    This file defines the program that launches the answerer	---*
 *---	and guesser programs for assignment #2.				---*
 *---									---*
 *---	----	----	----	----	----	----	----	----	---*
 *---									---*
 *---	Version 1a					Joseph Phillips	---*
 *---									---*
 *-------------------------------------------------------------------------*/

//---		Inclusion of header files				---//

#include	"assign2Headers.h"


//---		Global variables:					---//

int	answererPid;

int	guesserPid;

int	shouldRun	= 1;

//---		Global functions:					---//

void		timeUpHandler	(int		sig
				)
{
  kill(answererPid,TIME_OVER_SIGNAL);
  kill( guesserPid,TIME_OVER_SIGNAL);
  shouldRun	= 0;
}


void		childHandler	(int		sig
				)
{
  int		pid;
  int		status;

  while  ( (pid = waitpid(-1,&status,WNOHANG)) > 0);

  shouldRun	= 0;
}


int		main   		(int		argc,
				 char*		argv[]
				)
{
  //  I.  Application validity check:

  //  II.  Play game:
  //  II.A.  Install handlers:
  struct sigaction	act;

  memset(&act,'\0',sizeof(act));
  act.sa_handler	= timeUpHandler;
  sigaction(SIGALRM,&act,NULL);
  act.sa_handler	= childHandler;
  sigaction(SIGCHLD,&act,NULL);


  //  II.B.  Launch answerer:
  answererPid	= fork();

  if  (answererPid < 0)
    return(EXIT_FAILURE);

  if  (answererPid == 0)
  {
    execl(ANSWERER_PROGNAME,ANSWERER_PROGNAME,NULL);
    return(EXIT_FAILURE);
  }

  //  II.C.  Launch guesser:
  guesserPid	= fork();

  if  (guesserPid < 0)
    return(EXIT_FAILURE);

  if  (guesserPid == 0)
  {
    char	line[LINE_LEN];

    snprintf(line,LINE_LEN,"%d",answererPid);
    execl(GUESSER_PROGNAME,GUESSER_PROGNAME,line,NULL);
    return(EXIT_FAILURE);
  }


  //  II.D.  Set the alarm:
  alarm(NUM_SECONDS);


  //  II.E.  Wait for processes to finish:
  while  (shouldRun)
    sleep(1);

  //  II.F.  Wait to reap processes:
  sleep(1);
  sleep(1);
  
  //  III.  Finished:
  printf("launcher finished\n");
  return(EXIT_SUCCESS);
}
