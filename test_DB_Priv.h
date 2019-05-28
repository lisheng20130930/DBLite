#ifndef TEST_DB_PRIV_H
#define TEST_DB_PRIV_H


#define DBDevice     "DBDevice.DAT"


typedef struct _Product_s
{
	struct _Product_s *next;
	int coin;
	int sender;
	int time;
}Product_t;


typedef struct _Exception_s
{
	struct _Exception_s *next;
	int errorCode; //异常码
	int time; //时间
	int handled;//是否已经处理
}Exception_t;


typedef struct _Device_s
{
	char *szDeviceID; //设备唯一标识
	Product_t *productHead;//摇一摇记录
	Exception_t *pExceptionHead; //异常记录
	int createtime; //创建日期
	int lastLoginTime; //上次登陆日期
}Device_t;


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


static void freeDevice(Device_t *pDevice)
{
	while(pDevice->productHead){
		Product_t *pTemp = pDevice->productHead;
		pDevice->productHead = pTemp->next;
		free(pTemp);
	}
	
	while(pDevice->pExceptionHead){
		Exception_t *pTemp = pDevice->pExceptionHead;
		pDevice->pExceptionHead = pTemp->next;
		free(pTemp);
	}
	
	free(pDevice);
}


#endif