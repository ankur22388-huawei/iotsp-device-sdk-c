/**************************************************************************
copyright (C) Copyright 2015-2016 Cisco Systems Inc.; All Rights Reserved
History:
        03/18/2016 - Created.
******************************************************************************/

/*****************************************************************************************************************
* System Include Files
******************************************************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>

#include "iot_device_sdk_utils.h"

#define INVALIDVALUE 0xFFFFFFFF

/*****************************************************************************************************************
 *  Convert a externally used position, to an array index
 *****************************************************************************************************************/
static uint32_t positionToIndex( POBJLIST pObjList, uint32_t position )
{
  uint32_t index=0;

  if(pObjList) {
    for( index = 0; index < pObjList->listSize; index++ ) {
      if( pObjList->ppArray[ index ] != NULL ) {
        if( position == 0 ) {
          break;
        } else {
          position--;
        }
      }
    }
    if( position || index >= pObjList->listSize ) index = INVALIDVALUE;
  }
  return index;
}

/*****************************************************************************************************************
 *  Create a new object list that has initially enough room to hold 'initialSize'
 *  items.  The list will expand as needed.
 *****************************************************************************************************************/
POBJLIST Utils_ObjListCreate( uint32_t initialSize, uint32_t growRate )
{
    POBJLIST pObjList = (POBJLIST)malloc( sizeof( OBJLIST ) );

    if(pObjList) {
        memset( pObjList, 0, sizeof( OBJLIST ) );

        pObjList->listSize = initialSize;
        pObjList->growRate = growRate;
        pObjList->ppArray = malloc( sizeof( void *) * pObjList->listSize );

        Utils_ObjListClear( pObjList );
    }

    return pObjList;
}

/*****************************************************************************************************************
 *  Destroy the given object list
 *****************************************************************************************************************/
 void Utils_ObjListDestroy( POBJLIST pObjList )
{
    if(pObjList) {
        free( pObjList->ppArray );
        free( pObjList );
    }
}

/*****************************************************************************************************************
 *  Get a count of the number of items currently in the lsit
 *****************************************************************************************************************/
 uint32_t Utils_ObjListGetCount( POBJLIST pObjList )
{
    if(pObjList)
        return pObjList->listCount;
    else return 0;
}

/*****************************************************************************************************************
 *  Add a new item to the list
 *****************************************************************************************************************/
 void Utils_ObjListAdd( POBJLIST pObjList, void* pObj )
{
    uint32_t index = 0;

    if(pObjList && pObj) {
        pObjList->listCount++;

        if( pObjList->listCount > pObjList->listSize ){
            // time for a bigger list !
            void **ppOldList = pObjList->ppArray;
            uint32_t oldListSize = sizeof( void* ) * pObjList->listSize;

            pObjList->listSize += pObjList->growRate;

            pObjList->ppArray = malloc( sizeof( void *) * pObjList->listSize );
            memset( pObjList->ppArray, 0, sizeof( void *) * pObjList->listSize );

            memcpy( pObjList->ppArray, ppOldList, oldListSize );

            free( ppOldList );
        }

        // find the first available slot
        for( index = 0; index < pObjList->listSize; index++ ) {
            if( pObjList->ppArray[ index ] == NULL ) {
                pObjList->ppArray[ index ] = pObj;
                break;
            }
        }
    }
}

/*****************************************************************************************************************
 *  Get the 'index'th item in the list
 *****************************************************************************************************************/
 void* Utils_ObjListGetAt( POBJLIST pObjList, uint32_t position )
{
    uint32_t index = positionToIndex( pObjList, position );
    if(pObjList) {
        if( index != INVALIDVALUE ) {
            return pObjList->ppArray[ index ];
        } else {
            return NULL;
        }
    }
    return NULL;
}

/*****************************************************************************************************************
 *  Remove the 'index'th item in the list
 *****************************************************************************************************************/
 void Utils_ObjListRemoveAt( POBJLIST pObjList, uint32_t position )
{
    uint32_t index = positionToIndex( pObjList, position );
    if(pObjList) {
        if( index != INVALIDVALUE ) {
            pObjList->ppArray[ index ] = NULL;
            pObjList->listCount--;
        }
    }
}

#if 0
/*****************************************************************************************************************
 *  Replace the 'index'th item in the list.  The original item must not be numm
 *****************************************************************************************************************/
 void Utils_ObjListReplaceAt( POBJLIST pObjList, uint32_t position, void* pNewObj )
{
    uint32_t index = positionToIndex( pObjList, position );
    if(pObjList) {
        if( index != INVALIDVALUE ) {
            if( pObjList->ppArray[ index ] != NULL ) pObjList->ppArray[ index ] = pNewObj;
        }
    }
}
#endif 

/*****************************************************************************************************************
 *  Clear out the contents of the list, but leave the list size intact
 *****************************************************************************************************************/
 void Utils_ObjListClear( POBJLIST pObjList ) {
    if(pObjList) {
        pObjList->listCount = 0;
        if(pObjList->ppArray) {
            memset( pObjList->ppArray, 0, sizeof( void *) * pObjList->listSize );
        }
    }
}
