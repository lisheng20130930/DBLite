#include "libos.h"
#include "DBLite.h"
#include "DPRoto.h"


//////////////////////////////////////////////////////////////////////////
// Object Fields
static DB_Field USRFieldTable[]={
	DB_FIELD(USR_T,cbGender,DB_FIELD_TYPE_INT,0xFFFF),
	DB_FIELD(USR_T,ID,DB_FIELD_TYPE_STRZ,0xFFFF),
	DB_FIELD(USR_T,pTreasure,DB_FIELD_TYPE_OBJECT,1),
	DB_FIELD_END,
};


static DB_Field TreasureFieldTable[]={
	DB_FIELD(Treasure_t,iGold,DB_FIELD_TYPE_INT,0xFFFF),
	DB_FIELD_END,
};

//////////////////////////////////////////////////////////////////////////
// Sum
DB_Object  BJ[]={
	{USRFieldTable,sizeof(USR_T)},
	{TreasureFieldTable,sizeof(USR_T)},
	{NULL,0}
};


DB_Table DB[]={
	{USRDAT,0},
	{NULL,0xFFFF}
};


//////////////////////////////////////////////////////////////////////////
// smartFunS
void treasueRelease(Treasure_t *pTreasure)
{
	free(pTreasure);
}

void usrRelease(USR_T *pUsr)
{
	treasueRelease(pUsr->pTreasure);
	free(pUsr);
}