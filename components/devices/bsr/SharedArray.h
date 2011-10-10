#ifndef __components_devices_bsr_SharedArray_h__
#define __components_devices_bsr_SharedArray_h__

#include <stdlib.h>
#include <string>
#include <stdint.h>
using namespace std;

/**
 * \file components/devices/bsr/SharedArray.h
 *
 * \brief SharedArray class definition.
 */

/**
 * \brief SharedArray class
 *
 * A base class to encapsulate shared array data structure operations..
 */
class SharedArray
{
    public:
        /**
         * \brief Return codes for api functions.
         */
        enum RC {
            SUCCESS = 0, ///< Operation successed.
            PROCESSING,  ///< Operation not finished.
            FAILED,      ///< Operation failed.
        };

        /**
         * \brief SharedArray default constructor
         *
         * This constructor initializes member variables to default values.
         */
        SharedArray(unsigned int mem_cnt, bool is_leader,
                void* shm_block, size_t shm_block_sz, const char* name);

        virtual ~SharedArray();

        /**
         * \brief Initialization function.
         *
         * If this call returns SharedArray::SUCCESS, then the object's status should
         * be SharedArray::READY. Otherwise, the status is
         * SharedArray::NOT_READY.
         *
         * \param member_cnt Number of members on the local node
         * \param job_key    Job key is used to communicate with PNSD and also
         *                   used to generate an unique SHM key 
         * \param leader     Specifies if this is a leader member.
         * \param mem_id     Specifies the unique id (0 based) within the group
         *                   This is used by BSR priming only
         * \param init_val   Initial value for each task to perform barrier op
         *                   This is used by BSR priming only
         *
         * \return SharedArray::SUCCESS, SharedArray::FAILED
         */
        virtual RC CheckInitDone(const unsigned int   job_key, 
                                 const int            mem_id, 
                                 const unsigned char  init_val)=0;

        /**
         * \brief Loads one byte data from the array.
         *
         * \param offset a constant offset to indicate from which byte we
         *        retrieve the one byte of the data.
         */
        virtual unsigned char      Load1(const int offset) const =0;

        /**
         * \brief Loads two bytes data from the array.
         *
         * \param offset a constant offset to indicate from which byte we
         *        retrieve the two bytes of the data.
         */
        virtual unsigned short     Load2(const int offset) const =0;

        /**
         * \brief Loads four bytes data from the array.
         *
         * \param offset a constant offset to indicate from which byte we
         *        retrieve the four bytes of the data.
         */
        virtual unsigned int      Load4(const int offset) const =0;

        /**
         * \brief Loads eight bytes data from the array.
         *
         * \param offset a constant offset to indicate from which byte we
         *        retrieve the eight bytes of the data.
         */
        virtual unsigned long long Load8(const int offset) const =0;

        /**
         * \brief Store one byte data to the array.
         *
         * \param offset a constant offset to indicate starts from where
         *        we store the data.
         *
         * \param val the value to be stored.
         */
        virtual void Store1(const int offset, const unsigned char val)=0;

        /**
         * \brief Store two bytes data to the array.
         *
         * \param offset a constant offset to indicate starts from where
         *        we store the data.
         *
         * \param val the value to be stored.
         */
        virtual void Store2(const int offset, const unsigned short val)=0;

        /**
         * \brief Store four bytes data to the array.
         *
         * \param offset a constant offset to indicate starts from where
         *        we store the data.
         *
         * \param val the value to be stored.
         */
        virtual void Store4(const int offset, const unsigned int val)=0;

        /**
         * \brief Store eight bytes data to the array.
         *
         * \param offset a constant offset to indicate starts from where
         *        we store the data.
         *
         * \param val the value to be stored.
         */
        virtual void Store8(const int offset, const unsigned long long val)=0;

        /**
         * \brief Accessor function to the is_leader flag of this object.
         */
        bool IsLeader() const;

        const char*   name;
    protected:
        // Member variables
        unsigned int  member_cnt;   ///< Number of members on the local node.
        bool          is_leader;    ///< Indicates if this is a leader task.
        int           shm_size;     ///< The shm block size
        void*         shm_seg;      ///< Pointer to the SHM segment.
};

inline SharedArray::SharedArray(unsigned int mem_cnt, bool is_leader,
        void *shm_block, size_t shm_block_sz, const char *name):
                  member_cnt(mem_cnt),
                  is_leader(is_leader),
                  shm_size(shm_block_sz),
                  shm_seg(shm_block),
                  name(name) {};

inline SharedArray::~SharedArray() {};

inline bool SharedArray::IsLeader() const {
    return this->is_leader;
}

#endif /* _SHAREDARRAY_H */
