#include "libos.h"
#include "DBLite.h"
#include "DBM.h"
#include "time.h"
#include "assert.h"



static char *DBLsg_file2buffer(char *filename, int *size)
{
	/* your platform code here */
	return NULL;
}

static void DBLsg_buffer2file(char *filename, char *buffer, int size)
{
	/* your platform code here */
	return;
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

void* DBLsg_get(char *pTblName)
{
	DB_Table *pTable = getTable(pTblName);
	if(!pTable){
		return NULL;
	}
	
	int iLen = 0;
	char *buffer = DBLsg_file2buffer(pTblName, &iLen);
	
	if(!buffer){
		return NULL;
	}
	
	void *pObj = buffer2Obj(PRO[pTable->oIdx].pTbl,PRO[pTable->oIdx].oSize,buffer,iLen,NULL);
	free(buffer);
	
	return pObj;
}

bool DBLsg_put(char *pTblName, void *pObj)
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
	
	DBLsg_buffer2file(pTblName, buffer, iLen);

	return true;
}