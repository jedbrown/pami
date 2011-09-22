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
/*!
 * \brief Default constructor.
 */
ShmArray::ShmArray():
    shm(NULL),
    shm_size(0),
    shm_state(ST_NONE){};

/*!
 * \brief Initialization function.
 *
 * \TODO distinguish member count and byte count
 */
ShmArray::SHM_SETUP_STATE ShmArray::CheckShmReady()
{
    if (shm->ready_cnt == this->member_cnt) {
        ITRC(IT_BSR, "ShmArray(FAILOVER): initialized\n");
        return ST_SUCCESS;
    } else {
        return ST_SHM_CHECK_REF_CNT;
    }
}

ShmArray::SHM_SETUP_STATE ShmArray::CheckShmRefCount(
        const int member_id, const unsigned char init_val)
{
    /// Initialize SHM region with init_val
    Store1(member_id, init_val);

    fetch_and_add((atomic_p)&(shm->ready_cnt), 1);

    return CheckShmReady();
}

ShmArray::SHM_SETUP_STATE ShmArray::CheckShmDone()
{
    RC rc = IsPosixShmSetupDone(shm_size);
    switch (rc) {
        case SUCCESS:
            /// - attach to shared memory
            shm = (Shm *)shm_seg;

            ITRC(IT_BSR, "ShmArray(FAILOVER): %s ShmSetup with key <%s> PASSED.\n",
                    (this->is_leader)?"LEADER":"FOLLOWER",
                    this->shm_str);
            return ST_SHM_DONE;
        case PROCESSING:
            return ST_SHM_PROCESSING;
        default:
            ITRC(IT_BSR, "ShmArray(FAILOVER): %s ShmSetup with key <%s> FAILED"
                    " with rc %d.\n",
                    (this->is_leader)?"LEADER":"FOLLOWER",
                    this->shm_str, rc);
            return ST_FAIL;
    }
}

ShmArray::SHM_SETUP_STATE ShmArray::CheckShmSetup(
        const unsigned int member_cnt, const unsigned int job_key, 
        const uint64_t unique_key, const bool is_leader)
{
    sprintf(this->shm_str, "/SHM_%d_%llu", job_key, unique_key);
    ITRC(IT_BSR, "ShmArray(FAILOVER): Unique key string for POSIX shm setup <%s>\n",
            this->shm_str);

    /// Main steps:
    this->is_leader  = is_leader;
    this->member_cnt = member_cnt;
    ITRC(IT_BSR, "ShmArray(FAILOVER): initializing with %d members\n", member_cnt);

    /// - create shared memory or retrieve the existing one
    shm_size = sizeof(Shm) + member_cnt;

    return CheckShmDone();
}

SharedArray::RC ShmArray::CheckInitDone(const unsigned int   mem_cnt, 
                                        const unsigned int   job_key, 
                                        const uint64_t       unique_key,
                                        const bool           leader, 
                                        const int            mem_id, 
                                        const unsigned char  init_val)
{
    bool advance = true;
    while (advance) {
        advance = false;
        switch (shm_state) {
            case ST_NONE:
                shm_state = CheckShmSetup(mem_cnt, 
                                          job_key, 
                                          unique_key,
                                          is_leader);
                advance = (shm_state != ST_NONE);
                break;
            case ST_SHM_PROCESSING:
                shm_state = CheckShmDone();
                advance = (shm_state != ST_SHM_PROCESSING);
                break;
            case ST_SHM_DONE:
                shm_state = CheckShmRefCount(mem_id, init_val);
                advance = (shm_state != ST_SHM_DONE);
                break;
            case ST_SHM_CHECK_REF_CNT:
                shm_state = CheckShmReady();
                advance = (shm_state != ST_SHM_CHECK_REF_CNT);
                break;
            case ST_SUCCESS:
                return SUCCESS;
            case ST_FAIL:
                return FAILED;
        }
    }

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
    ITRC(IT_BSR, "ShmArray(FAILOVER): Destroyed\n");
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
