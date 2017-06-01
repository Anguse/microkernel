#include "application.h"
#include "kernel.h"
#include "alist.h"
#include "utest.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double DEFAULT_TEMPERATURE = 27.0;
mailbox* temperature;
double setPoint;
double delta = 0;
/**
    TASKS
*/
void measure(){
    double currentTemp = getTemp();
    send_no_wait(temperature,&currentTemp);
    terminate();
}

void control(){
  double currentTemp;
  receive_wait(temperature,&currentTemp);
  double deviation = setPoint - currentTemp;
  if(deviation<=3&&deviation>=-3){
    new_measure();
    terminate();
  }
  else{
    create_task(regulate,ticks()+10);
    send_wait(temperature,&deviation);
    new_measure();
    terminate();
  }
}

void regulate(){
  double deviation;
  receive_no_wait(temperature,&deviation);
  if(deviation<0){
    delta++;
  }
  else{
    delta--;
  }
  terminate();
}

/**
    Operations
*/
void create_setPoint(double value){
  setPoint = value;
}

void new_measure(){
  create_task(measure,ticks()+10);
  create_task(control,ticks()+20);
}

double getTemp(){
  return DEFAULT_TEMPERATURE+delta;
}

/**
    Main
*/
int main(void){
  create_setPoint(20);
  init_kernel();
  new_measure();
  temperature = create_mailbox(5,sizeof(double));
  run();
}
