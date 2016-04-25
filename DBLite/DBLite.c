/* This is an example File, You can mdofiy freely */
#include "DBLite.h"


#define USRDAT "Usr.DB"


typedef struct _STUB {
	pfn_DB_Callback pfnCb;
	void* pCaller;
}DBLite_STUB;


#define KEY_SIZE (24)
static const unsigned char g_key_map[KEY_SIZE] = {
    0xE3,0x82,0x49,0x40,0x34,0x52,0x89,0x78,0x65,0x54,0x83,0x75,0x48,0x53,0x79,0x72,0x27,0x38,0x84,0x25,0x06,0x88,0x33,0x55
};

static void _decrypt(unsigned char *buffer, int len)
{
    int i = 0;
    for (i = 0; i < len; i++) {
        *(buffer+i) += g_key_map[i%KEY_SIZE];
    }
}

static void _encrypt(unsigned char *buffer, int len)
{
    int i = 0;
    for (i = 0; i < len; i++) {
        *(buffer+i) -= g_key_map[i%KEY_SIZE];
    }
}


USR_T* DBLite_loadUsr(char *ID)
{
	int iLen = 0;
	char *buffer = DBase_get(USRDAT,ID,(strlen(ID)+1), &iLen);
	if(!buffer){
		return NULL;
	}
	
	USR_T *pUsr = buffer2Usr(buffer,iLen);
	free(buffer);

	return pUsr;
}

void   DBLite_dumpUsr(USR_T *usr)
{
	int iLen = 0;
	char *buffer = usr2Buffer(usr,&iLen);
	
	DBase_put(USRDAT,usr->ID,(strlen(usr->ID)+1),buffer, iLen);

	free(buffer);
}

static int DBLite_visit_callback(void* pCaller, char* key, int ksiz, char *val, int vsiz)
{
	DBLite_STUB *pStub = (DBLite_STUB*)pCaller;

	USR_T *pUsr = buffer2Usr(val, vsiz);
	int iRET = pStub->pfnCb(pStub->pCaller,pUsr);
	free(pUsr);

	return iRET;
}

int  DBLite_visit(pfn_DB_Callback pfnCb, void* pCaller)
{
	DBLite_STUB stStub = {pfnCb,pCaller};
    return DBase_visit(USRDAT,DBLite_visit_callback,(void*)&stStub);
}