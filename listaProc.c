#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h> 
#include <sys/types.h>
#include "listaProc.h"
#include <signal.h>

#define STOPPED 0
#define RUNNING 1
#define TERMINATED 2
#define STERMINATED 3 

char *signalname(int signal){
  if(signal==0)
     return "STOPPED";
  if(signal==1)
     return "RUNNING";
  if(signal==2)
     return "TERMINATED";
  if(signal==3)
     return "STERMINATED";
  return "UNKNOWN";
}

static struct SEN{
    char* nombre;
    int senal;
} sigstrnum[]={   
	{"HUP", SIGHUP},
	{"INT", SIGINT},
	{"QUIT", SIGQUIT},
	{"ILL", SIGILL}, 
	{"TRAP", SIGTRAP},
	{"ABRT", SIGABRT},
	{"IOT", SIGIOT},
	{"BUS", SIGBUS},
	{"FPE", SIGFPE},
	{"KILL", SIGKILL},
	{"USR1", SIGUSR1},
	{"SEGV", SIGSEGV},
	{"USR2", SIGUSR2}, 
	{"PIPE", SIGPIPE},
	{"ALRM", SIGALRM},
	{"TERM", SIGTERM},
	{"CHLD", SIGCHLD},
	{"CONT", SIGCONT},
	{"STOP", SIGSTOP},
	{"TSTP", SIGTSTP}, 
	{"TTIN", SIGTTIN},
	{"TTOU", SIGTTOU},
	{"URG", SIGURG},
	{"XCPU", SIGXCPU},
	{"XFSZ", SIGXFSZ},
	{"VTALRM", SIGVTALRM},
	{"PROF", SIGPROF},
	{"WINCH", SIGWINCH}, 
	{"IO", SIGIO},
	{"SYS", SIGSYS},
/*senales que no hay en todas partes*/
#ifdef SIGPOLL
	{"POLL", SIGPOLL},
#endif
#ifdef SIGPWR
	{"PWR", SIGPWR},
#endif
#ifdef SIGEMT
	{"EMT", SIGEMT},
#endif
#ifdef SIGINFO
	{"INFO", SIGINFO},
#endif
#ifdef SIGSTKFLT
	{"STKFLT", SIGSTKFLT},
#endif
#ifdef SIGCLD
	{"CLD", SIGCLD},
#endif
#ifdef SIGLOST
	{"LOST", SIGLOST},
#endif
#ifdef SIGCANCEL
	{"CANCEL", SIGCANCEL},
#endif
#ifdef SIGTHAW
	{"THAW", SIGTHAW},
#endif
#ifdef SIGFREEZE
	{"FREEZE", SIGFREEZE},
#endif
#ifdef SIGLWP
	{"LWP", SIGLWP},
#endif
#ifdef SIGWAITING
	{"WAITING", SIGWAITING},
#endif
 	{NULL,-1},
	};    /*fin array sigstrnum */


int ValorSenal(char * sen)  /*devuelve el numero de senial a partir del nombre*/ 
{ 
  int i;
  for (i=0; sigstrnum[i].nombre!=NULL; i++)
  	if (!strcmp(sen, sigstrnum[i].nombre))
		return sigstrnum[i].senal;
  return -1;
}


char *NombreSenal(int sen)  /*devuelve el nombre senal a partir de la senal*/ 
{			/* para sitios donde no hay sig2str*/
 int i;
  for (i=0; sigstrnum[i].nombre!=NULL; i++)
  	if (sen==sigstrnum[i].senal)
		return sigstrnum[i].nombre;
 return ("SIGUNKNOWN");
}


void createEmptyListP(tListP *P){
	*P=NULL;
}

bool createNodeP(tPosP *p){
    *p=malloc(sizeof(struct tNodeP));
    return *p!=NULL;
}

tPosP buscarProceso(pid_t pid, tListP P){
    tPosP p;
    for(p=firstP(P);p!=NULL && p->data.pid!=pid;p=nextP(p,P));
    return p;
}

bool insertProcess(pid_t pid, char *time, char* cmd[], char* pri, tListP *P){
    tItemP item;
    item.pid = pid;
    item.time = time;
    item.priority=malloc(sizeof(char*));
    strcpy(item.priority,pri);
    item.cmdline=cmd;
    item.signalnum=RUNNING;
    item.signal=NULL;
    return insertItemP(item, P);
}

bool insertItemP(tItemP d, tListP *P){
    tPosP q,r;
    if(!createNodeP(&q)){
        return false;
    }else{
        q->data=d;
        q->next=NULL;
    	if(*P==NULL){
            *P = q;
        }else {
     	r = lastP(*P);
     	r->next=q;
     	}
        return true;
    }
}

tPosP nextP(tPosP p, tListP P){
    return(p->next);
}

tPosP firstP(tListP P){
    return P;
}

tPosP lastP(tListP P){
    tPosP q;
    for(q=P; q->next!=NULL;q=q->next);
    return q;
}

void deleteTerminatedProcess(tListP *P){
  tPosP p;
  for(p=firstP(*P);p!=NULL;p=nextP(p,*P)){
    updateProcess(&p->data,0);
    if(p->data.signalnum==TERMINATED)
      deleteAtPositionP(p,P);
  }
}

void deleteTerminatedSignProcess(tListP *P){
  tPosP p;
  for(p=firstP(*P);p!=NULL;p=nextP(p,*P)){
    updateProcess(&p->data,0);
    if(p->data.signalnum==STERMINATED)
      deleteAtPositionP(p,P);
  }
}

void deleteProcess(tPosP p, tListP *P){
  updateProcess(&p->data, 1);
  deleteAtPositionP(p, P);
}

void freecmd(tItemP item){
  int i;
  for(i = 0; item.cmdline[i]!=NULL;i++){
    free(item.cmdline[i]);
  }
  free(item.cmdline);
}

void deleteAtPositionP(tPosP p, tListP *P){
tPosP q;
  if (p == *P){ 
    *P = (*P)->next;
  }
  else if (p->next == NULL){
    for (q = *P; q->next->next != NULL; q = q->next);
    q->next = NULL;
  }
  else{ 
    q = p->next;
    p->data = q->data;
    p->next = q->next;
    p = q;
  }
  free(p->data.time);
  freecmd(p->data);
  free(p->data.priority);
  if(p->data.signal!=NULL)
     free(p->data.signal);
  free(p); 
}

void deleteListP(tListP *P){
  while(firstP(*P)!=NULL){
    deleteProcess(firstP(*P), P);
  }
}

tItemP getItemP(tPosP p, tListP P){ 
  return (p->data);
}

void updateProcess(tItemP *item, int flag){
   int status;
   int wp;
   wp = waitpid(item->pid, &status, flag ? 0 : WNOHANG | WUNTRACED | WCONTINUED);
   if(wp==item->pid){
      if(item->signal==NULL)
         item->signal=malloc(sizeof(int));
      if(WIFSIGNALED(status)){
        item->signalnum = STERMINATED;
        *item->signal = WTERMSIG(status);
      }
      if(WIFEXITED(status)){
        item->signalnum = TERMINATED;
        *item->signal = WEXITSTATUS(status);
        return;
      }
      if(WIFSTOPPED(status)){
        item->signalnum = STOPPED;
        *item->signal = WSTOPSIG(status);
        return;
      }
      if(WIFCONTINUED(status)){
        item->signalnum = RUNNING;
        item->signal=NULL;
        free(item->signal);
        return;
      }
   }
}

void updateItem(tItemP item, tPosP p, tListP *P){
  p->data=item;
}

void updatePriority(pid_t pid, char* pri, tListP *P){
  tPosP p;
  tItemP item;
  p=buscarProceso(pid, *P);
  if(p!=NULL){
    item=p->data;
    strcpy(item.priority,pri);
    updateItem(item, p, P);
  } 
}


void mostrarProceso(tPosP p, tListP *P){
  int i;
  tItemP item = p->data;
  updateProcess(&item,0);
  updateItem(item,p,P);
  char sig[50];
  if(item.signalnum==STOPPED || item.signalnum==STERMINATED)
    sprintf(sig," Señal: %s ", NombreSenal(*item.signal));
  else if(item.signalnum==TERMINATED)
    sprintf(sig," Valor: %d ", *item.signal);
  else 
    sprintf(sig," ");
  printf("%d\tp = %s %s %s (%s)",(int) item.pid, item.priority, item.time, sig, signalname(item.signalnum));
  for(i=0;item.cmdline[i]!=NULL;i++)
    printf(" %s",item.cmdline[i]);
  printf("\n");
}


void mostrarListaProcesos(tListP P){
  tPosP p;
  for(p=firstP(P);p!=NULL;p=nextP(p, P)){
      mostrarProceso(p, &P);
  }
}