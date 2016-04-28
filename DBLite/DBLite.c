#include "libos.h"
#include "DBLite.h"
#include "DBM.h"
#include "time.h"
#include "assert.h"


#define DB_DUMP_CROND (32)
typedef struct _DTABLE
{
    int used; // 0/1
    TCHDB *db;
    char name[_NAME_MAX];
    char path[_PATH_MAX+_NAME_MAX]; // ´øÂ·¾¶µÄÎÄ¼þÃû×Ö
}DTABLE;


typedef struct _DBASE
{
    DTABLE arr[_DB_MAX];
    char *path;
}DBASE;


static DBASE G_DBase = {0};

static char* obj2Buffer(DB_Field *pFieldTable, void *pObj, int *piLen);
static void* buffer2Obj(DB_Field *pFieldTable, int oSize, char *buffer, int iLen, int *piSize);


static DTABLE* openedTable(char *name)
{
    int omode = HDBOREADER|HDBOWRITER;
    int i = 0;
    int found = FALSE;
    int t = (-1);
    
    for(i=0; i<_DB_MAX; i++){
        if(!G_DBase.arr[i].used){
            t = ((-1)==t)?i:t;
            continue;
        }        
        if (G_DBase.arr[i].used&&!stricmp(G_DBase.arr[i].name,name)) {
            found = TRUE;
            break;
        }
    }
    
    if(!found){
        if(t == (-1)){
            return NULL;
        }        
        strcpy(G_DBase.arr[t].name, name);        
        strcat(G_DBase.arr[t].path, G_DBase.path);
        strcat(G_DBase.arr[t].path, name);
        G_DBase.arr[t].used = TRUE;
        /* assign i */
        i = t;
        found = TRUE;
    }    
    
    if(NULL == G_DBase.arr[i].db){   
        /* new db */
        G_DBase.arr[i].db = tchdbnew();
        if(NULL == G_DBase.arr[i].db){
            return NULL;
        }        
        /* check dbexist */
        if(!tchdbexist(G_DBase.arr[i].path)){
            omode = omode | HDBOCREAT | HDBOTRUNC;
        }        
        /* open */
        if(!tchdbopen(G_DBase.arr[i].db,G_DBase.arr[i].path,omode)){
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

static unsigned char readByte(char *buffer, int *pos)
{
	unsigned char v = 0;
	memcpy(&v,buffer+*pos,1);
	*pos+=1;
	return 0;
}

static void writeByte(char **buffer, int *len, int *pos, unsigned char v)
{
	extendBuffer(buffer,len,*pos,1);
	memcpy(*buffer+*pos,(void*)&v,sizeof(unsigned char));
	*pos+=1;
}

static short readShort(char *buffer, int *pos)
{
	short v = 0;
	memcpy(&v,buffer+*pos,2);
	*pos+=2;
	return 0;
}

static void writeShort(char **buffer, int *len, int *pos, short v)
{
	extendBuffer(buffer,len,*pos,2);
	memcpy(*buffer+*pos,(void*)&v,sizeof(short));
	*pos+=2;
}

static int readInt32(char *buffer, int *pos)
{
	int v = 0;
	memcpy(&v,buffer+*pos,4);
	*pos+=4;
	return v;
}

static void writeInt32(char **buffer, int *len, int *pos, int v)
{
	extendBuffer(buffer,len,*pos,4);
	memcpy(*buffer+*pos,(void*)&v,sizeof(int));
	*pos+=4;
}

static char* readBuffer(char *buffer, int *pos, int *piLen)
{
	int iLen = readInt32(buffer, pos);
	if(iLen==0){
		return NULL;
	}
	char *pBuffer = (char*)malloc(iLen);
	memcpy(pBuffer,buffer+*pos,iLen);
	if(piLen){
		*piLen = iLen;
	}
	*pos+=iLen;
	return pBuffer;
}

static void writeBuffer(char **buffer,int *len, int *pos, char *v, int iLen)
{
	extendBuffer(buffer,len,*pos,iLen+4);
	writeInt32(buffer,len,pos,iLen);
	if(iLen!=0){
		memcpy(*buffer+*pos,v,iLen);
		*pos+=iLen;
	}
}

static char* readStrz(char *buffer, int *pos)
{
	return readBuffer(buffer,pos,NULL);
}

static void writeStrz(char **buffer,int *len, int *pos, char *v)
{
	writeBuffer(buffer,len,pos,v,strlen(v)+1);
}

static void* readObject(char *buffer, int len, int *pos, int oIdx)
{
	int iLen = 0;
	char *pBuffer = readBuffer(buffer, pos, &iLen);
	if(!pBuffer){
		return NULL;
	}
	int t = 0;
	void *v = buffer2Obj(BJ[oIdx].pFieldTable,BJ[oIdx].oSize,pBuffer,iLen,&t);
	free(pBuffer);
	return v;
}

static void writeObject(char **buffer,int *len, int *pos, void *v, int oIdx)
{
	if(oIdx<0||!v){
		writeBuffer(buffer,len,pos,NULL,0);
		return;
	}
	int iLen = 0;
	char *pBuffer = obj2Buffer(BJ[oIdx].pFieldTable,v,&iLen);
	if(!pBuffer){
		writeBuffer(buffer,len,pos,NULL,0);
		return;
	}	
	writeBuffer(buffer,len,pos,pBuffer,iLen);	
	free(pBuffer);
}

static void* buffer2Obj(DB_Field *pFieldTable, int oSize, char *buffer, int iLen, int *piSize)
{
	void *pObj = malloc(oSize);
	memset(pObj,0x00, oSize);
	
	DB_Field *pField = (DB_Field*)pFieldTable;
	int pos = 0;
	while(pField&&pField->name){
		switch(pField->type){
		case DB_FIELD_TYPE_BYTE:
			*((unsigned char*)((unsigned char*)pObj+pField->offset)) = readByte(buffer,&pos);
			break;
		case DB_FIELD_TYPE_SHORT:
			*((short*)((unsigned char*)pObj+pField->offset)) = readShort(buffer,&pos);
			break;
		case DB_FIELD_TYPE_INT:
			*((int*)((unsigned char*)pObj+pField->offset)) = readInt32(buffer,&pos);
			break;
		case DB_FIELD_TYPE_STRZ:
			*((char**)((unsigned char*)pObj+pField->offset)) = readStrz(buffer,&pos);
			break;
		case DB_FIELD_TYPE_OBJECT:
			*((void**)((unsigned char*)pObj+pField->offset)) = readObject(buffer,iLen,&pos,pField->oIdx);
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


static char* obj2Buffer(DB_Field *pFieldTable, void *pObj, int *piLen)
{
	int iLen = 0;	
	char *buffer = NULL;
	
	DB_Field *pField = (DB_Field*)pFieldTable;
	int pos = 0;
	while(pField&&pField->name){
		switch(pField->type){
		case DB_FIELD_TYPE_BYTE:
			writeByte(&buffer,&iLen,&pos,*((unsigned char*)((unsigned char*)pObj+pField->offset)));
			break;
		case DB_FIELD_TYPE_SHORT:
			writeShort(&buffer,&iLen,&pos,*((short*)((unsigned char*)pObj+pField->offset)));
			break;
		case DB_FIELD_TYPE_INT:
			writeInt32(&buffer,&iLen,&pos,*((int*)((unsigned char*)pObj+pField->offset)));
			break;
		case DB_FIELD_TYPE_STRZ:
			writeStrz(&buffer,&iLen,&pos,*((char**)((unsigned char*)pObj+pField->offset)));
			break;
		case DB_FIELD_TYPE_OBJECT:
			writeObject(&buffer,&iLen,&pos,*((void**)((unsigned char*)pObj+pField->offset)),pField->oIdx);
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
	
	void *pObj = buffer2Obj(BJ[pTable->oIdx].pFieldTable,BJ[pTable->oIdx].oSize,buffer,iLen,NULL);
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
	char *buffer = obj2Buffer(BJ[pTable->oIdx].pFieldTable,pObj,&iLen);

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

static void DBLite_dump(int flush)
{    
	if(!flush){
		return;
	}
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

void DBLite_loop(int loops)
{
    DBLite_dump(0==(loops%DB_DUMP_CROND));
}

int DBLite_init(const char *path)
{
    G_DBase.path = (char*)path;
    return 0;
}

void DBLite_uint()
{
    for(int i=0;i<_DB_MAX;i++){
        if(!G_DBase.arr[i].used){
            continue;
        }        
        if(NULL != G_DBase.arr[i].db){
            tchdbclose(G_DBase.arr[i].db);
            tchdbdel(G_DBase.arr[i].db);
            G_DBase.arr[i].db=NULL;
        }
    }
}
