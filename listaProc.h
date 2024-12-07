#include <string.h>
#include <stdbool.h>

typedef struct tNodeP* tPosP;

typedef struct tItemP{
    pid_t pid;
    char** cmdline;
    char* time;
    int *signal;
    int signalnum;
    char* priority;
}tItemP;

struct tNodeP{
    tItemP data;
    tPosP next;
};

typedef tPosP tListP;

void createEmptyListP(tListP *P);

bool createNodeP(tPosP *p);

bool insertProcess(pid_t pid, char *time, char* cmd[], char* pri, tListP *P);

bool insertItemP(tItemP d, tListP *P);

void showListP(tListP P);

void deleteListP(tListP *P);

void deleteAtPositionP(tPosP p, tListP *P);

void deleteTerminatedProcess(tListP *P);

void deleteTerminatedSignProcess(tListP *P);

void deleteProcess(tPosP p, tListP *P);

void updateProcess(tItemP *item, int flag);

void updateList(tListP *P);

void updatePriority(pid_t pid, char *pri, tListP *P);

tPosP buscarProceso(pid_t pid, tListP P);

tPosP lastP(tListP P);

tPosP firstP(tListP P);

tPosP nextP(tPosP p, tListP P);

tItemP getItemP(tPosP p, tListP P);

void mostrarProceso(tPosP p, tListP *P);

void mostrarListaProcesos(tListP P);