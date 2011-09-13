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
        ShmArray();
	~ShmArray();
        RC CheckInitDone(const unsigned int   mem_cnt, 
                         const unsigned int   job_key, 
                         const uint64_t       unique_key,
                         const bool           leader, 
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

    private:
        unsigned int            shm_size;
        enum SHM_SETUP_STATE {
            ST_NONE = 0,
            ST_SHM_PROCESSING,
            ST_SHM_DONE,
            ST_SHM_CHECK_REF_CNT,
            ST_SUCCESS,
            ST_FAIL
        } shm_state;

        struct Shm {
            volatile unsigned      ready_cnt;   // number of member has finished the init
            volatile unsigned char shm_data[0]; // byte array of shm data
        };

        Shm   *shm;
        ShmArray::SHM_SETUP_STATE CheckShmSetup(
                                        const unsigned int  member_cnt, 
                                        const unsigned int  job_key, 
                                        const uint64_t      unique_key,
                                        const bool          is_leader);
        ShmArray::SHM_SETUP_STATE CheckShmDone();
        ShmArray::SHM_SETUP_STATE CheckShmRefCount(const int            member_id, 
                                                   const unsigned char  init_val);
        ShmArray::SHM_SETUP_STATE CheckShmReady();

};

#endif
