#include "libos.h"
#include "DBLite.h"


#define STZIMSI "8613825468279"
int main(int argc, char **argv)
{
	USR_T *pUsr = (USR_T*)DBLite_get(USRDAT,STZIMSI);
	if(!pUsr){
		pUsr = pUsr = (USR_T*)malloc(sizeof(USR_T));
		memset(pUsr,0x00,sizeof(USR_T));
		pUsr->cbGender = 0;
		pUsr->ID = we_cmmn_strndup(STZIMSI,strlen(STZIMSI));
		pUsr->pTreasure = (Treasure_t*)malloc(sizeof(Treasure_t));
		memset(pUsr->pTreasure,0x00,sizeof(Treasure_t));
		pUsr->pTreasure->iGold = 2000;
		DBLite_put(USRDAT,STZIMSI,pUsr);
	}
	usrRelease(pUsr);

	pUsr = (USR_T*)DBLite_get(USRDAT,STZIMSI);
	usrRelease(pUsr);
	
	DBLite_flush();

	pUsr = (USR_T*)DBLite_get(USRDAT,STZIMSI);
	usrRelease(pUsr);

	return 0;
}
