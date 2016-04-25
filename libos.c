#include "libos.h"

char *we_cmmn_strndup(const char *s, int len)
{
    char *s_new;
    
    if (s != NULL) {
        s_new = (char*)malloc(len + 1);
        if (s_new != NULL) {
            memcpy(s_new, s, len);
            s_new[len] = '\0';
        }
    }
    else {
        s_new = (char*)malloc(1);
        if (s_new != NULL) {
            s_new[0] = '\0';
        }
    }
    
    return s_new;
}

int file2buff(char *pszFileName, char **ppszBuff, int *piLen)
{
    FILE *pFile = NULL;    
    char *pcFileBuff = NULL;
    int iInLen = 0;
    
    pFile = fopen(pszFileName, "rb");    
    if(0 == pFile)
    {
        return -1;
    }
    
    fseek(pFile, 0, SEEK_END);    
    iInLen = ftell(pFile);
    pcFileBuff = (char*)malloc(iInLen+1);
    memset(pcFileBuff, 0x00, iInLen+1);
    fseek(pFile, 0, SEEK_SET);    
    fread(pcFileBuff,iInLen, 1, pFile);    
    fclose(pFile);
    
    *ppszBuff = pcFileBuff;
    *piLen = iInLen;
    
    return 0;
}


char* we_str_merge(char *s, char* t)
{
	static char g_buffer[80]={0};
	strcpy(g_buffer,s);
	strcat(g_buffer,t);
	return g_buffer;
}

void we_getvalue(char* value, int len, char *line, char *key)
{
	char *pST = NULL;
	char *pED = NULL;
	int _len = 0;

	pST = strstr(line, key);
	if(pST){
		pST+=strlen(key);
		pED = strstr(pST,"&");
		if(!pED){
			pED=pST+strlen(pST);
		}
		if(pED-pST>0){
			_len = __min(len-1,pED-pST);
			memcpy(value,pST,_len);
			value[len-1]=0;
			return;
		}
	}
	
	memset(value,0x00,len);
}

char* Ip2Str(unsigned int ip, char *strip)
{
    unsigned int temp = 0;
    unsigned int IP_first = 0;
    unsigned int IP_second = 0;
    unsigned int IP_thrid = 0;
    unsigned int IP_fourth = 0;
    
    temp = ip << 8 * 3; 
    IP_first = temp >> 8 * 3;    
    temp = ip << 8 * 2; 
    IP_second= temp >> 8 * 3;    
    temp = ip << 8 * 1; 
    IP_thrid = temp >> 8 * 3;    
    IP_fourth=ip>> 8 * 3;
    
    sprintf(strip,"%d.%d.%d.%d",IP_first,IP_second,IP_thrid,IP_fourth);
    
    return strip;
}

int iptalbes(unsigned int ip)
{
	char *buffer = NULL;
	int len = 0;
	char szIP[32] = {0};
	int RET = (-1);

	Ip2Str(ip, szIP);

	file2buff("./DB/iptables.CFG", &buffer, &len);
	if(len>0){
		if(strstr(buffer,szIP)){
			RET = 0;
		}
		free(buffer);
	}
	
	return RET;
}