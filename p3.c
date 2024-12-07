/*
Miguel Durán Altés: miguel.duran.altes@udc.es
Ivanna Pombo Casais: ivanna.pombo@udc.es
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <fcntl.h>
#include "listaProg.h"
#include "listaMem.h"
#include "listaProc.h"

#define MAX 1024
#define MAX_BUF 1024
#define TAMANO 2048
#define MAXNAME 1024

char **arg3;
extern char **environ;

void ProcesarEntrada(char *tr[]);
int TrocearCadena(char * cadena, char * trozos[]);
void infoDir(char *tr, int link, int ilong, int acc, int hid, int reca, int recb, char path[MAX_BUF]);
void Cmd_autores(char *tr[]);
void Create_backprocess(char **argv, char **env, char *pri);


/* Creación de las variables globales que nos van a ser útiles durante el programa*/

tList L;
tListM M;
tListP P;
int comandos;

struct CMD{
 char *nombre;
 void(*pfunc)(char **);/* *pfunc es la direccion de memoria de la funcion, puntero a una  funcion que devuelve un void;void*(*pfunc)() puntero a una funcion que devuelve un puntero a una funcion*/
 char *ayuda;
};


int terminado=0;

/* Lista de errores locales del programa */

void sys_errorcode(int errorcode){
 switch(errorcode){
  case 0:
    printf("Error de escritura en el comando, compruebe la correcta escritura con el comando ayuda\n");
    break;
  case 1:
    printf("Error formateando fecha\n");
    break;
  case 2:
    printf("El caracter no es un número\n");
    break;
  case 3:
    printf("No existe el comando nº N\n");
    break;
  case 4:
    printf("Error al intentar borrar el directorio actual\n");
    break;
  case 5: 
    printf("Error al intentar borrar carpeta raíz\n");;
    break;
  case 6:
    printf("Lista llena, no se pueden meter más elementos\n");
    break;
  case 7:
    printf("No hay procesos con ese pid\n");
    break;
   case 8:
    printf("Error al intentar asignar prioridad\n");
    break;
   case 9:
    printf("Error al obtener el pid\n");
    break;
   case 10:
    printf("Error al insertar proceso en lista\n");
    break;
  default:
    break;
} 

}

/* Función para obtener la hora actual, se va a usar para insertar los nodos en la lista de bloques de memoria */
char* horaactual(){
  int byteEsc=0;
  time_t t;
  struct tm *tm;
  char* date = malloc (50);
  t= time(NULL);
  tm = localtime(&t);
  byteEsc=strftime(date, 50, "%H:%M:%S", tm);
  if(byteEsc!=0)
     return date;
  sys_errorcode(1);
  return "NULL";
}

int BuscarVariable (char * var, char *e[])  /*busca una variable en el entorno que se le pasa como parámetro*/{
  int pos=0;
  char aux[MAX];
  
  strcpy (aux,var);
  strcat (aux,"=");
  
  while (e[pos]!=NULL){
    if (!strncmp(e[pos],aux,strlen(aux)))
      return (pos);
    else 
      pos++;
  }
  errno=ENOENT;   /*no hay tal variable*/
  return(-1);
}

int CambiarVariable(char * var, char * valor, char *e[]) /*cambia una variable en el entorno que se le pasa como parámetro*/{                                                        /*lo hace directamente, no usa putenv*/
	  int pos;
	  char *aux;
	if ((pos=BuscarVariable(var,e))==-1)
	   	 return(-1);
	 
	if ((aux=(char *)malloc(strlen(var)+strlen(valor)+2))==NULL)
		return -1;
	strcpy(aux,var);
	strcat(aux,"=");
	strcat(aux,valor);
	e[pos]=aux;
	return (pos);
}


void print_var(char *env[], char *nombre){
	int i=0;
   for(i=0;env[i]!=NULL;i++){
      printf("%p->%s[%d]=(%p) %s\n", &env[i], nombre, i, env[i], env[i]);
   }
}

void print_entorno(char *env[], char *environ[]){
   printf("main arg3: %p (almacenado en %p)\n",env, &env);
	printf("environ:   %p (almacenado en %p)\n",environ, &environ);
}

void Cmd_changevar(char *tr[]){
	char *aux=malloc(MAX);
	if(tr[0]==NULL||tr[1]==NULL||tr[2]==NULL){
      sys_errorcode(0);
      return;
   }
	if(!strcmp(tr[0], "-a")){
		CambiarVariable(tr[1], tr[2], arg3);
      return;
	}if(!strcmp(tr[0],"-e")){
      CambiarVariable(tr[1],tr[2], environ);
      return;
   }if(!strcmp(tr[0],"-p")){	
      strcpy(aux,tr[1]);
      strcat(aux,"=");
      strcat(aux,(tr[2]));
      putenv(aux);
      return;
   }
   sys_errorcode(0);

}

void Cmd_showenv(char *tr[]){
	if(tr[0]==NULL){
		print_var(arg3, "main arg3");
	}else if(!strcmp(tr[0], "-environ")){
		print_var(environ, "environ");
	}else if(!strcmp(tr[0], "-addr")){
		print_entorno(arg3,environ);
	}else
		sys_errorcode(0);
}

void print_show(int a, int e, char *value, char *arg3[], char *environ[]){

	printf("Con arg3 main: \t%s (%p) %p\n", arg3[a], arg3[a], &arg3[a]);
	printf("Con environ: \t%s (%p) %p\n", environ[e], environ[e], &environ[e]);
	printf("Con getenv: \t%s (%p9\n", value, &value);
}

void Cmd_showvar(char *tr[]){
	int a, e;
	char *value;
	if(tr[0] == NULL){
		print_var(arg3, "main arg3");
      return;
	}
	value = getenv(tr[0]);
	if(value == NULL){
		printf("La variable %s no existe\n", tr[0]);
		return;
	}
	if((a = BuscarVariable(tr[0], arg3))==-1){
		printf("Error\n");
		return;
	}
	if((e = BuscarVariable(tr[0], environ))==-1){
		printf("Error\n");
		return;
	}
	print_show(a, e, value, arg3, environ);
}
 
void Cmd_priority(char *tr[]){
	int which = PRIO_PROCESS;
	id_t pid;
	int priority;
	if(tr[0] == NULL){
		printf("La prioridad del proceso %d es %d\n", getpid(), getpriority(which, getpid()));
	}
	else if(tr[1]==NULL){
		id_t pid=(id_t) strtoul(tr[0],NULL,10);
      if(pid!=0)
		   printf("La prioridad del proceso %d es %d\n", pid, getpriority(which, pid));
      return;
	}
	pid=(id_t) strtoul(tr[0],NULL,10);
	priority = (int) strtoul(tr[1], NULL, 10);
   if(priority==0||pid==0){
      sys_errorcode(0);
      return;
   }
		setpriority(which, pid, priority);
		updatePriority(pid,tr[1],&P);
}


void Cmd_fork (char *tr[]){
	pid_t pid;
	
	if ((pid=fork())==0){
	   deleteListP(&P);
		printf ("ejecutando proceso %d\n", getpid());
	}
	else if (pid!=-1)
		waitpid (pid,NULL,0);
}

char * Ejecutable (char *s){
	char path[MAXNAME];
	static char aux2[MAXNAME];
	struct stat st;
	char *p;
	if (s==NULL || (p=getenv("PATH"))==NULL)
		return s;
	if (s[0]=='/' || !strncmp (s,"./",2) || !strncmp (s,"../",3))
        	return s;       /*is an absolute pathname*/
	strncpy (path, p, MAXNAME);
	for (p=strtok(path,":"); p!=NULL; p=strtok(NULL,":")){
       		sprintf (aux2,"%s/%s",p,s);
	  	if (lstat(aux2,&st)!=-1)
			return aux2;
	}
	return s;
}

void setpri(char *pri){
   pid_t pid;
   int priority;
   priority=strtoul(pri,NULL,10);
   if(priority==0)
      return;
   pid=getpid();
   if(setpriority(PRIO_PROCESS,pid,priority)==-1)
      sys_errorcode(8);
}

/*argv y envp deben acabar en NULL*/
int OurExecvpe(char *file, char *const argv[], char *const envp[], char* pri){
   setpri(pri);
   return (execve(Ejecutable(file),argv, envp));
}

void Cmd_execute(char *tr[]){
   char **s;
   s=environ;
   if(tr[0]==NULL){
      sys_errorcode(0);
      return;
   }
   if(OurExecvpe(tr[0],tr, s, "1") == -1)
      printf("Error en la ejecución del programa %s: %s\n",tr[0],strerror(errno));
   }


void Cmd_exe(char *tr[]){
   int i, posarg = 0, posenv = 0, poss, back=0;
   char * argv[MAX/2], **s, *envp[MAX/2], *pri ="1";
   s=environ;
   for(i=0;tr[i]!=NULL;i++){
      if((poss=BuscarVariable(tr[i],s))!=-1){
         envp[posenv]=s[poss];
         posenv++;}
      else if(tr[i][0]=='@'){ pri=strtok(tr[i],"@");}
      else if(!strcmp(tr[i],"&")){ back=1;}
      else{ argv[posarg]=tr[i];
      posarg++;}
   }
   argv[posarg+1]=NULL;
   envp[posenv+1]=NULL;
   if(!back){
      if(posenv!=0){
         if(OurExecvpe(argv[0],argv,envp, pri)==-1)
            printf("Error en la ejecución del programa %s: %s\n",argv[0],strerror(errno));}
      else
         if(OurExecvpe(argv[0],argv,s, pri)==-1)
            printf("Error en la ejecución del programa %s: %s\n",argv[0],strerror(errno));}
   else 
      if(posenv!=0) Create_backprocess(argv,envp, pri);
      else Create_backprocess(argv,s, pri);

}

int spfork() {
  pid_t pidson;
  if ((pidson= fork())==-1)
    return -1;
  if (pidson!=0)
    return pidson;
  else
    return 0; 
}

void Create_backprocess(char **argv, char **env, char *pri){
   int pidson,i;
   char *time;
   char **cmd;
   pidson = spfork();
   if(pidson==-1)
      sys_errorcode(9);
   if(pidson==0){
      if(OurExecvpe(argv[0],(argv+1),env, pri)==-1)
         printf("Error en la ejecución del programa %s: %s\n",argv[0],strerror(errno));
   }
   else{
      cmd = malloc(sizeof(char**));
      for(i=0;argv[i]!=NULL;i++){
         char *line = malloc(sizeof(char*));
         strcpy(line, argv[i]);
         cmd[i]=line;
      }
      time = horaactual();
      if(!insertProcess(pidson, time, cmd, pri, &P))
         sys_errorcode(10);
   }
}

void Cmd_listjobs(char *tr[]){
   mostrarListaProcesos(P);
}

void Cmd_deljobs(char *tr[]){
   if(tr[0]==NULL){
      mostrarListaProcesos(P);
   }
   else if(!strcmp(tr[0],"-term")){
      deleteTerminatedProcess(&P);
   }
   else if(!strcmp(tr[0],"-sig")){
      deleteTerminatedSignProcess(&P);
   }
   else sys_errorcode(0);
}

void Cmd_job(char *tr[]){
   pid_t pid;
   tPosP pos;
   int i = 0, fg = 0;
   if (tr[0]==NULL){
      mostrarListaProcesos(P);
      return;
   }
   if(!strcmp(tr[i],"-fg")){
      fg = 1;
      i++;
   }
   pid = (pid_t) strtoul(tr[i],NULL,10);
   if(pid==0){
      sys_errorcode(0);
      return;
   }
   pos=buscarProceso(pid,P);
   if(pos==NULL){
      sys_errorcode(7);
      return;
   }
   if(!fg){
      mostrarProceso(pos, &P);
      return;
   }
   deleteProcess(pos, &P);
}


/* Funciones para asignar bloques de memoria de distintos tipos */
void allocate_Malloc(char *size){
  size_t tam;
  char *time;
  void * p;
  tam=(size_t) strtoul(size,NULL,10);
  if(tam == 0){
     sys_errorcode(0);
     return;
  }
  p=malloc(tam);
  time = horaactual();
  if(insertarNodoMalloc(p,tam,time, &M))
     printf("%ld bytes asignados en %p\n", tam, p);
  else 
     sys_errorcode(6);
}

void * ObtenerMemoriaShmget (key_t clave, size_t tam){
  void * p;
  int aux,id,flags=0777;
  struct shmid_ds s;
  char* time;
  if (tam)     /*tam distito de 0 indica crear */
      flags=flags | IPC_CREAT | IPC_EXCL;
  if (clave==IPC_PRIVATE)  /*no nos vale*/
      {errno=EINVAL; return NULL;}
  if ((id=shmget(clave, tam, flags))==-1)
      return (NULL);
  if ((p=shmat(id,NULL,0))==(void*) -1){
      aux=errno;
      if (tam)
         shmctl(id,IPC_RMID,NULL);
      errno=aux;
      return (NULL);
  }
  shmctl (id,IPC_STAT,&s);
  time= horaactual();
  insertarNodoShared (p, s.shm_segsz, time,  clave, &M);
  return (p);
}

void allocate_Shared (char *tr[]){
  key_t cl;
  size_t tam;
  void *p;
  if (tr[0]==NULL) {
     printf("*****Lista de bloques shared asignados al proceso %d\n",getpid());
     showListShared(M);		
     return; 
  }
  cl=(key_t)  strtoul(tr[0],NULL,10);
  if(tr[1]==NULL)
     tam = 0;
  else {
     tam = (size_t) strtoul(tr[1],NULL,10);
     if (tam==0) {
     printf ("No se asignan bloques de 0 bytes\n");
     return;
     }
  }
  if ((p=ObtenerMemoriaShmget(cl,tam))!=NULL)
      printf ("Asignados %lu bytes en %p\n",(unsigned long) tam, p);
  else
      printf ("Imposible asignar memoria compartida clave %lu:%s\n",(unsigned long) cl,strerror(errno));
}

void * MapearFichero (char * fichero, int protection){
  int df, map=MAP_PRIVATE,modo=O_RDONLY;
  char file[MAX/2];
  struct stat s;
  void *p;
  char *time; 
  strcpy(file, fichero);
  if (protection&PROT_WRITE)
      modo=O_RDWR;
  if (stat(fichero,&s)==-1 || (df=open(file, modo))==-1)
      return NULL;
  if ((p=mmap (NULL,s.st_size, protection,map,df,0))==MAP_FAILED)
      return NULL;
  time= horaactual();
  insertarNodoMmap (p, s.st_size,time,df,file, &M); 
  return p;
}

void allocate_Mmap(char *arg[]){ 
  char *perm;
  void *p;
  int protection=0;  
  if (arg[0]==NULL){
      printf("*****Lista de bloques mmap asignados al proceso %d\n",getpid());
      showListMmap(M);
      return;
      }
  if ((perm=arg[1])!=NULL && strlen(perm)<4) {
      if (strchr(perm,'r')!=NULL) protection|=PROT_READ;
      if (strchr(perm,'w')!=NULL) protection|=PROT_WRITE;
      if (strchr(perm,'x')!=NULL) protection|=PROT_EXEC;
     }
  if ((p=MapearFichero(arg[0],protection))==NULL)
      perror ("Imposible mapear fichero");
  else
      printf ("fichero %s mapeado en %p\n", arg[0], p);
}

void Cmd_allocate (char *tr[]){
  if(tr[0] == NULL){
     printf("*****Lista de bloques asignados al proceso %d\n",getpid());
     showList(M);
     return;
     }
  if (!strcmp(tr[0],"-malloc")){
      if(tr[1] == NULL){
        printf("*****Lista de bloques malloc asignados al proceso %d\n",getpid());
  	     showListMalloc(M); 
        return;
      }
      allocate_Malloc(tr[1]);     
      return;
  }
  if (!strcmp(tr[0],"-shared")){
      allocate_Shared(tr+1);
      return;
  }
  if(!strcmp(tr[0],"-mmap")){
      allocate_Mmap(tr+1);
      return;
  }
  sys_errorcode(0);
}

/* Funciones para desasignar bloques de memoria */

void deallocate_Mmap (char* file){
  tPosM p = buscarNodoMmap(file, M);  
  if (p == NULL){
     printf("No hay mmap de ese fichero\n");
     return;
  }
  if(munmap(p->data.adress,p->data.size)==-1){
     printf("Error al borrar el Mmap %s:%s\n",file,strerror(errno));
     return;
  }
  deleteAtPositionM(p, &M);
}

void do_DeallocateDelkey (key_t clave){
  int id;

   if (clave == IPC_PRIVATE){
        printf (" delkey necesita clave_valida\n");
        return;
   }
   if ((id=shmget(clave,0,0666))==-1){
        perror ("shmget: imposible obtener memoria compartida");
        return;
   }
   if (shmctl(id,IPC_RMID,NULL)==-1)
        perror ("shmctl: imposible eliminar memoria compartida\n");
}

void deleteNodoShared(key_t clave){
  tPosM p = buscarNodoShared(clave, M);
  if (p == NULL){
     printf("No hay bloque de esa clave mapeado en el proceso\n");
     return;
  }
  do_DeallocateDelkey(p->data.key);
  deleteAtPositionM(p, &M);
}

void deallocate_Shared(char *tr){
  key_t clave = (key_t) strtoul(tr,NULL,10);
  deleteNodoShared(clave);
}

void deleteNodoMalloc(size_t size){
  tPosM pos = buscarNodoMalloc(size, M);
  tItemM item;
  if (pos == NULL){
     printf("No hay bloque de ese tamano asignado con malloc\n");
     return;
  }
  item = getItemM(pos, M);
  free(item.adress);
  deleteAtPositionM(pos, &M);
}

void deallocate_Malloc(char *size){
  size_t tam;
  tam=(size_t) strtoul(size,NULL,10);
  if (tam==0) {
      printf ("No se designan bloques de 0 bytes\n");
      return;
  }
  deleteNodoMalloc(tam);
}

void Cmd_deallocate(char *tr[]){
  if(tr[0] == NULL){
     printf("*****Lista de bloques asignados al proceso %d\n",getpid()); 
     showList(M);
     return;
     }
  if (!strcmp(tr[0],"-malloc")){
      if(tr[1] == NULL){ 
         printf("*****Lista de bloques malloc asignados al proceso %d\n",getpid());
         showListMalloc(M);
         return;
      }
      deallocate_Malloc(tr[1]);     
      return;
  }
  if (!strcmp(tr[0],"-shared")){
  	if(tr[1] == NULL){
  	   printf("*****Lista de bloques shared  asignados al proceso %d\n",getpid());
  	   showListShared(M); 
  	   return;
  	}
        deallocate_Shared(tr[1]);
        return;
  }
  if(!strcmp(tr[0],"-mmap")){
      if (tr[1] == NULL){
      printf("*****Lista de bloques mmap asignados al proceso %d\n",getpid());
         showListMmap(M);
         return;
      }
      deallocate_Mmap(tr[1]);
      return;
  }
  sys_errorcode(0);
}

void Recursiva (int n){
  char automatico[TAMANO];
  static char estatico[TAMANO];
  printf ("parametro:%3d(%p) array %p, arr estatico %p\n",n,&n,automatico, estatico);
  if (n>0)
    Recursiva(n-1);
}

void Cmd_recurse (char *tr[]){
  int n;
  if (tr[0] == NULL){
     sys_errorcode(0);
     return;
  }   
  n = strtoul(tr[0],NULL,10);
  if (n == 0){
      sys_errorcode(0);
      return;
  }
  Recursiva(n);
}

/* Función auxiliar que convierte una cadena en un void* */

void* cadtop(char *adress){
  void *p;
  sscanf(adress, "%p", &p);
  return p;
}
/*Funciones de lectura y escritura en memoria */

ssize_t LeerFichero (char *f, void *p, size_t cont)
{
   struct stat s;
   ssize_t  n;  
   int df,aux;
   if (stat (f,&s)==-1 || (df=open(f,O_RDONLY))==-1)
	return -1;     
   if (cont==-1)   /* si pasamos -1 como bytes a leer lo leemos entero*/
	cont=s.st_size;
   if ((n=read(df,p,cont))==-1){
	aux=errno;
	close(df);
	errno=aux;
	return -1;
   }
   close (df);
   return n;
}

void do_I_O_read (char *tr[])
{
   void *p;
   size_t cont=-1;
   ssize_t n;
   if (tr[0]==NULL || tr[1]==NULL){
	printf ("faltan parametros\n");
	return;
   }
   p=cadtop(tr[1]);  /*convertimos de cadena a puntero*/
   if (tr[2]!=NULL)
	cont=(size_t) atoll(tr[2]);

   if ((n=LeerFichero(tr[0],p,cont))==-1)
	perror ("Imposible leer fichero");
   else
	printf ("leidos %lld bytes de %s en %p\n",(long long) n,tr[0],p);
}

ssize_t EscribirFichero (char *f, void *p, size_t cont,int overwrite)
{
   ssize_t  n;
   int df,aux, flags=O_CREAT | O_EXCL | O_WRONLY;

   if (overwrite)
	flags=O_CREAT | O_WRONLY | O_TRUNC;

   if ((df=open(f,flags,0777))==-1)
	return -1;

   if ((n=write(df,p,cont))==-1){
	aux=errno;
	close(df);
	errno=aux;
	return -1;
   }
   close (df);
   return n;
}

void do_I_O_write(char* tr[]){
   int overwrite = 0;
   void* p;
   size_t cont = -1;
   ssize_t  n;
   if (tr[0]==NULL){
      sys_errorcode(0);
      return;
   }
   if(!strcmp (tr[0], "-o")){
      overwrite++;
   }
   if(tr[overwrite]==NULL || tr[1+overwrite]==NULL){
	   printf ("faltan parametros\n");
	   return;
   }
   p=cadtop(tr[1+overwrite]);
   if(tr[2+overwrite]!=NULL)
      cont = (size_t) strtoul(tr[2+overwrite],NULL,10);

   if ((n=EscribirFichero(tr[overwrite],p,cont, overwrite))==-1)
	perror ("Imposible escribir fichero");
   else
	printf ("escritos %lld bytes de %p en %s\n",(long long) n,p, tr[overwrite]);
}

void Cmd_io (char *tr[]){
   if(tr[0]==NULL){
      sys_errorcode(0);
      return;
   }
   if(!strcmp((tr[0]), "read")){
      do_I_O_read(tr+1);
      return;
   }
   if(!strcmp((tr[0]),"write")){
      do_I_O_write(tr+1);
      return;
   }
   sys_errorcode(0);
}


void variables(){
  auto int x=0, y=0, z=0;
  static int m=0, n=0, k=0;
  printf("Variables automáticas: %p,%p, %p\n", &x, &y, &z);
  printf("Variables estáticas: %p, %p, %p\n", &m, &n, &k);
  printf("Variables globales: %p, %p, %p\n", &L, &terminado, &comandos);
}

void direccion_funciones(){
  printf("Funciones programa: %p, %p, %p\n", Cmd_autores, ProcesarEntrada, infoDir);
  printf("Funciones librería: %p, %p, %p\n",malloc,printf, strcmp);
}

/*permite ver el mapa de memoria de uno o más procesos*/
void Do_pmap (void){  
  pid_t pid;       /*hace el pmap (o equivalente) del proceso actual*/
  char elpid[32];
  char *argv[4]={"pmap",elpid,NULL}; 
  sprintf (elpid,"%d", (int) getpid());
   if ((pid=fork())==-1){
      perror ("Imposible crear proceso");
      return;
      }
   if (pid==0){
      if (execvp(argv[0],argv)==-1)
         perror("cannot execute pmap (linux, solaris)");
         
      argv[0]="procstat"; argv[1]="vm"; argv[2]=elpid; argv[3]=NULL;  
      if (execvp(argv[0],argv)==-1)/*No hay pmap, probamos procstat FreeBSD */
         perror("cannot execute procstat (FreeBSD)");
         
      argv[0]="procmap",argv[1]=elpid;argv[2]=NULL;    
            if (execvp(argv[0],argv)==-1)  /* probamos procmap OpenBSD */
         perror("cannot execute procmap (OpenBSD)");
         
      argv[0]="vmmap"; argv[1]="-interleave"; argv[2]=elpid;argv[3]=NULL;
      if (execvp(argv[0],argv)==-1) /* probamos vmmap Mac-OS */
         perror("cannot execute vmmap (Mac-OS)");      
      exit(1);
  }
  waitpid (pid,NULL,0);
}

void Memory_options(int blocks, int funcs, int vars, int pmap){
  if(vars)
     variables();
  if(funcs)
     direccion_funciones();
  if(blocks){
     printf("*****Lista de bloques asignados al proceso %d\n",getpid()); 
     showList(M);
  } 
  if(pmap)
     Do_pmap();
}

void Cmd_memory(char *tr[]){
  int blocks = 0, funcs = 0, vars = 0, pmap = 0, i;
  if(tr[0]==NULL){
     blocks = 1;
     funcs = 1;
     vars = 1;
     Memory_options(blocks, funcs, vars, pmap);
  return;
  }
  for(i=0; tr[i]!=NULL;i++){
      if(!strcmp(tr[i], "-blocks"))
         blocks = 1;
      if(!strcmp(tr[i], "-funcs"))
         funcs = 1;
      if(!strcmp(tr[i], "-vars"))
         vars = 1;
      if(!strcmp(tr[i], "-all")){
         blocks = 1;
         funcs = 1;
         vars = 1;
      }
      if(!strcmp(tr[i], "-pmap"))
      pmap = 1;
   }
  Memory_options(blocks, funcs, vars, pmap);
}

void LlenarMemoria (void *p, size_t cont, unsigned char byte)
{
  unsigned char *arr=(unsigned char *) p;
  size_t i;

  for (i=0; i<cont;i++)
		arr[i]=byte;
}

void Cmd_memfill(char *tr[]){
  void *p;
  size_t size;
  unsigned char byte;
  if(tr[0] == NULL || tr[1] == NULL){
     sys_errorcode(0);
     return;
  }
  p=cadtop(tr[0]);
  size = (size_t) strtoul(tr[1],NULL,10);
  if(tr[2] == NULL)
     byte = 'K';
  else 
     byte = tr[2][0];
  LlenarMemoria(p,size,byte);

}

void Cmd_memdump(char *tr[]){
   int cont, i;
   void *p;
   unsigned char* byte;
   if(tr[0]==NULL){
      sys_errorcode(0);
      return;
   }
   p=cadtop(tr[0]);
   if(tr[1]==NULL)
      cont=100;
   else 
      cont = (int) strtoul(tr[1],NULL,10);
   for(i = 0; i< cont; i++){
      byte = p+i;
      printf("%c  ",*byte);
   }
   printf("\n");
}

/* Función para obtener tamaño de un fichero */

off_t TamanoFichero2(char *name){
  struct stat s; 
  if(lstat(name,&s)==-1)
     return -1;
  return s.st_size;
}

void Cmd_tamano (char *name){
  off_t tam;
  tam=TamanoFichero2(name);
      if (tam!=-1)
           printf("%lld %s\n",(long long) tam, name);
      else 
           printf("No se puede acceder a %s: %s\n",name,strerror(errno));
}

/* Funciones para convertir permisos de octal a string */

char LetraTF (mode_t m)
{
  switch (m&S_IFMT) { /*and bit a bit con los bits de formato,0170000 */
      case S_IFSOCK: return 's'; /*socket */
      case S_IFLNK: return 'l'; /*symbolic link*/
      case S_IFREG: return '-'; /* fichero normal*/ 
      case S_IFBLK: return 'b'; /*block device*/
      case S_IFDIR: return 'd'; /*directorio */ 
      case S_IFCHR: return 'c'; /*char device*/
      case S_IFIFO: return 'p'; /*pipe*/
      default: return '?'; /*desconocido, no deberia aparecer*/
     }
}

char * ConvierteModo (mode_t m)
{
    static char permisos[12];
    strcpy (permisos,"---------- ");
    
    permisos[0]=LetraTF(m);
    if (m&S_IRUSR) permisos[1]='r';    /*propietario*/
    if (m&S_IWUSR) permisos[2]='w';
    if (m&S_IXUSR) permisos[3]='x';
    if (m&S_IRGRP) permisos[4]='r';    /*grupo*/
    if (m&S_IWGRP) permisos[5]='w';
    if (m&S_IXGRP) permisos[6]='x';
    if (m&S_IROTH) permisos[7]='r';    /*resto*/
    if (m&S_IWOTH) permisos[8]='w';
    if (m&S_IXOTH) permisos[9]='x';
    if (m&S_ISUID) permisos[3]='s';    /*setuid, setgid y stickybit*/
    if (m&S_ISGID) permisos[6]='s';
    if (m&S_ISVTX) permisos[9]='t';
    
    return permisos;
}

/* Función para devolver el link de un archivo */

char * GetEnlace (char * name, char * enlace){
  int n = readlink(name, enlace, MAX); 
  if (n<0)
      return ("Link ilegible");
  enlace[n]='\0';
  return enlace;
}

/* Función para imprimir información de un arhivo/directorio */

void PrintInfoFile(char* name, int ilong, int link, int acc){
 struct stat s;
 char* permisos,*time,*user,*group,enlace[MAX];
 struct passwd *p;
 struct group *g;
 
  if (lstat(name,&s)==-1){
      printf ("Imposible acceder a %s: %s\n",name,strerror(errno));
      return; 
  }
  if(!ilong){
      printf ("%s %lld\n",name,(long long) s.st_size);
      return;
 	}
  if (acc)
      time = strtok(ctime(&s.st_atime),"\n");
  else
      time = strtok(ctime(&s.st_mtime),"\n");
  permisos = ConvierteModo(s.st_mode);
  user= ((p=getpwuid(s.st_uid))==NULL? "UNKNOWN":p->pw_name);
  group=((g=getgrgid(s.st_gid))==NULL? "UNKNOWN":g->gr_name);
  printf("%s %ld (%ld)  %s  %s  %s  %lld  ",time, (long) s.st_nlink, (long) s.st_ino, user, group, permisos, (long long) s.st_size);
  if (link && S_ISLNK(s.st_mode))
      printf ("%s -> %s\n", name, GetEnlace(name,enlace));
  else
      printf ("%s\n",name);
     
}

void Cmd_stat(char *tr[]){
 int link, ilong, acc, i;
 char path[MAX_BUF];
 getcwd(path, MAX_BUF);
  if(tr[0] == NULL)
     printf("%s\n", path);
  else{
     for(i=0;tr[i]!=NULL;i++){
         if (!(strcmp(tr[i],"-long")))
             ilong = 1;
         else if (!(strcmp(tr[i],"-acc")))
             acc = 1;
         else if (!(strcmp(tr[i],"-link")))
             link = 1;
         else break;
     }
     PrintInfoFile(tr[i],ilong,link,acc);
  }
}

/* Función para tratar un directorio de forma recursiva */

void printRecDir(int link, int ilong, int acc, int reca, int recb, int hid){
  char path[MAX_BUF];
  getcwd(path, MAX_BUF);
  struct stat s;
  DIR *d;
  struct dirent *dir;
  d=opendir(path);
  if(d==NULL) {
     printf("No se puede acceder al directorio %s: %s\n",path, strerror(errno));
     return;
  }
  while((dir = readdir (d)) != NULL){
       if(lstat(dir->d_name,&s)==-1){
          printf("No se puede acceder a %s: %s\n",dir->d_name,strerror(errno)); 
          return;
       }
       if(S_ISDIR (s.st_mode) && strcmp(dir->d_name,".") != 0 && strcmp(dir->d_name,"..") != 0)
          infoDir(dir->d_name, link, ilong, acc, hid, reca, recb, path);
       }  
}

/* Procedimiento que imprime información del directorio */

void printInfoDir(int link, int ilong, int acc, int hid){
  char path[MAX_BUF];
  getcwd(path, MAX_BUF);
  DIR *d;
  struct dirent *dir;
  d=opendir(path);
  if(d==NULL) {
     printf("No se puede acceder al directorio %s: %s\n",path, strerror(errno));
     return;
  }
  printf("**************%s\n",path);
  while((dir = readdir (d)) != NULL){ /* Sin arc, abro el directorio y voy imprimiendo los archivos */
      if(dir->d_name[0]!='.' || hid == 1)
         PrintInfoFile(dir->d_name, ilong, link, acc);
  }
  closedir(d); 
}

void infoDir(char *dir, int link, int ilong, int acc, int hid, int reca, int recb, char path[MAX_BUF]){
  if (chdir(dir)!=0){
     printf("No se puede acceder al directorio %s: %s\n",dir,strerror(errno));
     return;
  }
  if(recb==1)
     printRecDir(link, ilong, acc, reca, recb, hid);
  printInfoDir(link, ilong, acc, hid);
  if(reca==1)
     printRecDir(link, ilong, acc, reca, recb, hid);
  chdir(path);  
}

void Cmd_list(char *tr[]){
  int link=0, ilong=0, acc=0, hid=0, reca=0, recb=0,i;
  char path[MAX_BUF];
  getcwd(path, MAX_BUF);
  if (tr[0]==NULL)
     printf("%s\n", path);
  else{
     for(i = 0; tr[i]!=NULL;i++){
         if(!strcmp(tr[i], "-long"))
  	    ilong = 1;
  	 else if(!strcmp(tr[i], "-acc"))
 	    acc = 1;
  	 else if(!strcmp(tr[i], "-link"))
  	    link = 1;
  	 else if(!strcmp(tr[i], "-hid"))
  	    hid = 1;
  	 else if(!strcmp(tr[i], "-reca"))
  	    reca = 1;
  	 else if(!strcmp(tr[i], "-recb"))
  	    recb = 1;
  	 else
  	    break;
     }
     infoDir(tr[i], link, ilong, acc, hid, reca, recb, path);
   } 
} 


void Cmd_delete(char *tr[]){
 char path[MAX_BUF];
 getcwd(path, MAX_BUF);
 int i = 0;
  if(tr[0] == NULL){
    printf("%s\n", path);
    return;
    } 
  while(tr[i]!=NULL){
    if(remove(tr[i]) == -1)
       printf("No se puede borrar %s: %s\n",tr[i],strerror(errno));
    i++;
  }
}

/* Función para borrar directorios recursivamente, primero accede a ellos, después cambia el directorio de trabajo actual al que se desea eliminar y por último vacía el directorio
   Al final de la función cierra el directorio, cambia el directorio de trabajo de la shell al inicial antes de la función y borra el directorio vacío */

void deldir(char* name, char* origin){
  char path_aux[MAX_BUF];
  int j;
  char *nombre;
  DIR *d;
  struct dirent *dir;
  struct stat s;
  if (lstat(name,&s)==-1){
    printf ("Imposible acceder a %s: %s\n",name,strerror(errno));
    return;
  }    
  if (S_ISREG (s.st_mode) && remove (name) == -1){
     printf("No se puede borrar %s: %s\n",name,strerror(errno)); 
     return;
  }
  d = opendir(name);
  if (d==NULL){
     printf("No se puede acceder al directorio %s: %s\n",name, strerror(errno));
     return;
  }
  if (chdir(name)!=0){
  printf("No se puede acceder al directorio %s: %s\n",name,strerror(errno));
  return;
  }
  getcwd(path_aux, MAX_BUF);
  j=0;
  while ((dir = readdir(d)) != NULL){
     errno=0;
     nombre=dir->d_name;
     /* Evitamos borrar directorios padre, o raíz*/
     if(strcmp(nombre,".") != 0 && strcmp(nombre,"..") != 0 && strcmp(nombre,"./") != 0){
        if(remove(nombre) == -1){
         /*errno 39: Directory not empty */
          if(errno == 39)
            deldir(nombre,path_aux);
          else 
            printf("No se puede borrar %s: %s\n",nombre,strerror(errno));    
       }   
     }                
     j++;
   }
   closedir(d);
   chdir(origin);    
   if(remove(name) == -1)
      printf("No se puede borrar %s: %s\n",name,strerror(errno));
}

void Cmd_deltree(char *tr[]){
 char path[MAX_BUF];
 getcwd(path, MAX_BUF);
  if(tr[0] == NULL)
    printf("%s\n", path);
  else if (!(strcmp(tr[0],".")) || (!(strcmp(tr[0],".."))))
    sys_errorcode(4);
  else if (!(strcmp(tr[0],"./")))
    sys_errorcode(5);  
  else if(remove(tr[0]) == -1 && errno == 39)
    deldir(tr[0], path);
}

void Cmd_create(char *tr[]){
 char path[MAX_BUF];
 FILE *fp;
 getcwd(path, MAX_BUF);
  if(tr[0] == NULL)
    printf("%s\n", path);
  else if(strcmp(tr[0],"-f") == 0){
    if (tr[1] == NULL)
    	sys_errorcode(0);
    else {
    fp = fopen(tr[1],"w");
    fclose(fp);
    }
  }
  else if (mkdir(tr[0] ,S_IRWXU)!=0)
    printf("No se puede crear el directorio %s: %s\n",tr[0],strerror(errno)); 
}

void printlogins(){
   printf("miguel.duran.altes@udc.es\tivanna.pombo@udc.es\n");
}
void printnames(){
   printf("Miguel Duran Altes\tIvanna Pombo Casais\n");
}
   
void Cmd_autores(char *tr[]){
  if(tr[0]==NULL){
     printlogins();
     printnames();
     }
  else if(strcmp(tr[0], "-l")==0)
     printlogins();
  else if(strcmp(tr[0], "-n")==0)
     printnames();
  else sys_errorcode(0);
}

void Cmd_carpeta(char *tr[]){
 char path[MAX_BUF];
 getcwd(path, MAX_BUF);
  if(tr[0]== NULL)
     printf("%s\n", path);
  else if (chdir(tr[0])==0)
     printf("Se ha cambiado al directorio %s\n",tr[0]);
  else 
     printf("No se puede cambiar al directorio %s: %s\n",tr[0],strerror(errno));
}

void Cmd_pid(char *tr[]){
 if(tr[0]==NULL){
    printf("Mi pid es: %d\n", getpid());
 }
 else if(strcmp(tr[0],"-p")==0){
    printf("Mi padre tiene el pid: %d\n", getppid());
 }
 else sys_errorcode(0);
}

void Cmd_fecha(char *tr[]){
 int byteEsc=0;
 time_t t;
 struct tm *tm;
 char date[50];
 t= time(NULL);
 tm = localtime(&t);
  if(tr[0]==NULL){
     byteEsc=strftime(date, sizeof date , "%d/%m/%Y %H:%M:%S", tm);
  }else if(strcmp(tr[0], "-d")==0){
     byteEsc=strftime(date, 50, "%d/%m/%Y", tm);
  }else if(strcmp(tr[0], "-h")==0){
     byteEsc=strftime(date, 50, "%H:%M:%S", tm);
  }else {
     sys_errorcode(0);
     return;
  }
 
  if(byteEsc!=0)
     printf("%s\n", date);
  else sys_errorcode(1);

}

void Cmd_hist(char *tr[]){
 tPosL p;
 int n;
 char *num;
 if(tr[0]==NULL){
    n=1;
    for(p=first(L);p!=NULL;p=next(p, L)){
	printf("%d -> %s\n",n, getItem(p,L));
	n++;
    }
    return;
 }
 if(strcmp(tr[0], "-c")==0){
    comandos=0;
    deleteList(&L);
    return;	
 } 
 /* Si se ejecuta el comando hist -N, cortamos el '-' de la cadena y lo convertimos a entero */
 if(*tr[0]!='-'){
    sys_errorcode(2);
    return;
    }
    num=strtok(tr[0],"-");
    int y = atoi(num);
    if(y == 0){
      sys_errorcode(0);
      return;
    }
       if(y>comandos){        
       sys_errorcode(3);
       return;
       }
       n=1;
       for(p=first(L);n<=y; p=next(p, L)){
          if (p==NULL)
	      continue;
       printf("%d -> %s\n",n, getItem(p, L));
       n++;
       }	
}

void Cmd_comando(char * tr[]){
   tPosL p;
   char* trozos[MAX/2];
   int n, i ;
   if(tr[0]==NULL){
      sys_errorcode(0);
      return;
   }
   n=atoi(tr[0]);
   if(n==0){
     sys_errorcode(0);
     return;
   }
   if(n>comandos){
     sys_errorcode(3);
     return;
   }
   i=1;
   for(p=first(L);(i<=n);p=next(p,L)){
      if(i==n){
         printf("%s\n", getItem(p,L));
         TrocearCadena(getItem(p,L), trozos);
         ProcesarEntrada(trozos);
      }
      i++;
   } 
} 

void Cmd_fin(char *tr[]){
    terminado=1;/*exit(0)*/
}


void Cmd_infosis(char * tr[]) {
 struct utsname unameSys;
  if(tr[0]==NULL){
     uname(&unameSys);
     printf("%s, %s, %s, ",unameSys.sysname,unameSys.nodename,unameSys.release);
     printf("%s, %s\n",unameSys.version,unameSys.machine); 
  }
  else sys_errorcode(0);
}

void GuardaHistoric(char * entrada, char *tr[]){
   if (tr[0]!=NULL){
        insertItem(entrada, &L);
   }
}

int TrocearCadena(char * cadena, char * trozos[]){
 int i=1;
  if ((trozos[0]=strtok(cadena," \n\t"))==NULL)
      return 0;
  while ((trozos[i]=strtok(NULL," \n\t"))!=NULL)
      i++;
  return i;
}

struct CMD c[]={
 {"autores", Cmd_autores,"autores: Imprime los nombres y los logins de los autores del programa, autores -n imprime solo los nombres y autores -l imprime solo los logins\n"}, 
 {"carpeta", Cmd_carpeta,"carpeta: Imprime el nombre del directorio actual, carpeta directorio cambia el directorio de trabajo actual de la terminal a directorio\n"}, 
 {"pid", Cmd_pid, "pid: Imprime el pid del proceso que se está ejecutando en el terminal, pid -p imprime el pid de los padres del proceso que se está ejecutando en terminal\n"},
 {"fin", Cmd_fin, "fin: Cierra la terminal\n"}, {"salir", Cmd_fin, "salir: Cierra la terminal\n"}, {"bye", Cmd_fin,"bye: Cierra la terminal\n"}, 
 {"fecha", Cmd_fecha, "fecha: Sin argumentos imrpime la fecha y la hora actual, fecha -d imprime la fecha actual con el formato DD/MM/YYYY y fecha -h imprime la hora en formato hh:mm:ss\n"},
 {"infosis",Cmd_infosis, "infosis: Imprime información de la máquina que está ejecutando la terminal\n"}, 
 {"hist", Cmd_hist,"hist: Muestra en pantalla el historial de comandos ejecutados en esta terminal. hist -c vacía la lista del historial de comandos y hist -N imprime los primeros N comandos\n"},
 {"comando", Cmd_comando, "comando N: repite el comando número N (del historial de comandos)\n"}, 
 {"create",Cmd_create, "create: Crea directorios o ficheros (con la opción -f) con el nombre indicado\n"}, 
 {"stat",Cmd_stat, "stat: Proporciona información sobre archivo o directorios. Opciones:\nstat __ devuelve el nombre del archivo/directorio y su tamaño\n -long proporciona información más detallada\n-acc proporciona el último tiempo de acceso, -link proporciona el nº de links (ambas opciones dependen de -long)\n"}, 
 {"delete", Cmd_delete, "Borra archivos y/o directorios vacíos\n"},
 {"list", Cmd_list, "list: Lista el contenido de los directorios. Opciones:\nlist __ devuelve el contenido del directorio y su tamaño\n -long, -acc, -link funcionan igual que en stat\n-reca imprime primero el contenido del directorio y después recursivamente sus subdirectorios, -recb al revés\n-hid mostraría también archivos ocultos\n"}, 
 {"deltree", Cmd_deltree, "deltree: Elimina archivos y/o directorios no vacíos recursivamente\n"}, 
 {"allocate", Cmd_allocate, "allocate: Asigna un bloque de memoria y lo añade a la lista de los bloques de memoria. Opciones:\n-malloc [tam]: asigna un bloque malloc de tamaño tam\n -shared [key] [tam]: asigna el bloque de memoria compartido por la clave key\n-mmap [fich] [perm]: mapea el fichero fich, perm son los permisos\n"}, 
 {"deallocate", Cmd_deallocate, "deallocate: Desasigna un bloque de memoria y lo borra de la lista de bloques de memoria. Opciones:\n-malloc [tam]\n-shared[key]-mmap [fich]\n"}, 
 {"recurse", Cmd_recurse, "recurse: Ejecuta uan función recursiva\n"}, {"i-o", Cmd_io,"i-o [read|write] [-o] (sobrescritura) fiche addr cont 	Lee (escribe) cont bytes desde (a) fiche a (desde) addr\n"}, 
 {"memory", Cmd_memory, "memory: Muestra información de la memoria del proceso. Opciones:\n-blocks: muestra la lista de los bloques de memoria asignados\nfuncs: muestra las direcciones de las funciones\nvars: muestra las direcciones de las variables\nall: realiza las 3 anteriores\npmap: muestra la salida del comando pmap\n"}, 
 {"memfill", Cmd_memfill, "memfill adress desp byte: Rellena la memoria a partir de adress con byte hasta desp\n"}, 
 {"memdump", Cmd_memdump, "memdump adress desp: Muestra en pantalla los contenidos (desp bytes) de la posición de memoria adress\n"}, 
 {"priority", Cmd_priority, ""}, 
 {"showvar", Cmd_showvar, "showvar Muestra el valor y las direcciones de una variable de entorno\n"}, 
 {"changevar", Cmd_changevar, "changevar[-a,-e,-p] var valor Cambia el valor de una variable de entorno\n -a: accede mediante el tercer argumento del main\n-e:accede mediante environ\n-p: accede  mediante putenv\n"}, 
 {"showenv", Cmd_showenv, "showenv[-environ|-addr] Muestra el entorno del proceso\n-environ: muestra el entorno del proceso a través de la variable externa environ\n-addr: muestra el valor de environ y el tercer argumento de main, además de las direcciones en las que se almacenan\n"},
 {"execute",Cmd_execute, "execute prog args ... Ejecuta, sin crear proceso, prog con argumentos\n"},
 {"fork", Cmd_fork, "fork: Hace una llamada fork para crear un proceso\n"},
 {"job", Cmd_job, "job [-fg] pid Muestra informacion del proceso pid. -fg lo pasa a primer plano\n"},
 {"listjobs", Cmd_listjobs, "listjobs 	Lista los procesos en segundo plano"},
 {"deljobs", Cmd_deljobs, "deljobs [-term][-sig] Elimina los propcesos terminados o terminados por senal de la lista de procesos en s.p.\n"},
 {NULL, NULL, NULL},
};

void ProcesarEntrada(char *tr[]){
 int i;
  if(tr[0]==NULL){
     return;}
  if (!strcmp(tr[0],"ayuda")){
    if(tr[1]==NULL){
       printf("ayuda 'cmd' donde cmd es uno de los siguientes comandos:\nautores\tcarpeta\tfecha\thist\tpid\tcomando\n");
       printf("create\tstat\tlist\tdelete\tdeltree\nbye\tfin\tsalir\tinfosis\tayuda\n");
       return;
       }
    if(!strcmp(tr[1],"ayuda")){
       printf("ayuda [cmd]\tMuestra ayuda sobre los comandos\n");
       return;
       }
    for(i=0;c[i].nombre!=NULL;i++){
         if(!strcmp(tr[1], c[i].nombre)){
            printf("%s",c[i].ayuda);
            return;}
       }
     printf("Command not found\n");
     return;
  }
  else {
     for(i=0;c[i].nombre!=NULL;i++){
         if(!strcmp(tr[0], c[i].nombre)){
            ((*c[i].pfunc)((tr+1)));
            return;}
       }
      Cmd_exe(tr);
   }

     
}

void printPrompt() {
    printf("C:\\>");
}

int main(int argc, char *argv[], char *envp[]){
   char entrada[MAX];
   char *trozos[MAX/2];
   arg3 = envp;
   createEmptyList(&L);
   createEmptyListM(&M);
   createEmptyListP(&P);
   comandos=0;
    while(terminado!=1){/*1*/
        printPrompt();
        fgets(entrada, MAX, stdin);
        if(TrocearCadena(entrada, trozos)==0)
        continue;
        comandos++;
        GuardaHistoric(entrada,trozos);
        ProcesarEntrada(trozos);
    }
   deleteList(&L);
   deleteListM(&M);
   deleteListP(&P);
}

