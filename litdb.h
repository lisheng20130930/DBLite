#ifndef LITEDB_H
#define LITEDB_H


#define _FIELD_TYPE_BYTE     1
#define _FIELD_TYPE_SHORT    2
#define _FIELD_TYPE_INT      3
#define _FIELD_TYPE_STRZ     4
#define _FIELD_TYPE_OBJECT   5
#define _FIELD_TYPE_FIXL     6


#define _FIELD(st,name,type,i) {#name,type,(int)((long)&(((st*)0)->name)),i}
#define _FIELD_END {NULL,0,0,-1}


typedef struct __Field{
	char *name;
	int type;
	int offset;
	int parm;  // used when type is OBJECT (Object Idx in PRO)/fixLen is the size
}_Field;


typedef struct _DB_Object{
	_Field *pTbl;
	int oSize; // c-struct sizeof
}DB_Object;


typedef struct _DB_Table{
	char *name;
	int oIdx;  // Table ROW Object Idx in OBJ
}DB_Table;


/*
 *	must be configed before call API
 *  to regist db and obj-fingures
 */
void litdb_register(char *dataDir, DB_Object PRO[], DB_Table DB[]);


/*
 *	tokyocabinet as dbm, muti-tables support 
 *  used by server mostly
 */
void* DBLite_get(char *pTblName, char *KEY);
bool  DBLite_put(char *pTblName, char *KEY, void *pObj);
void  DBLite_earse(char *pTblName, char *KEY);
int   DBLite_iterinit(char *pTblName);
char* DBLite_iternext(char *pTblName);
void  DBLite_flush();


/*
 *  light-file store. one object to a file
 *  which is siutable for client to store light data.
 *  used by client mostly
 */
void* DBLsg_get(char *pTblName);
bool  DBLsg_put(char *pTblName, void *pObj);


#endif