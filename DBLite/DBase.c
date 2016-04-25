#include "DBase.h"
#include "DBM.h"

#define DB_DUMP_CROND (32)
typedef struct _DTABLE
{
    int used; // 0/1
    TCHDB *db;
    char name[_NAME_MAX];
    char path[_PATH_MAX+_NAME_MAX]; // 带路径的文件名字
}DTABLE;


typedef struct _DBASE
{
    DTABLE arr[_DB_MAX];
    char *path;
}DBASE;


static DBASE G_DBase = {0};


static DTABLE* DBase_openedTable(char *name)
{
    int omode = HDBOREADER|HDBOWRITER;
    int i = 0;
    int found = FALSE;
    int t = (-1);
    
    for (i = 0; i < _DB_MAX; i++) {
        if (!G_DBase.arr[i].used) {
            t = ((-1)==t)?i:t;
            continue;
        }
        
        if (G_DBase.arr[i].used&&!stricmp(G_DBase.arr[i].name,name)) {
            found = TRUE;
            break;
        }
    }
    
    if (!found) {
        if (t == (-1)) {
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
    
    if (NULL == G_DBase.arr[i].db) {   
        /* new db */
        G_DBase.arr[i].db = tchdbnew();
        if (NULL == G_DBase.arr[i].db) {
            return NULL;
        }
        
        /* check dbexist */
        if (!tchdbexist(G_DBase.arr[i].path)) {
            omode = omode | HDBOCREAT | HDBOTRUNC;
        }
        
        /* open */
        if (!tchdbopen(G_DBase.arr[i].db, G_DBase.arr[i].path, omode)) {
            tchdbdel(G_DBase.arr[i].db);
            G_DBase.arr[i].db = NULL;
            return NULL;
        }
    }
    
    return &G_DBase.arr[i];
}

int DBase_init(const char *path)
{
    G_DBase.path = (char*)path;
    return TRUE;
}

void DBase_uint()
{
    int i = 0;
    
    for (i = 0; i < _DB_MAX; i++) {
        if (!G_DBase.arr[i].used) {
            continue;
        }
        
        if (NULL != G_DBase.arr[i].db) {
            tchdbclose(G_DBase.arr[i].db);
            tchdbdel(G_DBase.arr[i].db);
            G_DBase.arr[i].db = NULL;
        }
    }
}

static void DBase_dump(int flush)
{
    int i = 0;

	if(!flush){
		return;
	}
    
    /* close all tchdb */
    for (i = 0; i < _DB_MAX; i++) {
        if (!G_DBase.arr[i].used) {
            continue;
        }
        
        if (NULL == G_DBase.arr[i].db) {
            continue;
        }
        
        /* close */
        tchdbclose(G_DBase.arr[i].db);
        tchdbdel(G_DBase.arr[i].db);
        G_DBase.arr[i].db = NULL;
    }
}

void DBase_loop(int loops)
{
    DBase_dump(0==(loops%DB_DUMP_CROND));
}

int DBase_put(char *name, char *key, int ksiz, char* val, int vsiz)
{
    DTABLE *dbnode = NULL;
            
    dbnode = DBase_openedTable(name);
    if (NULL == dbnode) {
        return FALSE;
    }

    return tchdbput(dbnode->db, key, ksiz, val, vsiz);
}

char* DBase_get(char *name, char *key, int ksiz, int *vsiz)
{
    DTABLE *dbnode = NULL;
    
    dbnode = DBase_openedTable(name);
    if (NULL == dbnode) {
        return NULL;
    }

    return tchdbget(dbnode->db, key, ksiz, vsiz);
}

int DBase_erase(char *name, char *key, int ksiz)
{
    DTABLE *pTable = NULL;
    
    pTable = DBase_openedTable(name);
    if (NULL == pTable) {
        return FALSE;
    }

    return tchdbout(pTable->db, key, ksiz);
}

int DBase_visit(char *name, pfn_DBase_visit_callback visit, void* pUsr)
{
    DTABLE *pTable = NULL;
    char *key = NULL;
    int ksiz = 0;
    char *val = NULL;
    int vsiz = 0;
    int bContinue = TRUE;
    
    pTable = DBase_openedTable(name);
    if (NULL == pTable) {
        return FALSE;
    }

    tchdbiterinit(pTable->db);
    key = tchdbiternext(pTable->db, &ksiz);
    while (NULL!=key) {
        val = tchdbget(pTable->db, key, ksiz, &vsiz);
        if (NULL!=val) {
            bContinue = visit(pUsr, key, ksiz, val, vsiz);            
        }
        cp_zfree(val);
        cp_zfree(key);
        if (bContinue) {
            break;
        }        
        key = tchdbiternext(pTable->db, &ksiz);
    }
    
    return TRUE;
}
