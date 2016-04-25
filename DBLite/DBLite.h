/* This is an example File, You can mdofiy freely According to the DPRoto */
#ifndef UDB_H
#define UDB_H


#include "DBase.h"
#include "DPRoto.h"



#define DBLite_init(v)     DBase_init(v)
#define DBLite_uint()      DBase_uint()
#define DBLite_loop(v)     DBase_loop(v)

typedef int (*pfn_DB_Callback)(void* pCaller, USR_T *usr);


USR_T* DBLite_loadUsr(char *ID);
void   DBLite_dumpUsr(USR_T *usr);
int    DBLite_visit(pfn_DB_Callback pfnCb, void* pCaller);

#endif