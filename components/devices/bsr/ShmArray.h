/**
 * \file components/devices/bsr/ShmArray.h
 *
 * \brief ShmArray class definition.
 */
#ifndef __components_devices_bsr_ShmArray_h__
#define __components_devices_bsr_ShmArray_h__

#include "SharedArray.h"

/**
 * \brief ShmArray class
 *
 * A derived class to simulate Barrier Synchronization Register (BSR) device
 * operations using shared memory.
 */
class ShmArray : public SharedArray
{
    public:
        ShmArray(unsigned int mem_cnt, bool is_leader, void *shm_block, size_t shm_block_sz);
       ~ShmArray();
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
            return (sizeof(Shm) + mem_cnt);
        }

        /* for Checkpoint support */
        bool Checkpoint(int byte_offset) { (void)byte_offset;return true; } // nothing needed
        bool Restart   (int byte_offset) { (void)byte_offset;return true; } // nothing needed
        bool Resume    (int byte_offset) { (void)byte_offset;return true; } // nothing needed

    private:
        bool                       is_last;
        enum SETUP_STATE {
            ST_NONE = 0,
            ST_SHM_CHECK_REF_CNT
        } shm_state;

        struct Shm {
            volatile int           ready_cnt;   // must be at the 1st 4 bytes;
                                                // number of member has finished the init
            volatile unsigned char shm_data[0]; // byte array of shm data
        }                         *shm;
        unsigned int               shm_size;
};

#endif

