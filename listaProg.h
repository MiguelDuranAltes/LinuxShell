#include <string.h>
#include <stdbool.h>


typedef struct tNode* tPosL;

typedef char* tItemL;

struct tNode{
    tItemL data;
    tPosL next;
};

typedef tPosL tList;

void createEmptyList (tList *L);

bool createNode(tPosL *p);

bool insertItem(tItemL d, tList *L);

void deleteList(tList *L);

void deleteAtPosition(tPosL p, tList *L);

tPosL last(tList L);

tPosL first(tList L);

tPosL next(tPosL p, tList L);

tItemL getItem(tPosL p, tList L);

