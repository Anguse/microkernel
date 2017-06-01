/*A test for the send_wait function*/

#include "kernel.h"
#include "alist.h"
#include "utest.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

mailbox* mBox;
int testValue = 0;
int nVar = 1;
int ta_emot = 16;

void task1(void)
{
  int data = 1;
  receive_wait(mBox,&data);
  terminate();
}

void task2(void)
{
  send_wait(mBox,&ta_emot);
  terminate();
}

int main(void){
      init_kernel();
      create_task(task1,10);
      create_task(task2,20);
      mBox = create_mailbox(10,sizeof(int));
      run();

}
