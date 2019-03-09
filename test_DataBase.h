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
	int errorCode; //�쳣��
	int time; //ʱ��
	int handled;//�Ƿ��Ѿ�����
}Exception_t;


typedef struct _Device_s{
	char *szDeviceID; //�豸Ψһ��ʶ
	Product_t *productHead;//ҡһҡ��¼
	Exception_t *pExceptionHead; //�쳣��¼
	int createtime; //��������
	int lastLoginTime; //�ϴε�½����
}Device_t;



void DataBase_init(char *_dataDir);
void DataBase_uint();
void DataBase_loop();


Device_t* DataBase_getDevice(char *szDeviceID);
bool DataBase_putDevice(char *szDeviceID, Device_t *pDevice);



#endif