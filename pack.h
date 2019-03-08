#ifndef DBPACK_H
#define DBPACK_H


char* obj2Buffer(_Field *pTbl, void *pObj, int *piLen);
void* buffer2Obj(_Field *pTbl, int oSize, char *buffer, int iLen, int *piSize);


#endif