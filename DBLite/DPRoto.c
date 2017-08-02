#include "libos.h"
#include "DBLite.h"
#include "DPRoto.h"


//////////////////////////////////////////////////////////////////////////
// Object Fields
static _Field USRFieldTable[]={
	_FIELD(USR_T,cbGender,_FIELD_TYPE_INT,0xFFFF),
	_FIELD(USR_T,ID,_FIELD_TYPE_STRZ,0xFFFF),
	_FIELD(USR_T,pTreasure,_FIELD_TYPE_OBJECT,1),
	_FIELD_END,
};


static _Field TreasureFieldTable[]={
	_FIELD(Treasure_t,iGold,_FIELD_TYPE_INT,0xFFFF),
	_FIELD_END,
};

//////////////////////////////////////////////////////////////////////////
// Sum
DB_Object  PRO[]={
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
