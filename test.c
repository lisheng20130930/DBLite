#include "libos.h"
//#include "test_DataBase.h"
#include "test_User.h"


int main(int argc, char **argv)
{
	/*
	DataBase_init(".");


	Device_t stDevice;
	stDevice.createtime = 12;
	stDevice.lastLoginTime = 21;
	stDevice.pExceptionHead = NULL;
	stDevice.productHead = NULL;
	stDevice.szDeviceID = "DSAFSDFASFASD";

	DataBase_putDevice("DSAFSDFASFASD", &stDevice);


	Device_t *pDevice = DataBase_getDevice("DSAFSDFASFASD");


	while(1){
		DataBase_loop();
	}
	
	DataBase_uint();
	*/

	usr_registerDB(".");

	USR_T *usr = shared_usr();

	usr->age = 200;
	dump_usr();

	return 0;
}
