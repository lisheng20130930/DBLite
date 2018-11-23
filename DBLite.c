#include "libos.h"
#include "DBLite.h"
#include "DBM.h"
#include "time.h"
#include "assert.h"


#define _NAME_MAX  (32)
#define _DB_MAX    (5)

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

static __inline DB_Table* getTable(char *pTblName)
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
