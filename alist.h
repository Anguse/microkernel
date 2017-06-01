#ifndef _ALIST_H_
#define _ALIST_H_
#include "kernel.h"

list * create_list();
listobj * create_listobj(int num);
void insert(list * mylist, listobj * pObj);
listobj * extract(listobj * pObj);


//Function prototypes
void insertBeforeMarker(listobj * pObj, listobj * pMarker);
msg * extractLast(mailbox * mBox);

void r_insert(list * mylist, listobj * pObj);
void t_insert(list * mylist, listobj * pObj);
void w_insert(list * mylist, listobj * pObj);
listobj * t_extract(list * mylist);
listobj * r_extract(list * mylist);
listobj * w_extract(list * mylist, listobj * pObj);
msg * pop(mailbox * mBox);
void push(mailbox * mBox, msg * newMsg);
#endif
