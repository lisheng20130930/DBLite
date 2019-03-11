#ifndef TEST_USER_H
#define TEST_USER_H


/*
 *	client test
 */


typedef struct USR_T{
	int age;
}USR_T;


void usr_registerDB(char *_dataDir);


USR_T* shared_usr();
void dump_usr();



#endif