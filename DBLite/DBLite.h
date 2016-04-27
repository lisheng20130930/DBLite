#ifndef DBLITE_H
#define DBLITE_H


#include "DPRoto.h"


#define _PATH_MAX  (128)
#define _NAME_MAX  (80)
#define _DB_MAX    (32)


#define DB_FIELD_TYPE_BYTE     1
#define DB_FIELD_TYPE_SHORT    2
#define DB_FIELD_TYPE_INT      3
#define DB_FIELD_TYPE_STRZ     4
#define DB_FIELD_TYPE_OBJECT   5

#define DB_FIELD(st,name,type,i) {#name,type,(int)&(((st*)0)->name),i}
#define DB_FIELD_END {NULL,0,0,-1}


typedef struct _DB_Field
{
	char *name;
	int type;
	int offset;
	int oIdx;  // used when type is OBJECT (Object Idx in OBJ)
}DB_Field;


typedef struct _DB_Object
{
	DB_Field *pFieldTable;
	int oSize; // c-struct sizeof
}DB_Object;


typedef struct _DB_Table
{
	char *name;
	int oIdx;  // Table ROW Object Idx in OBJ
}DB_Table;


extern DB_Object  BJ[];  // Defined By DPRoto
extern DB_Table   DB[];  // Defined By DPRoto


int   DBLite_init(const char *path);
void  DBLite_loop(int loops);
void  DBLite_uint();

void* DBLite_get(char *pTblName, char *KEY);
bool  DBLite_put(char *pTblName, char *KEY, void *pObj);
void  DBLite_earse(char *pTblName, char *KEY);
int   DBLite_iterinit(char *pTblName);
char* DBLite_iternext(char *pTblName);



#endif