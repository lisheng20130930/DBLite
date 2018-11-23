/*
* DBLsg.h 
*
* The DataBase Lite store for app client
* client one object as one file
* you can also use DBLite as your app api
*
* Copyright (C) Listen.Li, 2018
*
*/
#ifndef DBLSG_H
#define DBLSG_H


void* DBLsg_get(char *pTblName);
bool  DBLsg_put(char *pTblName, void *pObj);


#endif