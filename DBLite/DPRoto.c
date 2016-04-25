/* This is an example File, You can mdofiy freely */
#include "DPRoto.h"
#include "TLV.h"
#include "assert.h"
#include "libos.h"

//////////////////////////////////////////////////////////////////////////
// overlapitem
#define OVERLAPITEM_TYPE  _T(1,eint32)
#define OVERLAPITEM_ID    _T(2,eint32)
#define OVERLAPITEM_NUM	  _T(3,eint32)

overlapitem_t* buffer2OverlapItem(char *buffer, int len)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	
	tlv_decode(&pHead, buffer, len);
	assert(pHead);

	overlapitem_t *pItem = (overlapitem_t*)malloc(sizeof(overlapitem_t));
	memset(pItem,0x00,sizeof(overlapitem_t));

	pTLV = tlv_get(pHead,OVERLAPITEM_TYPE);
	pItem->etype = pTLV->v.int32Value;
	pTLV = tlv_get(pHead,OVERLAPITEM_ID);
	pItem->Id = pTLV->v.int32Value;
	pTLV = tlv_get(pHead,OVERLAPITEM_NUM);
	pItem->iNum = pTLV->v.int32Value;

	tlv_destroy(pHead);
	
	return pItem;
}

char* overlapitem2Buffer(overlapitem_t *obj, int *piLen)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	int size = 0;

	pTLV = tlv_createint32(OVERLAPITEM_TYPE, obj->etype);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createint32(OVERLAPITEM_ID, obj->Id);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createint32(OVERLAPITEM_NUM, obj->iNum);
	tlv_add(&pHead,pTLV);

	int len = tlv_encode(pHead,NULL);
	char *buffer = (char*)malloc(len);
	memset(buffer,0x00,len);
	size = tlv_encode(pHead,buffer);
	tlv_destroy(pHead);

	if(len!=size){
		free(buffer);
		return NULL;
	}

	*piLen = len;
	return buffer;
}


void overlapitemRelease(overlapitem_t *obj)
{
	free(obj);
}


//////////////////////////////////////////////////////////////////////////
// entry_t
#define GMENTRY_TYPE			_T(1,eint32)
#define GMENTRY_GOLD			_T(2,eint32)
#define GMENTRY_OVERLAPITEM		_T(3,ebytes)

entry_t* buffer2Entry(char *buffer, int len)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	
	tlv_decode(&pHead, buffer, len);
	assert(pHead);

	entry_t *pEntry = (entry_t*)malloc(sizeof(entry_t));
	memset(pEntry,0x00,sizeof(entry_t));

	pTLV = tlv_get(pHead,GMENTRY_TYPE);
	pEntry->etype = pTLV->v.int32Value;
	switch(pEntry->etype){
	case EGOLD:
		pTLV = tlv_get(pHead,GMENTRY_GOLD);
		pEntry->u.iGoldD = pTLV->v.int32Value;
		break;
	case EOVERLAPITEM:
		pTLV = tlv_get(pHead,GMENTRY_OVERLAPITEM);
		pEntry->u.overlapitm = buffer2OverlapItem((char*)pTLV->v.bytesValue.b,pTLV->v.bytesValue.l);
		break;
	default:
		break;
	}
	
	tlv_destroy(pHead);

	return pEntry;
}

char* entry2Buffer(entry_t *obj, int *piLen)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	int size = 0;

	pTLV = tlv_createint32(GMENTRY_TYPE, obj->etype);
	tlv_add(&pHead,pTLV);

	int _len = 0;
	char *_buffer = NULL;
	switch(obj->etype){
	case EGOLD:
		pTLV = tlv_createint32(GMENTRY_GOLD, obj->u.iGoldD);
		tlv_add(&pHead,pTLV);
		break;
	case EOVERLAPITEM:
		_buffer = overlapitem2Buffer(obj->u.overlapitm, &_len);
		pTLV=tlv_create(GMENTRY_OVERLAPITEM,_len,_buffer);
		tlv_add(&pHead,pTLV);
		break;
	default:
		break;
	}
	
	int len = tlv_encode(pHead,NULL);
	char *buffer = (char*)malloc(len);
	memset(buffer,0x00,len);
	size = tlv_encode(pHead,buffer);
	tlv_destroy(pHead);
	
	if(len!=size){
		free(buffer);
		return NULL;
	}
	
	*piLen = len;
	return buffer;
}

void entryRelease(entry_t *obj)
{
	switch(obj->etype){
	case EOVERLAPITEM:
		overlapitemRelease(obj->u.overlapitm);
	default:
		break;
	}
	
	free(obj);
}

//////////////////////////////////////////////////////////////////////////
// mail
#define MAIL_TITLE      _T(2,estr)
#define MAIL_CONTENT    _T(3,estr)
#define MAIL_DATE       _T(4,estr)
#define MAIL_STATUS     _T(5,eint32)
#define MAIL_ATTACHMENT _T(6,ebytes)
 
mail_t* buffer2Mail(char *buffer, int len)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	
	tlv_decode(&pHead, buffer, len);
	assert(pHead);
	
	mail_t *mail = (mail_t*)malloc(sizeof(mail_t));
	memset(mail,0x00,sizeof(mail_t));
	
	pTLV = tlv_get(pHead,MAIL_TITLE);
	mail->title = we_cmmn_strndup(pTLV->v.strValue,strlen(pTLV->v.strValue));
	pTLV = tlv_get(pHead,MAIL_CONTENT);
	mail->content = we_cmmn_strndup(pTLV->v.strValue,strlen(pTLV->v.strValue));
	pTLV = tlv_get(pHead,MAIL_DATE);
	mail->pszTime = we_cmmn_strndup(pTLV->v.strValue,strlen(pTLV->v.strValue));
	pTLV = tlv_get(pHead,MAIL_STATUS);
	mail->iStatus = pTLV->v.int32Value;
	
	#define _N (128)
	static tlv_t *___arr[_N] = {0};
	int i = 0;
	int n = 0;
	
	n=tlv_gets(pHead,MAIL_ATTACHMENT,___arr,_N);
	for(i=0;i<n;i++){
		entry_t *entry=buffer2Entry((char*)___arr[i]->v.bytesValue.b,(int)___arr[i]->v.bytesValue.l);
		entry->next=mail->pHeadAttachment;
		mail->pHeadAttachment=entry;
	}
	
	tlv_destroy(pHead);
	
	return mail;
}

char* mail2Buffer(mail_t *obj, int *piLen)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	int size = 0;
	
	pTLV = tlv_createstr(MAIL_TITLE, obj->title);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createstr(MAIL_CONTENT, obj->content);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createstr(MAIL_DATE, obj->pszTime);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createint32(MAIL_STATUS, obj->iStatus);
	tlv_add(&pHead,pTLV);
	
	char *_buffer=NULL;
	int _len = 0;	
	entry_t* entry = obj->pHeadAttachment;
	while(entry){
		_buffer = entry2Buffer(entry, &_len);
		pTLV=tlv_create(MAIL_ATTACHMENT,_len,_buffer);
		tlv_add(&pHead,pTLV);
		entry = entry->next;
	}
	
	int len = tlv_encode(pHead,NULL);
	char *buffer = (char*)malloc(len);
	memset(buffer,0x00,len);
	size = tlv_encode(pHead,buffer);
	tlv_destroy(pHead);
	
	if(len!=size){
		free(buffer);
		return NULL;
	}
	
	*piLen = len;
	return buffer;
}

void mailRelease(mail_t *obj)
{
	entry_t* entry = obj->pHeadAttachment;
	entry_t* pTempE = NULL;
	while(entry){		
		pTempE = entry->next;
		entryRelease(entry);
		entry = pTempE;
	}
	free(obj);
}

//////////////////////////////////////////////////////////////////////////
// rank
#define RANK_NNAME       _T(1,estr)
#define RANK_GENDER      _T(2,eint32)
#define RANK_GOLD        _T(3,eint32)

rank_t* buffer2Rank(char *buffer, int len)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	
	tlv_decode(&pHead, buffer, len);
	assert(pHead);
	
	rank_t *rank = (rank_t*)malloc(sizeof(rank_t));
	memset(rank,0x00,sizeof(rank_t));
	
	pTLV = tlv_get(pHead,RANK_NNAME);
	rank->name = we_cmmn_strndup(pTLV->v.strValue,strlen(pTLV->v.strValue));
	pTLV = tlv_get(pHead,RANK_GENDER);
	rank->cbGender = pTLV->v.int32Value;
	pTLV = tlv_get(pHead,RANK_GOLD);	
	rank->iGold = pTLV->v.int32Value;
	
	tlv_destroy(pHead);
	
	return rank;
}

char* rank2Buffer(rank_t* obj, int *piLen)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	int size = 0;
	
	pTLV = tlv_createstr(RANK_NNAME,obj->name);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createint32(RANK_GENDER, obj->cbGender);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createint32(RANK_GOLD, obj->iGold);
	tlv_add(&pHead,pTLV);
	
	int len = tlv_encode(pHead,NULL);
	char *buffer = (char*)malloc(len);
	memset(buffer,0x00,len);
	size = tlv_encode(pHead,buffer);
	tlv_destroy(pHead);
	
	if(len!=size){
		free(buffer);
		return NULL;
	}
	
	*piLen = len;
	return buffer;
}

void rankRelease(rank_t *obj)
{
	free(obj->name);
	free(obj);
}


//////////////////////////////////////////////////////////////////////////
// trigger
#define TRIGGER_EID     _T(1,eint32)
#define TRIGGER_UINT32  _T(2,eint32)
trigger_t* buffer2Trigger(char *buffer, int len)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	
	tlv_decode(&pHead, buffer, len);
	assert(pHead);
	
	trigger_t *tg = (trigger_t*)malloc(sizeof(trigger_t));
	memset(tg,0x00,sizeof(trigger_t));
	
	pTLV = tlv_get(pHead,TRIGGER_EID);
	tg->eTriger = pTLV->v.int32Value;
	pTLV = tlv_get(pHead,TRIGGER_UINT32);	
	*((int*)&tg->u) = pTLV->v.int32Value;
		
	tlv_destroy(pHead);
	
	return tg;
}

char* trigger2Buffer(trigger_t *obj, int *piLen)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	int size = 0;
	
	pTLV = tlv_createint32(TRIGGER_EID, obj->eTriger);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createint32(TRIGGER_UINT32, *((int*)&obj->u));
	tlv_add(&pHead,pTLV);
		
	int len = tlv_encode(pHead,NULL);
	char *buffer = (char*)malloc(len);
	memset(buffer,0x00,len);
	size = tlv_encode(pHead,buffer);
	tlv_destroy(pHead);
	
	if(len!=size){
		free(buffer);
		return NULL;
	}
	
	*piLen = len;
	return buffer;
}

void triggerRelease(trigger_t *obj)
{
	free(obj);
}


//////////////////////////////////////////////////////////////////////////
// user
#define USR_GOLD	 _T(1,eint32)
#define USR_GENDER	 _T(2,eint32)
#define USR_SCORE	 _T(3,eint32)
#define USR_MAIL     _T(4,ebytes)
#define USR_ENTRY    _T(5,ebytes)
#define USR_TG		 _T(6,ebytes)
#define USR_RANK     _T(7,ebytes)
#define USR_ID       _T(8,estr)
#define USR_NAME     _T(9,estr)
#define USR_IMSI     _T(10,estr)

USR_T* buffer2Usr(char *buffer, int len)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	
	tlv_decode(&pHead, buffer, len);
	assert(pHead);

	USR_T *usr = (USR_T*)malloc(sizeof(USR_T));
	memset(usr,0x00,sizeof(USR_T));
	
	pTLV = tlv_get(pHead,USR_ID);
	usr->ID = we_cmmn_strndup(pTLV->v.strValue,strlen(pTLV->v.strValue));
	pTLV = tlv_get(pHead,USR_NAME);
	usr->pszNickName = we_cmmn_strndup(pTLV->v.strValue,strlen(pTLV->v.strValue));
	pTLV = tlv_get(pHead,USR_GOLD);
	usr->iGold = pTLV->v.int32Value;
	pTLV = tlv_get(pHead,USR_GENDER);
	usr->cbGender = pTLV->v.int32Value;
	pTLV = tlv_get(pHead,USR_SCORE);
	usr->iScore = pTLV->v.int32Value;
	pTLV = tlv_get(pHead,USR_IMSI);
	usr->IMSI = we_cmmn_strndup(pTLV->v.strValue,strlen(pTLV->v.strValue));
	
	#define _N (128)
	static tlv_t *___arr[_N] = {0};
	int i = 0;
	int n = 0;
	
	n=tlv_gets(pHead,USR_ENTRY,___arr,_N);
	for(i=0;i<n;i++){
		entry_t *entry=buffer2Entry((char*)___arr[i]->v.bytesValue.b,(int)___arr[i]->v.bytesValue.l);
		entry->next=usr->pEntryHead;
		usr->pEntryHead=entry;
	}
	
	n=tlv_gets(pHead,USR_MAIL,___arr,_N);
	for(i=0;i<n;i++){
		mail_t *mail=buffer2Mail((char*)___arr[i]->v.bytesValue.b,(int)___arr[i]->v.bytesValue.l);
		mail->next=usr->headMail;
		usr->headMail=mail;
	}

	n=tlv_gets(pHead,USR_RANK,___arr,_N);
	for(i=0;i<n;i++){
		rank_t *rank=buffer2Rank((char*)___arr[i]->v.bytesValue.b,(int)___arr[i]->v.bytesValue.l);
		rank->next=usr->headRank;
		usr->headRank=rank;
	}

	n=tlv_gets(pHead,USR_TG,___arr,_N);
	for(i=0;i<n;i++){
		trigger_t *tg=buffer2Trigger((char*)___arr[i]->v.bytesValue.b,(int)___arr[i]->v.bytesValue.l);
		tg->next=usr->headTG;
		usr->headTG=tg;
	}
	
	tlv_destroy(pHead);
	
	return usr;
}

char* usr2Buffer(USR_T *obj, int *piLen)
{
	tlv_t *pHead = NULL;
	tlv_t *pTLV = NULL;
	int size = 0;
	
	pTLV = tlv_createstr(USR_ID, obj->ID);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createstr(USR_NAME, obj->pszNickName);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createint32(USR_GOLD, obj->iGold);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createint32(USR_GENDER, obj->cbGender);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createint32(USR_SCORE, obj->iScore);
	tlv_add(&pHead,pTLV);
	pTLV = tlv_createstr(USR_IMSI, obj->IMSI);
	tlv_add(&pHead,pTLV);

	char *_buffer=NULL;
	int _len = 0;

	entry_t* entry = obj->pEntryHead;
	while(entry){		
		_buffer = entry2Buffer(entry, &_len);
		pTLV=tlv_create(USR_ENTRY,_len,_buffer);
		tlv_add(&pHead,pTLV);
		entry = entry->next;
	}

	mail_t* mail = obj->headMail;
	while(mail){
		_buffer = mail2Buffer(mail, &_len);
		pTLV=tlv_create(USR_MAIL,_len,_buffer);
		tlv_add(&pHead,pTLV);
		mail = mail->next;
	}

	rank_t* rank = obj->headRank;
	while(rank){
		_buffer = rank2Buffer(rank, &_len);
		pTLV=tlv_create(USR_RANK,_len,_buffer);
		tlv_add(&pHead,pTLV);
		rank = rank->next;
	}

	trigger_t *tg = obj->headTG;
	while(tg){
		_buffer = trigger2Buffer(tg, &_len);
		pTLV=tlv_create(USR_TG,_len,_buffer);
		tlv_add(&pHead,pTLV);
		tg = tg->next;
	}
	
	int len = tlv_encode(pHead,NULL);
	char *buffer = (char*)malloc(len);
	memset(buffer,0x00,len);
	size = tlv_encode(pHead,buffer);
	tlv_destroy(pHead);
	
	if(len!=size){
		free(buffer);
		return NULL;
	}
	
	*piLen = len;
	return buffer;
}

void usrRelease(USR_T *obj)
{
	free(obj->ID);
	free(obj->pszNickName);

	entry_t* entry = obj->pEntryHead;
	entry_t* pTempE = NULL;
	while(entry){		
		pTempE = entry->next;
		entryRelease(entry);
		entry = pTempE;
	}

	mail_t* mail = obj->headMail;
	mail_t* pTempM = NULL;
	while(mail){
		pTempM = mail->next;
		mailRelease(mail);
		mail = pTempM;
	}

	rank_t* rank = obj->headRank;
	rank_t* pTempR = NULL;
	while(mail){
		pTempR = rank->next;
		rankRelease(rank);
		rank = pTempR;
	}

	trigger_t* tg = obj->headTG;
	trigger_t* pTempT = NULL;
	while(tg){
		pTempT = tg->next;
		triggerRelease(tg);
		tg = pTempT;
	}

	free(obj);
}
