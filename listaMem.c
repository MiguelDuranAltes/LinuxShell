#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "listaMem.h"

void createEmptyListM(tListM *M){
	*M=NULL;
}

bool createNodeM(tPosM *p){
    *p=malloc(sizeof(struct tNodeM));
    return *p!=NULL;
}

bool insertItemM(tItemM d, tListM *M){
    tPosM q,r;
    if(!createNodeM(&q)){
        return false;
    }else{
        q->data=d;
        q->next=NULL;
    	if(*M==NULL){
            *M = q;
        }else {
     	r = lastM(*M);
     	r->next=q;
     	}
        return true;
    }
}

bool insertarNodoMalloc(void *p, size_t size, char *time, tListM *M){
    tItemM item;
    item.adress=p;
    item.size=size; 
    item.time=time;
    item.type="malloc";
    item.key=-1;
    item.name="NULL";
    item.df=-1;
    return insertItemM(item, M);
}

bool insertarNodoShared(void *p, size_t size, char* time, key_t clave, tListM *M){
    tItemM item;
    item.adress=p;
    item.size=size;
    item.time=time;
    item.type="shared";
    item.key= clave;
    item.name="NULL";
    item.df=-1;
    return insertItemM(item, M);
}

bool insertarNodoMmap(void *p, size_t size, char* time, int df, char* fichero, tListM *M){
    tItemM item;
    item.adress=p;
    item.size=size;
    item.time=time;
    item.type="mmap";
    item.key= -1;
    item.name = malloc(16*sizeof(char *));
    strcpy(item.name, fichero);
    item.df=df;
    return insertItemM(item, M);
}

tPosM buscarNodoMalloc(size_t size, tListM M){
    tPosM p;
    tItemM item;
    for(p=firstM(M);p!=NULL;p=nextM(p,M)){
        item=p->data;
        if(!strcmp(item.type,"malloc") && item.size==size)
           break;
    }
    return p;
}

tPosM buscarNodoShared(key_t clave, tListM M){
    tPosM p;
    tItemM item;
    for(p=firstM(M);p!=NULL;p=nextM(p,M)){
        item=p->data;
        if(!strcmp(item.type,"shared") && item.key==clave)
           break;
    }
    return p;
}

tPosM buscarNodoMmap(char* file, tListM M){
    tPosM p;
    tItemM item;
    for(p=firstM(M);p!=NULL;p=nextM(p,M)){
        item=p->data;
        if(!strcmp(item.type,"mmap") && !strcmp(item.name, file))
           break;
    }
    return p;
}


void showPosition(tPosM p, tListM M){
    if(!strcmp(p->data.type,"malloc")){
       printf("%p\t%ld B\t%s\t%s\n", p->data.adress, p->data.size, p->data.time, p->data.type);
       return;
    }
    if(!strcmp(p->data.type,"shared")){
       printf("%p\t%ld B\t%s\t%s\t%d\n", p->data.adress, p->data.size, p->data.time, p->data.type,(int) p->data.key);
       return;
    }
    if(!strcmp(p->data.type,"mmap")){
       printf("%p\t%ld B\t%s\t%s\t%s\t%d\n", p->data.adress, p->data.size, p->data.time, p->data.type, p->data.name, p->data.df);
       return;
    }
    
}

void showList(tListM M){
    tPosM p;
    for(p=firstM(M);p!=NULL;p=nextM(p,M)) {
    showPosition(p, M);
    }
}

void showListMalloc(tListM M){
    tPosM p;
    for(p=firstM(M);p!=NULL;p=nextM(p,M)){
        if(!strcmp(p->data.type,"malloc"))
           printf("%p\t%ld B\t%s\t%s\n", p->data.adress, p->data.size, p->data.time, p->data.type);
    }
}

void showListShared(tListM M){
    tPosM p;
    for(p=firstM(M);p!=NULL;p=nextM(p,M)){
        if(!strcmp(p->data.type,"shared"))
           printf("%p\t%ld B\t%s\t%s\t%d\n", p->data.adress, p->data.size, p->data.time, p->data.type,(int) p->data.key);
    }
}
/* Imprime el name hasta que pongo allocate -mmap intuyo que apunta a fichero y en ese caso es null*/
void showListMmap(tListM M){
    tPosM p;
    for(p=firstM(M);p!=NULL;p=nextM(p,M)){
        if(!strcmp(p->data.type,"mmap"))
        printf("%p\t%ld B\t%s\t%s\t%s\t%d\n", p->data.adress, p->data.size, p->data.time, p->data.type, p->data.name, p->data.df);
    }
}    
    

tPosM nextM(tPosM p, tListM M){
    return(p->next);
}

tPosM firstM(tListM M){
    return M;
}

tPosM lastM(tListM M){
    tPosM q;
    for(q=M; q->next!=NULL;q=q->next);
    return q;
}

void deleteAtPositionM(tPosM p, tListM *M){
tPosM q;
  if (p == *M){ 
    *M = (*M)->next;
  }
  else if (p->next == NULL){
    for (q = *M; q->next->next != NULL; q = q->next);
    q->next = NULL;
  }
  else{ 
    q = p->next;
    p->data = q->data;
    p->next = q->next;
    p = q;
  }

  if(!strcmp(p->data.type,"mmap"))
    free(p->data.name);
  free(p->data.time);
  free(p); 
}

void deleteListM(tListM *M){
  while(firstM(*M)!=NULL){
  deleteAtPositionM(firstM(*M), M);
  }
}

tItemM getItemM(tPosM p, tListM M){
    return (p->data);
}
