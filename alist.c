// dlist.c
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "alist.h"
#include "kernel.h"

list * create_list(){
	list * mylist = (list *)calloc(1, sizeof(list));
	if (mylist == NULL) {
		return NULL;
	}

	mylist->pHead = (listobj *)calloc(1, sizeof(listobj));
	if (mylist->pHead == NULL) {
		free(mylist);
		return NULL;
	}

	mylist->pTail = (listobj *)calloc(1, sizeof(listobj));
	if (mylist->pTail == NULL) {
		free(mylist->pHead);
		free(mylist);
		return NULL;
	}
	mylist->pHead->pPrevious = mylist->pHead;
	mylist->pHead->pNext = mylist->pTail;
	mylist->pTail->pPrevious = mylist->pHead;
	mylist->pTail->pNext = mylist->pTail;
	return mylist;
}


listobj * create_listobj(int num){
	listobj * myobj = (listobj *)calloc(1, sizeof(listobj));
	if (myobj == NULL)
	{
		return NULL;
	}
	myobj->nTCnt = num;
	return (myobj);
}

void insert(list * mylist, listobj * pObj){
	// insert first in list
	listobj *pMarker;
	pMarker = mylist->pHead;

	/* Position found, insert element */
	pObj->pNext = pMarker->pNext;
	pObj->pPrevious = pMarker;
	pMarker->pNext = pObj;
	pObj->pNext->pPrevious = pObj;
}

listobj * extract(listobj *pObj){
	pObj->pPrevious->pNext = pObj->pNext;
	pObj->pNext->pPrevious = pObj->pPrevious;
	pObj->pNext = pObj->pPrevious = NULL;
	return (pObj);
}

/*###################################
  ###########--FUNCTIONS--###########
  ###################################*/

void insertBeforeMarker(listobj * pObj, listobj * pMarker){
	pObj->pNext = pMarker;
	pObj->pPrevious = pMarker->pPrevious;
	pObj->pPrevious->pNext = pObj;
	pMarker->pPrevious = pObj;
}
/*Lowest nTCnt first*/
void t_insert(list * mylist, listobj * pObj){
  listobj *pMarker = mylist->pHead->pNext;
	while(pMarker != mylist->pTail){
		if(pMarker->nTCnt >= pObj->nTCnt) {
			break;
		}
		pMarker = pMarker->pNext;
	}
	insertBeforeMarker(pObj,pMarker);
}
/*Extract from head*/
listobj * t_extract(list * mylist){
	if(isEmptyList(mylist)==TRUE){
  	return FAIL;
	}
	listobj * pObj = mylist->pHead->pNext;
	pObj->pPrevious->pNext = pObj->pNext;
	pObj->pNext->pPrevious = pObj->pPrevious;
	pObj->pNext = pObj->pPrevious = NULL;
	return (pObj);
}
/*Lowest deadline first*/
void w_insert(list * mylist, listobj * pObj){
  listobj *pMarker = mylist->pHead->pNext;
	while(pMarker != mylist->pTail){
		if(pMarker->pTask->DeadLine >= pObj->pTask->DeadLine) {
			break;
		}
		pMarker = pMarker->pNext;
	}
	insertBeforeMarker(pObj,pMarker);
}
/*Extract object*/
listobj * w_extract(list * mylist, listobj * pObj){
	if(isEmptyList(mylist)==TRUE){
  	return FAIL;
	}
	pObj->pNext->pPrevious = pObj->pPrevious;
	pObj->pPrevious->pNext = pObj->pNext;
	pObj->pNext = pObj->pPrevious = NULL;
	return pObj;
}
/*Lowest deadline first*/
void r_insert(list * mylist, listobj * pObj){
  listobj * pMarker = mylist->pHead->pNext;
	while(pMarker != mylist->pTail){
		if(pMarker->pTask->DeadLine >= pObj->pTask->DeadLine){
			break;
		}
		pMarker = pMarker->pNext;
	}
	insertBeforeMarker(pObj,pMarker);
}
/*Extract from head*/
listobj * r_extract(list * mylist){
	listobj * pObj = mylist->pHead->pNext;
	pObj->pPrevious->pNext = pObj->pNext;
	pObj->pNext->pPrevious = pObj->pPrevious;
	pObj->pNext = pObj->pPrevious = NULL;
	return (pObj);
}
/*FIFO, insert from tail*/
void push(mailbox * mBox, msg * newMsg){
	msg * mBoxMsg = mBox->pHead->pNext;
	if(isFull(mBox)==TRUE){
			 msg * removedMessage = pop(mBox);
			 mBox->nMessages--;
			 mBox->nBlockedMsg -= removedMessage->Status;
			 free(removedMessage);
		 }
        newMsg->pNext = mBox->pTail;
				newMsg->pPrevious = mBox->pTail->pPrevious;
				newMsg->pPrevious->pNext = newMsg;
				mBox->pTail->pPrevious = newMsg;

				mBox->nMessages++;
				mBox->nBlockedMsg += newMsg->Status;
}
/*FIFO, extract from head*/
msg * pop(mailbox * mBox){
	if(isEmptyMailbox(mBox)==TRUE){
		return FAIL;
	}
	msg * mBoxMsg = mBox->pHead->pNext;

	mBox->pHead->pNext = mBoxMsg->pNext;
	mBox->pHead->pNext->pPrevious = mBox->pHead;
	mBoxMsg->pNext = mBoxMsg->pPrevious = NULL;
	mBox->nMessages--;
	mBox->nBlockedMsg -= mBoxMsg->Status;
	return mBoxMsg;
}
