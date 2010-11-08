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
#else
#include <sys/atomic_op.h>
#endif
/*#include "lapi_macro.h"*/
#include "ShmArray.h"
#include "lapi_itrace.h"

/*!
 * \brief Default constructor.
 */
ShmArray::ShmArray():shm(NULL) {};

/*!
 * \brief Initialization function.
 *
 * \TODO distinguish member count and byte count
 */
SharedArray::RC ShmArray::Init(const unsigned int member_cnt,
        const unsigned int key, const bool is_leader)
{
#ifndef __64BIT__
    ITRC(IT_BAR, "ShmArray: Not supported on 32Bit applications\n");
    return NOT_AVAILABLE;
#endif
    // if it is already initialized, then do nothing.
    if (status == READY) {
        return SUCCESS;
    }

    /// Main steps:
    this->is_leader  = is_leader;
    this->member_cnt = member_cnt;
    ITRC(IT_BAR, "ShmArray: initializing with %d members\n", member_cnt);

    /// - create shared memory or retrieve the existing one
    unsigned int  num_bytes = sizeof(Shm) + sizeof(Cacheline) * member_cnt;

    if (FAILED == ShmSetup(key, num_bytes, 300)) {
        ITRC(IT_BAR, "ShmArray: ShmSetup failed\n");
        return FAILED;
    }

    /// - attach to shared memory
    shm = (Shm *)shm_seg;

    /// - at the end, set the status to READY
    status = READY;

    ITRC(IT_BAR, "ShmArray: initialized\n");
    return SUCCESS;
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
    status = NOT_READY;
    ShmDestory();

    ITRC(IT_BAR, "ShmArray: Destroyed\n");
}

unsigned char      ShmArray::Load1(const int offset) const
{
    return shm->line[offset].byte;
}

unsigned short     ShmArray::Load2(const int offset) const
{
    unsigned short value;
    unsigned char *byte = (unsigned char *)&value;
    for (size_t i = 0; i < sizeof(value); i++)
        byte[i] = shm->line[offset+i].byte;
    return value;
}

unsigned int       ShmArray::Load4(const int offset) const
{
    unsigned int   value;
    unsigned char *byte = (unsigned char *)&value;
    for (size_t i = 0; i < sizeof(value); i++)
        byte[i] = shm->line[offset+i].byte;
    return value;
}

unsigned long long ShmArray::Load8(const int offset) const
{
    unsigned long long value;
    unsigned char *byte = (unsigned char *)&value;
    for (size_t i = 0; i < sizeof(value); i++)
        byte[i] = shm->line[offset+i].byte;
    return value;
}

void ShmArray::Store1(const int offset, const unsigned char val)
{
    isync();
    shm->line[offset].byte = val;
    lwsync();
}

void ShmArray::Store2(const int offset, const unsigned short val)
{
    assert(0 && "ShmArray::Store2 Not supported yet");
}

void ShmArray::Store4(const int offset, const unsigned int val)
{
    assert(0 && "ShmArray::Store4 Not supported yet");
}

void ShmArray::Store8(const int offset, const unsigned long long val)
{
    assert(0 && "ShmArray::Store8 Not supported yet");
}
