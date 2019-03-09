#ifndef TEST_DATABASE_H
#define TEST_DATABASE_H


typedef struct _Product_s{
	struct _Product_s *next;
	int coin;
	int sender;
	int time;
}Product_t;


typedef struct _Exception_s{
	struct _Exception_s *next;
	int errorCode; //异常码
	int time; //时间
	int handled;//是否已经处理
}Exception_t;


typedef struct _Device_s{
	char *szDeviceID; //设备唯一标识
	Product_t *productHead;//摇一摇记录
	Exception_t *pExceptionHead; //异常记录
	int createtime; //创建日期
	int lastLoginTime; //上次登陆日期
}Device_t;



void DataBase_init(char *_dataDir);
void DataBase_uint();
void DataBase_loop();


Device_t* DataBase_getDevice(char *szDeviceID);
bool DataBase_putDevice(char *szDeviceID, Device_t *pDevice);



#endif