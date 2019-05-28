#include "libos.h"
#include "litdb.h"
#include "pack.h"
#include "config.h"


static char realName[1024] = {0};


static bool buffer2file(char *buffer, int len, char *pszFileName)
{
    sprintf(realName, "%s/%s", g_dataDir,pszFileName);
	int RET = remove(realName);
    FILE *pOUT = fopen(realName, "ab+");
	if(NULL==pOUT){		
        return false;
    }
    RET=fwrite(buffer,len,1,pOUT);
    fclose(pOUT);
	return true;
}

static bool file2buffer(char *pszFileName, char **ppszBuff, int *piLen)
{	
	sprintf(realName, "%s/%s", g_dataDir,pszFileName);	
    FILE *pFile = NULL;    
    char *pcFileBuff = NULL;
    int iInLen = 0;
    
    pFile = fopen(realName, "rb");
    if(0 == pFile){
        return false;
    }
    
    fseek(pFile, 0, SEEK_END);    
    iInLen = ftell(pFile);
    pcFileBuff = (char*)malloc(iInLen+1);
    memset(pcFileBuff, 0x00, iInLen+1);
    fseek(pFile, 0, SEEK_SET);    
    fread(pcFileBuff,iInLen, 1, pFile);    
    fclose(pFile);
    
    *ppszBuff = pcFileBuff;
	if(piLen)*piLen = iInLen;
    
    return true;
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

void* DBLsg_get(char *pTblName)
{
	DB_Table *pTable = getTable(pTblName);
	if(!pTable){
		return NULL;
	}
	
	int iLen = 0;
	char *buffer = NULL;	
	if(!file2buffer(pTblName, &buffer, &iLen)){
		return NULL;
	}
	
	void *pObj = buffer2Obj(PRO[pTable->oIdx].pTbl,PRO[pTable->oIdx].oSize,buffer,iLen,NULL);
	free(buffer);
	
	return pObj;
}

bool  DBLsg_put(char *pTblName, void *pObj)
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
	
	buffer2file(buffer, iLen, pTblName);
	
	return true;
}