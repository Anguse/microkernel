
#include "utest.h"

int isEqualPointer(void * comp1, void * comp2)
{
	if (comp1 == comp2)
		return 1;
	else
		return 0;
}

int isNotEqualPointer(void * comp1, void * comp2)
{
	if (comp1 != comp2)
		return 1;
	else
		return 0;
}


int isEqualInt(int comp1, int comp2)
{
	if (comp1 == comp2)
		return 1;
	else
		return 0;
}

int isNotEqualInt(int comp1, int comp2)
{
	if (comp1 == comp2)
		return 1;
	else
		return 0;
}

int isEmptyList(list * this_list)
{
	if (isEqualPointer(this_list->pHead->pNext, this_list->pTail) &&
		isEqualPointer(this_list->pTail->pPrevious, this_list->pHead) &&
		isEqualPointer(this_list->pTail->pNext, this_list->pTail) &&
		isEqualPointer(this_list->pHead->pPrevious, this_list->pHead))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
