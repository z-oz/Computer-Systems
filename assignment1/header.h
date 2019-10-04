/*-------------------------------------------------------------------------*
 *---									---*
 *---		header.h						---*
 *---									---*
 *---	----	----	----	----	----	----	----	----	---*
 *---									---*
 *---	Version 1a					Joseph Phillips	---*
 *---									---*
 *-------------------------------------------------------------------------*/

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>

// YOUR CODE HERE
void quickSort (char** array,
                int arrayLen
               );

void insertionSort (char** array, int arrayLen);
                    int partition (char** array,
                    char* pivot,
                    int lo,
                    int hi
                   );

void swap (char** array,
           int index0,
           int index1
          );

int pivot (char** array,
           int lo,
           int hi
          );

void quickSort_ (char** array,
                 int lo,
                 int hi
                );

int strLen;