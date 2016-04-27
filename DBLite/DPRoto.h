#ifndef PROTOB_H
#define PROTOB_H


#define USRDAT  "Usr.DAT"


typedef struct _Treasure_s
{
	int iGold;
}Treasure_t;

void treasueRelease(Treasure_t *pTreasure);

typedef struct _USR_T
{
	int cbGender;
	char *ID;
	Treasure_t *pTreasure;
}USR_T;

void usrRelease(USR_T *pUsr);


#endif