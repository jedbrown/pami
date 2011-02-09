/*!
 * \file ShmArray.h
 *
 * \brief ShmArray class definition.
 */
#ifndef _SHMARRAY_H
#define _SHMARRAY_H

#include "SharedArray.h"

/*!
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
        struct Cacheline {
            volatile char byte;
            char          pad[SharedArray::CACHE_LINE_SZ-1];// a full cache line
        };

        struct Shm {
            Cacheline     line[0];
        };

        Shm   *shm;
};

#endif