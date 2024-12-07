#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "listaProg.h"

void createEmptyList(tList *L){
	*L=NULL;
}

bool createNode(tPosL *p){
    *p=malloc(sizeof(struct tNode));
    return *p!=NULL;
}

bool insertItem(char * d, tList *L){
    tPosL q,r;
    if(!createNode(&q)){
        return false;
    }else{
        q->data=malloc(16*sizeof(char *));
        strcpy(q->data, d);
        q->next=NULL;
    	if(*L==NULL){
            *L = q;
        }else {
     	r = last(*L);
     	r->next=q;
     	}
        return true;
    }
}

tPosL next(tPosL p, tList L){
    return(p->next);
}

tPosL first(tList L){
    return L;
}

tPosL last(tList L){
    tPosL q;
    for(q=L; q->next!=NULL;q=q->next);
    return q;
}

void deleteAtPosition(tPosL p, tList *L){
  *L=p->next;
  free(p->data);
  free(p);
}

void deleteList(tList *L){
  while(first(*L)!=NULL){
  deleteAtPosition(first(*L), L);
  }
}

tItemL getItem(tPosL p, tList L){
    return (p->data);
}
