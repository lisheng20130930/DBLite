#include "stdlib.h"
#include "DBLite.h"


static USR_T* newUsr(char *IMSI)
{
	USR_T *pUsr = (USR_T*)malloc(sizeof(USR_T));
	memset(pUsr,0x00,sizeof(USR_T));
	pUsr->cbGender = 0;
	pUsr->ID = we_cmmn_strndup(IMSI,strlen(IMSI));
	pUsr->pszNickName = we_cmmn_strndup(pUsr->ID,strlen(pUsr->ID));
	pUsr->IMSI = we_cmmn_strndup(IMSI,strlen(IMSI));
	
	return pUsr;
}


#define IMSI "8613825468279"
int main(int argc, char **argv)
{
	DBLite_init("./");

	USR_T *pUsr = DBLite_loadUsr(IMSI);
	if(!pUsr){
		pUsr = newUsr(IMSI);
		DBLite_dumpUsr(pUsr);
	}
	usrRelease(pUsr);

	pUsr = DBLite_loadUsr(IMSI);
	usrRelease(pUsr);
	
	// Use the DBLite_loop in the Server Loop, to dump Dirty-DATA to DISK
	// Here is Simple
	DBLite_loop(0);

	pUsr = DBLite_loadUsr(IMSI);
	usrRelease(pUsr);

	DBLite_uint();
	return 0;
}