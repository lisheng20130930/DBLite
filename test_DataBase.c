#include "libos.h"
#include "litdb.h"
#include "test_DataBase.h"


#define DBDevice     "DBDevice.DAT"


static _Field DeviceTable[]={
	_FIELD(Device_t,szDeviceID,_FIELD_TYPE_STRZ,0xFFFF),
	_FIELD(Device_t,productHead,_FIELD_TYPE_OBJECT,1),
	_FIELD(Device_t,pExceptionHead,_FIELD_TYPE_OBJECT,2),
	_FIELD(Device_t,createtime,_FIELD_TYPE_INT,0xFFFF),
	_FIELD(Device_t,lastLoginTime,_FIELD_TYPE_INT,0xFFFF),
	_FIELD_END,
};

static _Field ProductTable[]={
	_FIELD(Product_t,next,_FIELD_TYPE_OBJECT,1),
	_FIELD(Product_t,coin,_FIELD_TYPE_INT,0xFFFF),	
	_FIELD(Product_t,sender,_FIELD_TYPE_INT,0xFFFF),
	_FIELD(Product_t,time,_FIELD_TYPE_INT,0xFFFF),
	_FIELD_END,
};

static _Field ExceptionTable[]={
	_FIELD(Exception_t,next,_FIELD_TYPE_OBJECT,2),	
	_FIELD(Exception_t,errorCode,_FIELD_TYPE_INT,0xFFFF),
	_FIELD(Exception_t,time,_FIELD_TYPE_INT,0xFFFF),
	_FIELD(Exception_t,handled,_FIELD_TYPE_INT,0xFFFF),
	_FIELD_END,
};


static DB_Object PRO[]={
	{DeviceTable,sizeof(Device_t)},
	{ProductTable,sizeof(Product_t)},
	{ExceptionTable,sizeof(Exception_t)},
	{NULL,0}
};


static DB_Table DB[]={
	{DBDevice,0},
	{NULL,0xFFFF}
};


Device_t* DataBase_getDevice(char *szDeviceID)
{
	return (Device_t*)DBLite_get(DBDevice,szDeviceID);
}

bool DataBase_putDevice(char *szDeviceID, Device_t *pDevice)
{
	return DBLite_put(DBDevice,szDeviceID,pDevice);
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