#include "libos.h"
#include "DBLite.h"
#include "DBM.h"
#include "time.h"
#include "assert.h"


typedef struct _DTABLE
{
    int used; // 0/1
    TCHDB *db;
    char name[_NAME_MAX];
}DTABLE;


typedef struct _DBASE
{
    DTABLE arr[_DB_MAX];
}DBASE;


static DBASE G_DBase = {0};

static char* obj2Buffer(_Field *pTbl, void *pObj, int *piLen);
static void* buffer2Obj(_Field *pTbl, int oSize, char *buffer, int iLen, int *piSize);


static DTABLE* openedTable(char *name)
{
    int omode = HDBOREADER|HDBOWRITER;
    int i = 0;
    int found = false;
    int t = (-1);
    
    for(i=0; i<_DB_MAX; i++){
        if(!G_DBase.arr[i].used){
            t = ((-1)==t)?i:t;
            continue;
        }        
        if (G_DBase.arr[i].used&&!stricmp(G_DBase.arr[i].name,name)) {
            found = true;
            break;
        }
    }
    
    if(!found){
        if(t == (-1)){
            return NULL;
        }        
        strcpy(G_DBase.arr[t].name, name);
        G_DBase.arr[t].used = true;
        /* assign i */
        i = t;
        found = true;
    }    
    
    if(NULL == G_DBase.arr[i].db){   
        /* new db */
        G_DBase.arr[i].db = tchdbnew();
        if(NULL == G_DBase.arr[i].db){
            return NULL;
        }        
        /* check dbexist */
        if(!tchdbexist(G_DBase.arr[i].name)){
            omode = omode | HDBOCREAT | HDBOTRUNC;
        }        
        /* open */
        if(!tchdbopen(G_DBase.arr[i].db,G_DBase.arr[i].name,omode)){
            tchdbdel(G_DBase.arr[i].db);
            G_DBase.arr[i].db = NULL;
            return NULL;
        }
    }
    
    return &G_DBase.arr[i];
}

static DB_Table* getTable(char *pTblName)
{
	DB_Table *pTable = DB;
	while(pTable&&pTable->name){
		if(!strcmp(pTable->name,pTblName)){
			return pTable;
		}
		pTable++;
	}
	return NULL;
}

static void extendBuffer(char **buffer,int *piLen, int iPos, int iAppendLen)
{
	if(*piLen>=iPos+iAppendLen){
		return;
	}
	
	int incLen = __max(128,(iAppendLen+iPos-*piLen));
	char *temp = *buffer;	
	*buffer = (char*)malloc(*piLen+incLen);
	memset(*buffer,0x00,*piLen+incLen);

	if(temp){
		memcpy(*buffer,temp,*piLen);
		free(temp);
	}	
	*piLen += incLen;
}

static void readFixL(void *p, char *buffer, int *pos, int size)
{
	memcpy(p,buffer+*pos,size);
	*pos+=size;
	return;
}

static void writeFixL(char **buffer, int *len, int *pos, void *p, int size)
{
	extendBuffer(buffer,len,*pos,size);
	memcpy(*buffer+*pos,p,size);
	*pos+=size;
}

static unsigned char readByte(char *buffer, int *pos)
{
	unsigned char v = 0;
	readFixL(&v,buffer,pos,1);
	return v;
}

static void writeByte(char **buffer, int *len, int *pos, unsigned char v)
{
	writeFixL(buffer,len,pos,&v,1);
}

static short readShort(char *buffer, int *pos)
{
	short v = 0;
	readFixL(&v,buffer,pos,2);
	return v;
}

static void writeShort(char **buffer, int *len, int *pos, short v)
{
	writeFixL(buffer,len,pos,&v,2);
}

static int readInt32(char *buffer, int *pos)
{
	int v = 0;
	readFixL(&v,buffer,pos,4);
	return v;
}

static void writeInt32(char **buffer, int *len, int *pos, int v)
{
	writeFixL(buffer,len,pos,&v,4);
}

static char* readBuffer(char *buffer, int *pos, int *piLen)
{
	int iLen = readInt32(buffer, pos);
	if(iLen==0){
		return NULL;
	}
	char *pBuffer = (char*)malloc(iLen);
	readFixL(pBuffer,buffer,pos,iLen);
	if(piLen){
		*piLen = iLen;
	}
	return pBuffer;
}

static void writeBuffer(char **buffer,int *len, int *pos, char *v, int iLen)
{	
	writeInt32(buffer,len,pos,iLen);
	if(iLen!=0){
		writeFixL(buffer,len,pos,v,iLen);
	}
}

static char* readStrz(char *buffer, int *pos)
{
	return readBuffer(buffer,pos,NULL);
}

static void writeStrz(char **buffer,int *len, int *pos, char *v)
{
	writeBuffer(buffer,len,pos,v?v:"",v?(strlen(v)+1):1);
}

static void* readObject(char *buffer, int len, int *pos, int parm)
{
	int iLen = 0;
	char *pBuffer = readBuffer(buffer, pos, &iLen);
	if(!pBuffer){
		return NULL;
	}
	int t = 0;
	void *v = buffer2Obj(PRO[parm].pTbl,PRO[parm].oSize,pBuffer,iLen,&t);
	free(pBuffer);
	return v;
}

static void writeObject(char **buffer,int *len, int *pos, void *v, int parm)
{
	if(parm<0||!v){
		writeBuffer(buffer,len,pos,NULL,0);
		return;
	}
	int iLen = 0;
	char *pBuffer = obj2Buffer(PRO[parm].pTbl,v,&iLen);
	if(!pBuffer){
		writeBuffer(buffer,len,pos,NULL,0);
		return;
	}	
	writeBuffer(buffer,len,pos,pBuffer,iLen);	
	free(pBuffer);
}

static void* buffer2Obj(_Field *pTbl, int oSize, char *buffer, int iLen, int *piSize)
{
	void *pObj = malloc(oSize);
	memset(pObj,0x00, oSize);
	
	_Field *pField = (_Field*)pTbl;
	int pos = 0;
	while(pField&&pField->name){
		switch(pField->type){
		case _FIELD_TYPE_BYTE:
			*((unsigned char*)((unsigned char*)pObj+pField->offset)) = readByte(buffer,&pos);
			break;
		case _FIELD_TYPE_SHORT:
			*((short*)((unsigned char*)pObj+pField->offset)) = readShort(buffer,&pos);
			break;
		case _FIELD_TYPE_INT:
			*((int*)((unsigned char*)pObj+pField->offset)) = readInt32(buffer,&pos);
			break;
		case _FIELD_TYPE_STRZ:
			*((char**)((unsigned char*)pObj+pField->offset)) = readStrz(buffer,&pos);
			break;
		case _FIELD_TYPE_OBJECT:
			*((void**)((unsigned char*)pObj+pField->offset)) = readObject(buffer,iLen,&pos,pField->parm);
			break;
		case _FIELD_TYPE_FIXL:
			readFixL(*((void**)((unsigned char*)pObj+pField->offset)),buffer,&pos,pField->parm);
			break;
		default:
			break;
		}
		pField++;
	}

	if(piSize){
		*piSize = pos;
	}
	return pObj;
}


static char* obj2Buffer(_Field *pTbl, void *pObj, int *piLen)
{
	int iLen = 0;	
	char *buffer = NULL;
	
	_Field *pField = (_Field*)pTbl;
	int pos = 0;
	while(pField&&pField->name){
		switch(pField->type){
		case _FIELD_TYPE_BYTE:
			writeByte(&buffer,&iLen,&pos,*((unsigned char*)((unsigned char*)pObj+pField->offset)));
			break;
		case _FIELD_TYPE_SHORT:
			writeShort(&buffer,&iLen,&pos,*((short*)((unsigned char*)pObj+pField->offset)));
			break;
		case _FIELD_TYPE_INT:
			writeInt32(&buffer,&iLen,&pos,*((int*)((unsigned char*)pObj+pField->offset)));
			break;
		case _FIELD_TYPE_STRZ:
			writeStrz(&buffer,&iLen,&pos,*((char**)((unsigned char*)pObj+pField->offset)));
			break;
		case _FIELD_TYPE_OBJECT:
			writeObject(&buffer,&iLen,&pos,*((void**)((unsigned char*)pObj+pField->offset)),pField->parm);
			break;
		case _FIELD_TYPE_FIXL:
			writeFixL(&buffer,&iLen,&pos,*((void**)((unsigned char*)pObj+pField->offset)),pField->parm);
			break;
		default:
			break;
		}
		pField++;
	}
	
	*piLen = pos;
	return buffer;
}

void* DBLite_get(char *pTblName, char *KEY)
{
	DB_Table *pTable = getTable(pTblName);
	if(!pTable){
		return NULL;
	}
	
	DTABLE *pDB = openedTable(pTable->name);
    if(!pDB){
        return NULL;
    }

	int iLen = 0;
	char *buffer = tchdbget(pDB->db, KEY,(strlen(KEY)+1), &iLen);

	if(!buffer){
		return NULL;
	}
	
	void *pObj = buffer2Obj(PRO[pTable->oIdx].pTbl,PRO[pTable->oIdx].oSize,buffer,iLen,NULL);
	free(buffer);

	return pObj;
}

bool DBLite_put(char *pTblName, char *KEY, void *pObj)
{
	DB_Table *pTable = getTable(pTblName);
	if(!pTable){
		return false;
	}

	int iLen = 0;
	char *buffer = obj2Buffer(PRO[pTable->oIdx].pTbl,pObj,&iLen);

	if(!buffer){
		return false;
	}

	DTABLE *pDB = openedTable(pTable->name);
    if(!pDB) {
        return false;
    }
    return tchdbput(pDB->db, KEY,(strlen(KEY)+1),buffer,iLen);
}

void  DBLite_earse(char *pTblName, char *KEY)
{
	DB_Table *pTable = getTable(pTblName);
	if(!pTable){
		return;
	}
	DTABLE *pDB = openedTable(pTable->name);
    if(!pDB){
        return;
    }
    tchdbout(pDB->db, KEY,(strlen(KEY)+1));
}

int DBLite_iterinit(char *pTblName)
{
	DB_Table *pTable = getTable(pTblName);
	if(!pTable){
		return -1;
	}	
	DTABLE *pDB = openedTable(pTable->name);
    if(!pDB){
        return -1;
    }
    tchdbiterinit(pDB->db);
	return 0;
}

char* DBLite_iternext(char *pTblName)
{
	DB_Table *pTable = getTable(pTblName);
	if(!pTable){
		return NULL;
	}
	
	DTABLE *pDB = openedTable(pTable->name);
	if(!pDB){
		return NULL;
	}

	int ksiz = 0;
	return tchdbiternext(pDB->db, &ksiz);
}

void DBLite_flush()
{
    for(int i=0;i<_DB_MAX;i++){
        if(!G_DBase.arr[i].used){
            continue;
        }        
        if(NULL == G_DBase.arr[i].db){
            continue;
        }        
        tchdbclose(G_DBase.arr[i].db);
        tchdbdel(G_DBase.arr[i].db);
        G_DBase.arr[i].db = NULL;
    }
}
