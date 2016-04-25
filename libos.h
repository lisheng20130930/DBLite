#ifndef LIBOS_H
#define LIBOS_H

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "time.h"
#include "stdbool.h"

#ifndef NULL
#define NULL  ((void*)0)
#endif

#ifndef __min
#define __min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef __max
#define __max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define TRUE  (1)
#define FALSE (0)

#define cp_zfree(p) if(p){free(p);}

#ifndef WIN32
#define stricmp strcasecmp
#define _vsnprintf vsnprintf
#else
#define SIGHUP   (1)
#endif


char* we_cmmn_strndup(const char *s, int len);

int file2buff(char *pszFileName, char **ppszBuff, int *piLen);

char* we_str_merge(char *s, char* t);

void we_getvalue(char* value, int len, char *line, char *key);

char* Ip2Str(unsigned int ip, char *strip);

int iptalbes(unsigned int ip);

#endif