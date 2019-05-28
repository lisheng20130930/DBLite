#ifndef TEST_DATABASE_H
#define TEST_DATABASE_H


void DataBase_init(char *_dataDir);
void DataBase_uint();
void DataBase_loop();


void updatelastLoginTime(char *szDeviceID);



#endif