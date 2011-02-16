#ifndef _BSR_H
#define _BSR_H

/*!
 * \file Bsr.h
 *
 * \brief Bsr class definition.
 *
 */

#include <sys/shm.h>
#include <sys/vminfo.h>
#include "SharedArray.h"

/*!
 * \brief Bsr class
 *
 * A derived class for Barrier Synchronization Register (BSR) on Power
 * machines with BSR support - AIX 6.1.L and later
 *
 * 4KB is the smallest shared memory region size supported.
 * It is expected that 4KB should be sufficient for most users of BSR memory.
 *
 * When using BSR shared memory, P7 hardware supports 1-byte and 2-byte store
 * instructions, and any size load instructions.
 *
 * \note The progress callback function is only used in Init(), ShmSetup(),
 *       and ShmDestory() functions.
 *
 */
class Bsr : public SharedArray
{
    public:
        Bsr();
        ~Bsr();
        RC Init(const unsigned int member_cnt, const unsigned int key,
                const bool is_leader);
        unsigned char      Load1(const int offset) const;
        unsigned short     Load2(const int offset) const;
        unsigned int       Load4(const int offset) const;
        unsigned long long Load8(const int offset) const;
        void Store1(const int offset, const unsigned char val);
        void Store2(const int offset, const unsigned short val);
        void Store4(const int offset, const unsigned int val);
        void Store8(const int offset, const unsigned long long val);

    private:
        key_t                   bsr_key;        // BSR key
        int                     bsr_id;         // BSR id
        size_t                  bsr_size;       // BSR region size, min 4KB
        unsigned char*          bsr_addr;       // bsr address
        bool                    is_bsr_attached;


        enum BSR_SETUP_STATE {
            ST_NONE    = 0x00000000,
            ST_ATTACHED= 0x00000001,
            ST_FAIL    = 0x00009999
        };
        struct Shm {
            // current state of BSR setup process
            volatile BSR_SETUP_STATE bsr_setup_state;
            // ref count; number of successful bsr setup
            volatile unsigned int bsr_setup_ref;
            // BSR ID
            volatile int          bsr_id;
        };
        Shm*                      shm; // shm block used to do internal
                                       // communication.

        // helper function
        void CleanUp();
};
#endif
