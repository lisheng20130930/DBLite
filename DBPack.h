#ifndef DBPACK_H
#define DBPACK_H


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


extern DB_Object  PRO[];  // Defined By DPRoto
extern DB_Table   DB[];  // Defined By DPRoto



char* obj2Buffer(_Field *pTbl, void *pObj, int *piLen);
void* buffer2Obj(_Field *pTbl, int oSize, char *buffer, int iLen, int *piSize);


#endif