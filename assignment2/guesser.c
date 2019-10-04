/*-------------------------------------------------------------------------*
 *---									---*
 *---		answerer.c						---*
 *---									---*
 *---	    This file defines the answerer program for assignment #2.	---*
 *---									---*
 *---	----	----	----	----	----	----	----	----	---*
 *---									---*
 *---	Version 1a					Joseph Phillips	---*
 *---									---*
 *-------------------------------------------------------------------------*/

//---		Inclusion of header files				---//

#include	"assign2Headers.h"


//---		Global variables:					---//

int		shouldRun	= 1;



//---		Global functions:					---//

void		timeUpHandler	(int 		sig
				)
{
  printf("\nOh no!  The time is up!\n");
  shouldRun	= 0;
}


void		successHandler	(int		sig
				)
{
  printf("\nCongratulations!  You found it!\n");
  shouldRun	= 0;
}


void		answerHandler	(int		sig
				)
{
  if  (sig == CORRECT_SIGNAL)
    printf("Yay!  That was right!\n");
  else
    printf("Oops!  That was wrong.  Please restart from the beginning.\n");
}


int		main		(int		argc,
				 char*		argv[]
				)
{
  //  I.  Application validity check:
  if  (argc < 2)
  {
    fprintf(stderr,"Usage:\t%s <answererPid>\n",GUESSER_PROGNAME);
    return(EXIT_FAILURE);
  }

  int	answererPid	= strtol(argv[1],NULL,10);

  //  II.  Do program:
  //  II.A.  Install signal handlers:
  struct sigaction	act;

  memset(&act,'\0',sizeof(act));
  act.sa_handler	= timeUpHandler;
  sigaction(TIME_OVER_SIGNAL,&act,NULL);

  act.sa_handler	= successHandler;
  sigaction(WIN_SIGNAL,&act,NULL);

  act.sa_handler	= answerHandler;
  sigaction(  CORRECT_SIGNAL,&act,NULL);
  sigaction(INCORRECT_SIGNAL,&act,NULL);

  //  II.B.  Play game:
  while  (shouldRun)
  {
    char	line[LINE_LEN];
    int		guess;

    do
    {
      printf("What would you like your next guess to be: 0 or 1? ");
      fgets(line,LINE_LEN,stdin);
      guess = strtol(line,NULL,10);
    }
    while  ( (guess < 0) || (guess > 1) );

    int		toSend	 = (guess == 1) ? ONE_SIGNAL : ZERO_SIGNAL;

    kill(answererPid,toSend);

    sleep(1);
  }

  //  III.  Finished:
  printf("guesser finished\n");
  return(EXIT_SUCCESS);
}
