/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/*!
 * \file ShmArray.cpp
 *
 * \brief Implementation of ShmArray class.
 */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#ifdef _LAPI_LINUX
#include "lapi_linux.h"
#endif
#include "atomics.h"
#include "ShmArray.h"
#include "lapi_itrace.h"
#include "Arch.h"
#include "Memory.h"
#include "lapi_assert.h"
/*!
 * \brief Default constructor.
 */
ShmArray::ShmArray(unsigned int mem_cnt, bool is_leader, void *shm_block, size_t shm_block_sz):
    SharedArray(mem_cnt, is_leader, shm_block, shm_block_sz, "ShmArray"),
    is_last(false),
    shm_state(ST_NONE),
    shm((Shm*)shm_block),
    shm_size(shm_block_sz)
{
    assert(GetCtrlBlockSz(member_cnt) <= shm_block_sz);
    /* check alignment for variables that used by atomic */
    size_t align_mask = (sizeof(size_t) - 1);
    assert(((size_t)(&shm->ready_cnt) & align_mask) == 0);
};

/*!
 * \brief Nonblocking initialization function.
 *
 */
SharedArray::RC ShmArray::CheckInitDone(const unsigned int   job_key, 
                                        const int            mem_id, 
                                        const unsigned char  init_val)
{
    switch (shm_state) {
        case ST_NONE:
            /// Main steps:
            ITRC(IT_BSR, 
                    "ShmArray: member_cnt:%u is_leader:%d job_key:%u\n",
                    member_cnt, is_leader, job_key);
            ASSERT(shm_size >= sizeof(Shm)+member_cnt);
            shm = (Shm *)shm_seg;
            /// Initialize SHM region with init_val
            Store1(mem_id, init_val);
            fetch_and_add((atomic_p)&(shm->ready_cnt), 1);
            shm_state = ST_SHM_CHECK_REF_CNT;
            // fall through
        case ST_SHM_CHECK_REF_CNT:
            if (shm->ready_cnt == (int)this->member_cnt) {
                ITRC(IT_BSR, "ShmArray: Ready to use\n");
                return SUCCESS;
            } else {
                return PROCESSING;
            }
        default:
            assert(0 && "should not be here");
    }

    assert(0 && "should not be here");
    return PROCESSING;
}

/*!
 * \brief Destructor function.
 *
 * Detach from and remove the shared memory block (leader only)
 *
 * \return Bsr::SUCCESS
 */
ShmArray::~ShmArray()
{
    if (ST_NONE != shm_state) {
        int cnt = fetch_and_add((atomic_p)&(shm->ready_cnt), -1);
        if (cnt == 1) {
            is_last = true;
        }
        ITRC(IT_BSR, "ShmArray: Destroyed ready_cnt=%d->%d is_last=%d\n",
                cnt, cnt-1, is_last);
        assert(cnt > 0);
    } else {
        ITRC(IT_BSR, "ShmArray: Destroyed (ST_NONE)\n");
    }
}

unsigned char      ShmArray::Load1(const int byte_offset) const
{
    return shm->shm_data[byte_offset];
}

unsigned short     ShmArray::Load2(const int byte_offset) const
{
    return *((unsigned short*)(shm->shm_data + byte_offset));
}

unsigned int       ShmArray::Load4(const int byte_offset) const
{
    return *((unsigned int*)(shm->shm_data + byte_offset));
}

unsigned long long ShmArray::Load8(const int byte_offset) const
{
    return *((unsigned long long*)(shm->shm_data + byte_offset));
}

void ShmArray::Store1(const int byte_offset, const unsigned char val)
{
  // LAPI code had  isync();
    PAMI::Memory::sync<PAMI::Memory::instruction>();
    shm->shm_data[byte_offset] = val;
    // LAPI Code had  lwsync();
    PAMI::Memory::sync();
}

void ShmArray::Store2(const int byte_offset, const unsigned short val)
{
  // LAPI code had  isync();
    PAMI::Memory::sync<PAMI::Memory::instruction>();
    *((unsigned short*)(shm->shm_data + byte_offset)) = val;
    // LAPI Code had  lwsync();
    PAMI::Memory::sync();
}

void ShmArray::Store4(const int byte_offset, const unsigned int val)
{
    assert(0 && "ShmArray::Store4 Not supported yet");
}

void ShmArray::Store8(const int byte_offset, const unsigned long long val)
{
    assert(0 && "ShmArray::Store8 Not supported yet");
}
