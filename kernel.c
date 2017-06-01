#include "kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "alist.h"

//Globals
TCB * Running;
uint tick_counter;
int mode;// 1=RUNNING, 0=INIT
list * ready_list;
list * timer_list;
list * waiting_list;


/*###################################
  ##########--FUNCTIONS--############
  ###################################*/

void idle(void){
  while(1){
    SaveContext();
    TimerInt();
    LoadContext();
  }
}

listobj * GetRdyListObj(){
  return ready_list->pHead->pNext;
}

listobj * GetTimerListObj(){
  return timer_list->pHead->pNext;
}

listobj * GetWaitListObj(){
  return waiting_list->pHead->pNext;
}

void updateRunningTask(){
  Running = ready_list->pHead->pNext->pTask;
}

msg * allocMsg(){
  msg * newMsg = (msg*)calloc(1,sizeof(msg));
  if(newMsg == NULL){
    return NULL;
  }
  newMsg->pBlock = NULL;
  return newMsg;
}

mailbox * allocmBox(){
  mailbox * mBox = (mailbox*)calloc(1,sizeof(mailbox));
  if(mBox == NULL){
    return NULL;
  }
  mBox->pHead = allocMsg();
  mBox->pTail = allocMsg();
  if(mBox->pHead == NULL){
    free(mBox);
    return NULL;
  }
  if(mBox->pTail == NULL){
    free(mBox->pHead);
    free(mBox);
    return NULL;
  }
  return mBox;
}

int isEmptyMailbox(mailbox * mBox){
  if(mBox->pHead->pNext==mBox->pTail){
    return TRUE;
  }
  return FALSE;
}

int isFull(mailbox * mBox){
  if(mBox->nMessages>=mBox->nMaxMessages){
    return TRUE;
  }
  return FALSE;
}

int isEmptyList(list * mylist){
  if(mylist->pHead->pNext==mylist->pTail){
    return TRUE;
  }
  return FALSE;
}

/*###################################
  ######--TASK ADMINISTRATION--######
  ###################################*/

exception init_kernel(){
  tick_counter = 0;
  ready_list = create_list();
  timer_list = create_list();
  waiting_list = create_list();
  if(ready_list == NULL){
    return FAIL;
  }
  if(waiting_list == NULL){
    free(ready_list);
    return FAIL;
  }
  if(timer_list == NULL){
    free(ready_list);
    free(waiting_list);
    return FAIL;
  }
  mode = INIT;
  create_task(idle,UINT_MAX);
  updateRunningTask();
  return OK;
}

exception create_task(void(*task_body)(),uint deadline){
  volatile int first = TRUE;
  listobj * newObj = create_listobj(0);
  TCB * newTCB = (TCB *)calloc(1,sizeof(TCB));
	if (newObj == NULL) {
    return FAIL;
	}
  if(newTCB == NULL){
    free(newObj);
    return FAIL;
  }
  newTCB->DeadLine = deadline;
  newTCB->PC = task_body;
  newTCB->SPSR = 0;
  newTCB->SP = &(newTCB->StackSeg[STACK_SIZE-1]);
  newObj->pTask = newTCB;
  if(mode==INIT){
    r_insert(ready_list, newObj);
    updateRunningTask();
    return OK;
  }
  else{
    isr_off();
    SaveContext();
    if(first){
      first = FALSE;
      r_insert(ready_list, newObj);
      updateRunningTask();
      LoadContext();
    }
  }
  return OK;
}

void run(){
  mode = RUNNING;
  updateRunningTask();
  isr_on();
  LoadContext();
}

void terminate(){
  listobj * terminatedTask = r_extract(ready_list);
  free(terminatedTask->pTask);
  free(terminatedTask);
  updateRunningTask();
  LoadContext();
}

/*###################################
  ########--COMMUNICATION--##########
  ###################################*/

mailbox*	create_mailbox( uint nMaxMessages, uint nDataSize ){
  mailbox * mBox =  allocmBox();
  if(mBox == NULL){
    return NULL;
  }
  mBox->pHead->pNext = mBox->pTail;
  mBox->pHead->pPrevious = mBox->pHead;
  mBox->pTail->pNext = mBox->pTail;
  mBox->pTail->pPrevious = mBox->pHead;

  mBox->nMaxMessages = nMaxMessages;
  mBox->nDataSize = nDataSize;
  return mBox;
}

exception remove_mailbox(mailbox *mBox){
  if(isEmptyMailbox(mBox)==TRUE){
    free(mBox);
    return OK;
  }
  else{
    return NOT_EMPTY;
  }
}

exception send_wait(mailbox * mBox, void * Data){
  volatile int first = TRUE;
  isr_off();
  SaveContext();
  listobj* rdyListObj;
  msg* mBoxMsg = mBox->pHead->pNext;
  if(first){
    first = FALSE;
    if(mBoxMsg->Status==RECEIVER){
      memcpy(mBoxMsg->pData,Data,mBox->nDataSize);
      pop(mBox);
      r_insert(ready_list,w_extract(waiting_list,mBoxMsg->pBlock));
      updateRunningTask();
    }
    else{
      msg * newMsg = allocMsg();
      if(newMsg == NULL){
        return FAIL; //Alternative return value?
      }
      newMsg->pData = Data;
      newMsg->pBlock = rdyListObj;
      newMsg->Status = SENDER;
      rdyListObj = GetRdyListObj();
      rdyListObj->pMessage = newMsg;
      push(mBox,newMsg);
      w_insert(waiting_list,r_extract(ready_list));
      updateRunningTask();
    }
    LoadContext();
  }
  else{
    if(tick_counter>=Running->DeadLine){
      isr_off();
      mBoxMsg = pop(mBox);
      free(mBoxMsg);
      isr_on();
      return DEADLINE_REACHED;
    }
  }
  return OK;
}

exception receive_wait(mailbox* mBox, void* Data){
  volatile int first = TRUE;
  isr_off();
  SaveContext();
  msg* mBoxMsg = mBox->pHead->pNext;
  listobj* rdyListObj = GetRdyListObj();
  if(first){
    first = FALSE;
    if(mBoxMsg->Status==SENDER){
      memcpy(Data,mBoxMsg->pData,mBox->nDataSize);
      mBoxMsg = pop(mBox);
      if(mBoxMsg->pBlock!=NULL){
        r_insert(ready_list,w_extract(waiting_list,mBoxMsg->pBlock));
        updateRunningTask();
      }
      else{
        free(mBoxMsg->pData);
      }
    }
    else{
      msg * newMsg = allocMsg();
      if(newMsg == NULL){
        return FAIL;//Other return value?
      }
      newMsg->pData = Data;
      newMsg->pBlock = rdyListObj;
      newMsg->Status = RECEIVER;
      rdyListObj->pMessage = newMsg;
      push(mBox,newMsg);
      w_insert(waiting_list,r_extract(ready_list));
      updateRunningTask();
      rdyListObj = GetRdyListObj();
    }
    LoadContext();
  }
  else{
    if(tick_counter>=Running->DeadLine){
      isr_off();
      mBoxMsg = pop(mBox);
      free(mBoxMsg);
      rdyListObj->pMessage = NULL;
      isr_on();
      return DEADLINE_REACHED;
    }
  }
  return OK;
}

exception send_no_wait(mailbox * mBox, void* Data){
  volatile int first = TRUE;
  isr_off();
  SaveContext();
  msg * mBoxMsg = mBox->pHead->pNext;
  listobj * rdyListObj = GetRdyListObj();
  if(first){
    first = FALSE;
    if(mBoxMsg->Status==RECEIVER){
      memcpy(mBoxMsg->pData,Data,mBox->nDataSize);
      mBoxMsg = pop(mBox);
      r_insert(ready_list,w_extract(waiting_list,mBoxMsg->pBlock));
      updateRunningTask();
      LoadContext();
    }
    else{
      msg * newMsg = allocMsg();
      if(newMsg == NULL){
        return FAIL; //Alternative return?
      }
      newMsg->pData = Data;
      newMsg->Status = SENDER;
      rdyListObj->pMessage = newMsg;
      if(mBox->nMessages >= mBox->nMaxMessages){
        pop(mBox);
      }
      push(mBox,newMsg);
    }
   }
  return OK;
}

exception receive_no_wait(mailbox * mBox, void * Data){
  volatile int first = TRUE;
  isr_off();
  SaveContext();
  msg * mBoxMsg = mBox->pHead->pNext;
  if(first){
    first = FALSE;
    if(mBoxMsg->Status==SENDER){
      memcpy(Data,mBoxMsg->pData,mBox->nDataSize);
      pop(mBox);
      if(mBoxMsg->pBlock != NULL){
        r_insert(ready_list,w_extract(waiting_list,mBoxMsg->pBlock));
      }
      else{
        mBoxMsg->pData = NULL;
      }
      updateRunningTask();
    }
    LoadContext();
  }
  return OK;
}

/*###################################
  ############--TIMING--#############
  ###################################*/

exception wait(uint nTicks){
  volatile int first = TRUE;
  isr_off();
  SaveContext();
  listobj * rdyListObj = GetRdyListObj();
  if(first){
    first = FALSE;
    rdyListObj->nTCnt = tick_counter + nTicks;
    t_insert(timer_list,r_extract(ready_list));
    updateRunningTask();
    LoadContext();
  }
  else{
    if(tick_counter >= Running->DeadLine){
      return DEADLINE_REACHED;
    }
  }
  return OK;
}

void set_ticks(uint nTicks){
  tick_counter = nTicks;
}

uint ticks(void){
  return tick_counter;
}

uint deadline(void){
  return Running->DeadLine;
}

void set_deadline(uint deadline){
  isr_off();
  volatile int first = TRUE;
  SaveContext();
  if(first){
    first = FALSE;
    Running->DeadLine = deadline;
    r_insert(ready_list,r_extract(ready_list));
    LoadContext();
  }
}

void TimerInt(void){
  tick_counter++;
  listobj * timerListObj = GetTimerListObj();
  listobj * waitListObj = GetWaitListObj();

    while(timerListObj->nTCnt <= ticks()&&timerListObj!=timer_list->pTail){
        r_insert(ready_list,t_extract(timer_list));
        updateRunningTask();
        timerListObj = GetTimerListObj();
    }
    while(waitListObj->pTask->DeadLine <= ticks()&&waitListObj!=waiting_list->pTail){
        r_insert(ready_list,w_extract(waiting_list,waitListObj));
        updateRunningTask();
        waitListObj = GetWaitListObj();
    }
}
