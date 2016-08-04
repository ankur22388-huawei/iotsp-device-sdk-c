/**************************************************************************
copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
History:
        03/18/2016 - Created.
******************************************************************************/

#ifndef UTILS_H_
#define UTILS_H_

#ifdef  __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdlib.h>

/*****************************************************************************************************************
* Structures
******************************************************************************************************************/
typedef struct _objlist
{
  uint32_t    listSize;       // the size of the list
  uint32_t    growRate;       // when more space is needed, grow by this amount
  uint32_t    listCount;      // number of items in the list
  void**      ppArray;        // pointers to the objects
}OBJLIST, *POBJLIST;

POBJLIST Utils_ObjListCreate( uint32_t initialSize, uint32_t growRate );
void Utils_ObjListDestroy( POBJLIST pObjList );
uint32_t Utils_ObjListGetCount( POBJLIST pObjList );
void Utils_ObjListAdd( POBJLIST pObjList, void* pObj );
void* Utils_ObjListGetAt( POBJLIST pObjList, uint32_t position );
//void Utils_ObjListReplaceAt( POBJLIST pObjList, uint32_t position, void* pNewObj );
void Utils_ObjListRemoveAt( POBJLIST pObjList, uint32_t position );
void Utils_ObjListClear( POBJLIST pObjList );

#ifdef  __cplusplus
}
#endif

#endif /* UTILS_H_ */
