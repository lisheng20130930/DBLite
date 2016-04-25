#ifndef DBASE_H
#define DBASE_H


#include "libos.h"


#define _PATH_MAX  (128)
#define _NAME_MAX  (80)
#define _DB_MAX    (32)


typedef int (*pfn_DBase_visit_callback)(void* pUsr, char* key, int ksiz, char *val, int vsiz);

int   DBase_init(const char *path);
void  DBase_uint();
void  DBase_loop(int loops);
int   DBase_put(char *dbname, char *key,  int ksiz, char* val, int vsiz);
char* DBase_get(char *dbname, char *key,  int ksiz, int *pvsiz);
int   DBase_erase(char *dbname, char *key, int ksiz);
int   DBase_visit(char *dbname, pfn_DBase_visit_callback visit, void* pUsr);


#endif