#include "libos.h"
#include "litdb.h"
#include "test_DataBase.h"
#include "test_DB_Priv.h"


static Device_t* getDeviceByID(char *szDeviceID)
{
	Device_t *pDevice = (Device_t*)DBLite_get(DBDevice,szDeviceID);
	if(!pDevice){
		pDevice = (Device_t*)malloc(sizeof(Device_t));
		memset(pDevice,0x00,sizeof(Device_t));
		pDevice->szDeviceID = strdup(szDeviceID);
		pDevice->createtime = time(NULL);
	}	
	return pDevice;
}

void updatelastLoginTime(char *szDeviceID)
{
	Device_t *pDevice = getDeviceByID(szDeviceID);
	pDevice->lastLoginTime = time(NULL);
	
	DBLite_put(DBDevice,szDeviceID,(void*)pDevice);
	freeDevice(pDevice);
}

void DataBase_init(char *_dataDir)
{
	litdb_register(_dataDir,PRO,DB);
}

void DataBase_uint()
{
	DBLite_flush();
}

void DataBase_loop()
{
	DBLite_flush();
}