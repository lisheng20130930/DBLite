#include "libos.h"
#include "litdb.h"


DB_Object  *PRO = NULL;  // Defined By DPRoto
DB_Table   *DB = NULL;  // Defined By DPRoto
char	   *g_dataDir = NULL;

/*
 *	must be configed before call API
 *  to regist db and obj-fingures
 */
void litdb_register(char *dataDir, DB_Object PRO[], DB_Table DB[])
{
	PRO = PRO;
	DB = DB;
	g_dataDir = dataDir;
}