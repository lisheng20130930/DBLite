/* This is an example File, You can mdofiy freely */

#ifndef PROTOB_H
#define PROTOB_H

// overlapitem overlap item types
enum{
	EOIBAOBOX = 0,  // 宝箱
};

// 道具(数目类的东西)
typedef struct _overlapitem_s
{
    int etype;  // 类型
    int Id; // ID
    int iNum; // 数目
}overlapitem_t;


// 实体类型定义
enum{
	EGOLD = 0, // gold
	EOVERLAPITEM, // OverlapItem	
};

typedef struct _entry_s
{
	struct _entry_s *next; // NEXT
    int etype;  // entry type
    union{
        overlapitem_t *overlapitm;
        int iGoldD;
    }u;
}entry_t;

enum{
	ENEW=0,
		EREAD=1,
};
typedef struct _mail_s
{
	struct _mail_s *next; // NEXT
	char *title;
	char *pszTime;
	char *content;
	int iStatus;
	entry_t *pHeadAttachment;
}mail_t;


typedef struct _rank_s
{
	struct _rank_s *next; // NEXT
	char *name;
	int cbGender;
	int iGold;
}rank_t;


typedef struct _trigger_s{
	struct _trigger_s *next;  // NEXT
	int eTriger;
	union{
		unsigned int firstPlayDate;
		unsigned int signLastDate;
		unsigned int weekEndLastDate;
		unsigned int monLastDate;
		unsigned int _3fightLastDate;
		unsigned int feedBackLastDate;
	}u;
}trigger_t;


typedef struct USR_S
{
	entry_t *pEntryHead; // 背包里的东西
	mail_t *headMail;
	trigger_t *headTG;
	rank_t *headRank;
	int iGold;
	int cbGender;
	int iScore; // 积分
	char *ID; // ID
	char *IMSI;
	char *pszNickName; // 昵称
	unsigned int iLastSynDate; // 最后一次同服务器同步的日期
}USR_T;



overlapitem_t* buffer2OverlapItem(char *buffer, int len);
char* overlapitem2Buffer(overlapitem_t *obj, int *piLen);
void overlapitemRelease(overlapitem_t *obj);

entry_t* buffer2Entry(char *buffer, int len);
char* entry2Buffer(entry_t *obj, int *piLen);
void entryRelease(entry_t *obj);

mail_t* buffer2Mail(char *buffer, int len);
char* mail2Buffer(mail_t *obj, int *piLen);
void mailRelease(mail_t *obj);

rank_t* buffer2Rank(char *buffer, int len);
char* rank2Buffer(rank_t* obj, int *piLen);
void rankRelease(rank_t *obj);

trigger_t* buffer2Trigger(char *buffer, int len);
char* trigger2Buffer(trigger_t *obj, int *piLen);
void triggerRelease(trigger_t *obj);

USR_T* buffer2Usr(char *buffer, int len);
char* usr2Buffer(USR_T *obj, int *piLen);
void usrRelease(USR_T *obj);



#endif