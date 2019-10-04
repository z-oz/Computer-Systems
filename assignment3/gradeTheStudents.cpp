/*-------------------------------------------------------------------------*
 *---									---*
 *---		gradeTheStudents.cpp					---*
 *---									---*
 *---	    This file defines functions for a program that simulates	---*
 *---	having students take tests, having graders add the scores, and	---*
 *---	having a professor give the tests back.				---*
 *---									---*
 *---	    It exercises knowledge of threading, and though C++,	---*
 *---	it exercises C-style memory-management.				---*
 *---									---*
 *---	----	----	----	----	----	----	----	----	---*
 *---									---*
 *---	Version 1a					Joseph Phillips	---*
 *---									---*
 *-------------------------------------------------------------------------*/

//---									---//
//	Compile with:					 		   //
//	$ g++ gradeTheStudents.cpp -o gradeTheStudents -lpthread -g	   //
//---									---//


//---									---//
//			Inclusion of std headers:			   //
//---									---//

#include	<stdlib.h>
#include	<stdio.h>
#include	<pthread.h>
#include	<sys/types.h>
#include	<unistd.h>


//---									---//
//			Definition of constants:			   //
//---									---//

const int	NUM_TESTS_PER_STUDENT	= 4;

const int	NUM_QUESTIONS_PER_TEST	= 5;

const int	MAX_TEST_SCORE		= 100;

const int	MAX_SCORE_PER_QUESTION	= MAX_TEST_SCORE /
      					  NUM_QUESTIONS_PER_TEST;

const int	NUM_STUDENTS		= 8;

const int	NUM_TESTS		= NUM_TESTS_PER_STUDENT * NUM_STUDENTS;

const int	NUM_GRADERS		= 2;

const int	NUM_TESTS_PER_GRADER	= NUM_TESTS / NUM_GRADERS;

const int	NUM_NOTEWORTHY_TESTS	= 4;

const int	TAKEN_TEST_BUFFER_LEN	= 16;


//---									---//
//	    Declaration of struct(s) and class(es) for this program:	   //
//---									---//

#include	"Test.h"
#include	"Buffer.h"


struct		ForStudent
{
  int		studentId_;
  Buffer*	takenTestBufferPtr_;
  Buffer*	personalTestBufferPtr_;
};


struct		ForGrader
{
  Buffer*	takenTestBufferPtr_;
  Buffer*	gradedTestBufferPtr_;
};


struct		ForProfessor
{
  Buffer*	gradedTestBufferPtr_;
  Buffer*	personalTestBufferArray_;
};


//---									---//
//				Global vars:				   //
//---									---//

pthread_mutex_t	outputLock;


//---									---//
//		    Functions that do main work of program:		   //
//---									---//

//  PURPOSE:  To create and return a pointer to a Test instance for the
//	student 'studentId'.
Test*		takeTest	(int	studentId
				)
{
  Test*	toReturn	= (Test*)malloc(sizeof(Test));

  for  (int i = 0;  i < NUM_QUESTIONS_PER_TEST;  i++)
  {
    toReturn->questionScore_[i]	=
    	(MAX_SCORE_PER_QUESTION / 2) + (rand() % (MAX_SCORE_PER_QUESTION/2+1));
  }

  toReturn->total_	= 0;
  toReturn->studentId_	= studentId;

  pthread_mutex_lock(&outputLock);
  printf("Student %d: \"I got:",studentId);

  for  (int i = 0;  i < NUM_QUESTIONS_PER_TEST;  i++)
  {
    printf(" %d",toReturn->questionScore_[i]);
  }

  printf("\"\n");
  pthread_mutex_unlock(&outputLock);

  return(toReturn);
}


//  PURPOSE:  To return the appropriate commentary for test '*testPtr'.
const char*	commentOnTest	(const Test*	testPtr
				)
{
  if  (testPtr->total_ < 70)
    return("I bombed it!");

  if  (testPtr->total_ < 85)
    return("I did okay");

  return("I aced it!");
}


//  PURPOSE:  To add up the numbers in 'questionScore_[]' of '*testPtr', and
//	put the sum in 'sum_'.
void		gradeTest	(Test*	testPtr
				)
{
  int	sum	= 0;

  for  (int i = 0;  i < NUM_QUESTIONS_PER_TEST;  i++)
  {
    sum += testPtr->questionScore_[i];
  }

  testPtr->total_	= sum;

  pthread_mutex_lock(&outputLock);
  printf("Grader: \"");

  for  (int i = 0;  i < NUM_QUESTIONS_PER_TEST;  i++)
  {
    if  (i > 0)
      putchar('+');

    printf(" %d ",testPtr->questionScore_[i]);
  }

  printf(" = %d\"\n",sum);
  pthread_mutex_unlock(&outputLock);
}


//  PURPOSE:  To do the work of the students.  'vPtr' points to an instance
//	of a struct that holds:
//	(1) student id
//	(2) the address of 'takenTestBuffer'
//	    (or a reference to it, if you are comfortable with C++)
//	(3) the address of the 'studentTestsBufferArray[]' element that is
//	    the buffer that the student with this id will receive their
//	    Test* values back from the professor
//	    (or a reference to it, if you are comfortable with C++)
//
//	This function should have 2 loops.
//
//	The first loop, done 'NUM_TESTS_PER_STUDENT' times, has the student
//	take the test (see function 'takeTest()'), and then put the test
//	in the taken test buffer.
//
//	The second loop, done 'NUM_TESTS_PER_STUDENT' times, has the student
//	get the graded test out of their personal test buffer, and comment
//	on it (see function 'commentOnTest()').
//
//	Returns 'vPtr'.
void*		doStudent	(void*		vPtr
				)
{
  struct ForStudent*	forStudentPtr	= (struct ForStudent*)vPtr;

  for  (int i = 0;  i < NUM_TESTS_PER_STUDENT;  i++)
  {
    Test*	testPtr	= takeTest(forStudentPtr->studentId_);

    forStudentPtr->takenTestBufferPtr_->putIn(testPtr);
  }

  for  (int i = 0;  i < NUM_TESTS_PER_STUDENT;  i++)
  {
    Test*	testPtr	= forStudentPtr->personalTestBufferPtr_->pullOut();

    pthread_mutex_lock(&outputLock);
    printf("Student %d: \"%d, %s\"\n",
	   forStudentPtr->studentId_,testPtr->total_,commentOnTest(testPtr)
	  );
    pthread_mutex_unlock(&outputLock);

    free(testPtr);
  }

  return(vPtr);
}



//  PURPOSE:  To do the work of the students.  'vPtr' points to an instance
//	of a struct that holds:
//	(1) the address of 'takenTestBuffer'
//	    (or a reference to it, if you are comfortable with C++)
//	(2) the address of 'gradedTestBuffer'
//	    (or a reference to it, if you are comfortable with C++)
//
//	This function has a loop, done 'NUM_TESTS_PER_GRADER' times, that
//	takes a test out of the taken test buffer, grades it (see function
//	 'gradeTest()'), and puts it into the graded test buffer.
//
//	Returns 'vPtr'.
void*		doGrader	(void*		vPtr
				)
{
  struct ForGrader*	forGraderPtr	= (struct ForGrader*)vPtr;

  for  (int i = 0;  i < NUM_TESTS_PER_GRADER;  i++)
  {
    Test*	testPtr	= forGraderPtr->takenTestBufferPtr_->pullOut();

    gradeTest(testPtr);
    forGraderPtr->gradedTestBufferPtr_->putIn(testPtr);
  }

  return(vPtr);
}



//  PURPOSE:  To do the work of the students.  'vPtr' points to an instance
//	of a struct that holds:
//	(1) the address of 'gradedTestBuffer'
//	    (or a reference to it, if you are comfortable with C++)
//	(2) the 'studentTestsBufferArray[]' as a Buffer* pointer.
//	    (leave this one a pointer, even for the C++ savvy, because you
//	     will need to access it as an array)
//
//	This function has a loop, done 'NUM_TESTS' times, that takes a test
//	out of the graded test buffer, outputs the student id and grade,
//	and puts it into the 'studentTestsBufferArray[]' of that student id.
//
//	Returns 'vPtr'.
void*		doProfessor	(void*		vPtr
				)
{
  struct ForProfessor*	forProfessorPtr	= (struct ForProfessor*)vPtr;


  for  (int i = 0;  i < NUM_TESTS;  i++)
  {
    Test*	testPtr	= forProfessorPtr->gradedTestBufferPtr_->pullOut();

    pthread_mutex_lock(&outputLock);
    printf("Professor: \"Student %d: you got %d\"\n",
	   testPtr->studentId_,testPtr->total_
	  );
    pthread_mutex_unlock(&outputLock);

    forProfessorPtr->personalTestBufferArray_[testPtr->studentId_].
		putIn(testPtr);
  }

  return(vPtr);
}


//  PURPOSE:  To do the high-level work of this program.  Ignores command-line
//	arguments.  Returns 'EXIT_SUCCESS' to OS:
int		main		()
{
  Buffer	takenTestBuffer(TAKEN_TEST_BUFFER_LEN);
  Buffer	gradedTestBuffer(TAKEN_TEST_BUFFER_LEN);
  Buffer	studentTestsBufferArray[NUM_STUDENTS];
  pthread_t	studentThreadIdArray[NUM_STUDENTS];
  pthread_t	graderThreadIdArray[NUM_GRADERS];
  pthread_t	profThreadId;

  srand(getpid());

  pthread_mutex_init(&outputLock,NULL);

  //  YOUR CODE HERE TO:
  //  (1) malloc() memory for struct for doProfessor
  //  (2) filling in the member vars:
  //      (a) the address of gradedTestBuffer
  //	  (b) studentTestsBufferArray,
  //	      which because it is an array already is an address
  //  (3) starting thread 'profThreadId' to do 'doProfessor' given your struct
  struct ForProfessor*	forProfessorPtr;

  forProfessorPtr	= (struct ForProfessor*)
			  malloc(sizeof(struct ForProfessor));
  forProfessorPtr->gradedTestBufferPtr_	       = &gradedTestBuffer;
  forProfessorPtr->personalTestBufferArray_    = studentTestsBufferArray;
  pthread_create(&profThreadId,NULL,doProfessor,forProfessorPtr);


  for  (int i = 0;  i < NUM_GRADERS;  i++)
  {
    //  YOUR CODE HERE TO:
    //  (1) malloc() memory for struct for doGrader
    //  (2) filling in the member vars:
    //      (a) the address of takenTestBuffer
    //      (b) the address of gradedTestBuffer
    //  (3) starting thread 'graderThreadIdArray[i]' to do 'doGrader' given
    //	    your struct
    struct ForGrader*	forGraderPtr;

    forGraderPtr			= (struct ForGrader*)
					  malloc(sizeof(struct ForGrader));
    forGraderPtr->takenTestBufferPtr_	= &takenTestBuffer;
    forGraderPtr->gradedTestBufferPtr_	= &gradedTestBuffer;
    pthread_create(&graderThreadIdArray[i],NULL,doGrader,forGraderPtr);
  }

  for  (int i = 0;  i < NUM_STUDENTS;  i++)
  {
    //  YOUR CODE HERE TO:
    //  (1) malloc() memory for struct for doStudent
    //  (2) filling in the member vars:
    //      (a) the address of takenTestBuffer
    //      (b) the address of studentTestsBufferArray[i]
    //	    (c) the student id, which is just index 'i'
    //  (3) starting thread 'studentThreadIdArray[i]' to do 'doStudent' given
    //	    your struct
    struct ForStudent*	forStudentPtr;

    forStudentPtr			= (struct ForStudent*)
					  malloc(sizeof(struct ForStudent));
    forStudentPtr->takenTestBufferPtr_	= &takenTestBuffer;
    forStudentPtr->personalTestBufferPtr_
					= &studentTestsBufferArray[i];
    forStudentPtr->studentId_		= i;

    pthread_create(&studentThreadIdArray[i],NULL,doStudent,forStudentPtr);
  }


  void*	vPtr;

  for  (int i = 0;  i < NUM_STUDENTS;  i++)
  {
    //  YOUR CODE HERE TO:
    //  (a) wait for thread 'studentThreadIdArray[i]'
    //	(b) 'free()' the address of the struct that it gives back
    pthread_join(studentThreadIdArray[i],&vPtr);
    free(vPtr);
  }

  for  (int i = 0;  i < NUM_GRADERS;  i++)
  {
    //  YOUR CODE HERE TO:
    //  (a) wait for thread 'graderThreadIdArray[i]'
    //	(b) 'free()' the address of the struct that it gives back
    pthread_join(graderThreadIdArray[i],&vPtr);
    free(vPtr);
  }

  //  YOUR CODE HERE TO:
  //  (a) wait for thread 'profThreadId'
  //  (b) 'free()' the address of the struct that it gives back
  pthread_join(profThreadId,&vPtr);
  free(vPtr);

  pthread_mutex_destroy(&outputLock);

  //  
  return(EXIT_SUCCESS);
}
