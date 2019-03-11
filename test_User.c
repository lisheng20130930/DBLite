#include "libos.h"
#include "litdb.h"
#include "test_User.h"



#define UsrDAT     "Usr.DAT"



static _Field UsrTable[]={	
	_FIELD(USR_T,age,_FIELD_TYPE_INT,0xFFFF),
	_FIELD_END,
};


static DB_Object PRO[]={
	{UsrTable,sizeof(USR_T)},
	{NULL,0}
};


static DB_Table DB[]={
	{UsrDAT,0},
	{NULL,0xFFFF}
};


static USR_T *g_sharedUsr = NULL;


void usr_registerDB(char *_dataDir)
{
	litdb_register(_dataDir,PRO,DB);
}


USR_T* shared_usr()
{
	if(g_sharedUsr){
		return g_sharedUsr;	
	}
	
	g_sharedUsr = (USR_T*)DBLsg_get(UsrDAT);
	if(!g_sharedUsr){
		g_sharedUsr = (USR_T*)malloc(sizeof(USR_T));
		memset(g_sharedUsr,0x00,sizeof(USR_T));
		g_sharedUsr->age = 0;
		DBLsg_put(UsrDAT,g_sharedUsr);
	}

	return g_sharedUsr;
}


void dump_usr()
{
	if(g_sharedUsr){
		DBLsg_put(UsrDAT,g_sharedUsr);
	}
}