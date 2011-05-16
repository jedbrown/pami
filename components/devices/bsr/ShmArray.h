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
        RC Init(const unsigned int member_cnt,
                const unsigned int group_id, const unsigned int job_key, 
                const bool is_leader, const int member_id, const unsigned char init_val);
        unsigned char      Load1(const int offset) const;
        unsigned short     Load2(const int offset) const;
        unsigned int       Load4(const int offset) const;
        unsigned long long Load8(const int offset) const;
        void Store1(const int offset, const unsigned char val);
        void Store2(const int offset, const unsigned short val);
        void Store4(const int offset, const unsigned int val);
        void Store8(const int offset, const unsigned long long val);

    private:
        struct Cacheline {
            volatile char byte;
            char          pad[SharedArray::CACHE_LINE_SZ-1];// a full cache line
        };

        struct Shm {
            volatile unsigned ready_cnt; // number of member has finished the init
            Cacheline         line[0];
        };

        Shm   *shm;
};

#endif
