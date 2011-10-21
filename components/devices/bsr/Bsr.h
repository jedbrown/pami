#ifndef __components_devices_bsr_Bsr_h__
#define __components_devices_bsr_Bsr_h__

/**
 * \file components/devices/bsr/Bsr.h
 *
 * \brief Bsr class definition.
 *
 */

#include <sys/shm.h>
#include <sys/mman.h>
#ifndef _LAPI_LINUX
#include <sys/vminfo.h>
#endif
#include "SharedArray.h"

/**
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
 *       and ShmDestroy() functions.
 *
 */

class Bsr : public SharedArray
{
    public:
        Bsr(unsigned int mem_cnt, bool is_leader, size_t done_mask,
                void *shm_block, size_t shm_block_sz);
        ~Bsr();
        RC CheckInitDone(const unsigned int   job_key, 
                         const int            mem_id, 
                         const unsigned char  init_val);

        unsigned char      Load1(const int byte_offset) const;
        unsigned short     Load2(const int byte_offset) const;
        unsigned int       Load4(const int byte_offset) const;
        unsigned long long Load8(const int byte_offset) const;
        void Store1(const int byte_offset, const unsigned char val);
        void Store2(const int byte_offset, const unsigned short val);
        void Store4(const int byte_offset, const unsigned int val);
        void Store8(const int byte_offset, const unsigned long long val);

        static size_t GetCtrlBlockSz(unsigned int mem_cnt) {
            /* need pad structure to natural word alignment */
            size_t align_mask = (sizeof(size_t) - 1);
            return ((sizeof(Shm) + align_mask) & ~(align_mask));
        }
    private:
        size_t                  done_mask;      // non-zero value
        int                     bsr_id;         // BSR id
        unsigned char*          bsr_addr;       // BSR address
#ifdef _LAPI_LINUX 
        uint32_t                smask;
        uint32_t                fmask;
#endif
        enum BSR_SETUP_STATE {
            ST_NONE = 0,
            ST_BSR_ACQUIRED,   /* bsr_id acquired (on AIX, shmctl with SHM_BSR also succeeded) */
            ST_BSR_WAIT_ATTACH,/* attached to BSR (primed and ref++) and wait for others */
            ST_SUCCESS,
            ST_FAIL 
        } bsr_state; // state on local task

        struct Shm {
            // Flag to signal upper layer to clean up the shm
            // Must be at the 1st word
            volatile size_t       done_flag;
            // ref count; number of successful bsr setup
            volatile int          setup_ref;
            // BSR ID. Set by leader.
            volatile int          bsr_id;
            // Do we have BSR allocated? Updated by leader only
            volatile bool         bsr_acquired;
            // Do we need to abort BSR setup?
            volatile bool         setup_failed;
        }                        *shm; // shm block used to do internal
                                       // communication.

        // helper function
        void CleanUp();
#ifndef _LAPI_LINUX
        int  GetBsrUniqueKey(unsigned int j_key);
#endif
        bool GetBsrResource(unsigned int job_key);
        bool AttachBsr(int mem_id, unsigned char init_val);
        bool IsBsrReady();

        void ReleaseBsrResource(); // release bsr_id
        void DetachBsr();          // detach BSR from bsr_addr

        void SetDoneFlag() {
            shm->done_flag = done_mask;
        }
};
#endif
