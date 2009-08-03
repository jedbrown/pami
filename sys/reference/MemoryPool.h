/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* OBJECT CODE ONLY SOURCE MATERIALS                                      */
/*                                                                        */
/* (C) COPYRIGHT International Business Machines Corp. 2008               */
/* All Rights Reserved                                                    */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
#ifndef _MEMORY_POOL_H
#define _MEMORY_POOL_H

/*
 * "@(#) 1.9 src/rsct/lapi/include/MemoryPool.h, lapi, rsct_rcf 08/12/08 13:39:10"
 *
 * IBM Confidential
 *
 * OCO Source Materials
 *
 * 5765-296
 * 5765-529
 *
 * (C) Copyright IBM Corp. 2008
 *
 * The source code for this program is not published or otherwise divested
 * of its trade secrets, irrespective of what has been deposited with the
 * U.S. Copyright Office.
 *
 */
/**************************************************************************
** Program           : LAPI memory pool
** File              : MemoryPool.h
** Start Date        : Feb. 2008
** Change History    : 
**
**            2008, Mar  5th:  HX: D150085 - Rewrite Put & Get on Am
**                  Mar 24th:  TJ: D150343 - Catch the out of memory error
**                  Jun  3rd:  TJ: F151033 - Remove union in the class
**                  Jun  3rd:  DH: F151033 - Merge new scaling code with lapi flow
**                  Jun 16th:  HT: F151069 - Add high water mark for Memory Pool
**                  Jun 16th:  TJ: F151069 - Initialize member variable "extra"
**                  Jul 21st:  TJ: D151920 - Added member function: Clear()
*/

#include <assert.h>
#include <string.h>

template<class T>
class MemoryPool
{
public:
    // TODO: add initial size
    MemoryPool() 
        : head(NULL), extra(0) 
    { 
        // TODO: use static assert
        //assert(sizeof(T) >= sizeof(void *)); 
        memset(&initializer, 0, sizeof(initializer));
        high_water_mark_count=0;
    }
    
    ~MemoryPool() 
    {
        this->Clear();
    }

    void Clear()
    {
        while (head) {
            Element *element = head;
            head = head->next;
            delete[] (char *)element;
        }
    }

    T *Allocate() 
    {
        T *element;
        if (head == NULL) {
            if (sizeof(T) + extra > sizeof(Element)) {
                element = (T*) new char [sizeof(T) + extra];
            } else {
                element = (T*) new char [sizeof(Element)];
            } 
            *element = initializer;
            high_water_mark_count++;
        } else {
            element = (T*)head;
            head = head->next;
        }
        return element;
    }

    void Free(T *block)
    {
        Element *element = (Element *)block;
        element->next = head;
        head = element;
    }

    size_t Size() const 
    { 
        size_t num_elements = 0;
        Element *ptr = head;
        while (ptr) {
            ptr = ptr->next;
            num_elements++;
        }
        return num_elements; 
    }

    int  GetHighWaterMarkCount() const { return high_water_mark_count;}

    bool IsEmpty() const { return (head == NULL); }

private:
    struct Element {
        Element  *next;
    };
    Element *head;
    int      high_water_mark_count;
protected:
    T        initializer;
    int      extra;
};

#endif
