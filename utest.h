#ifndef _UTEST_H_
#define _UTEST_H_

#include <assert.h>
#include "kernel.h"


int isEqualPointer(void * comp1, void * comp2);
int isNotEqualPointer(void * comp1, void * comp2);
int isEqualInt(int comp1, int comp2);
int isNotEqualInt(int comp1, int comp2);
int isEmptyList(list * this_list);
int exist_list(list * list);

#endif
