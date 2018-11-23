/*
* DBLite.h 
*
* The DataBase Lite store for app server
* server use TC as its core Engine
* complex maybe, but you can also use the api for app client
* TC uses file-mem map, be sure your platform support that
*
* Copyright (C) Listen.Li, 2018
*
*/
#ifndef DBLITE_H
#define DBLITE_H


#include "DPRoto.h"
#include "DBPack.h"


void* DBLite_get(char *pTblName, char *KEY);
bool  DBLite_put(char *pTblName, char *KEY, void *pObj);
void  DBLite_earse(char *pTblName, char *KEY);
int   DBLite_iterinit(char *pTblName);
char* DBLite_iternext(char *pTblName);

void  DBLite_flush();


#endif
