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
            NOT_AVAILABLE///< Shared array is not available.
        };

        /**
         * \brief SharedArray default constructor
         *
         * This constructor initializes member variables to default values.
         */
        SharedArray();

        /**
         * \brief Default destructor.
         *
         * The function frees all the resources that associate with
         * this object.
         *
         */
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
         * \param unique_key A unique number generated at the time the geometry
         *                   is created to generate an unique SHM key
         * \param leader     Specifies if this is a leader member.
         * \param mem_id     Specifies the unique id (0 based) within the group
         *                   This is used by BSR priming only
         * \param init_val   Initial value for each task to perform barrier op
         *                   This is used by BSR priming only
         *
         * \return SharedArray::SUCCESS, SharedArray::FAILED
         */
        virtual RC CheckInitDone(const unsigned int   mem_cnt, 
                                 const unsigned int   job_key, 
                                 const uint64_t       unique_key,
                                 const bool           leader, 
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

    protected:
        /// Memory cache line size in bytes.
        static const int CACHE_LINE_SZ = 128;

        // Helper functions for derived classes:
        /**
         * \brief Helper function to setup shared memory segment.
         *
         * This SHM segment is used for internal communication.
         * This protected function creates the SHM segment of the given
         * size and attaches to it. If the function could not finish in
         * the period of timeout, it returns FAILED.
         *
         * \param shm_key Shared memory key to use.
         * \param size    Number of bytes to allocate.
         * \param timeout Number of seconds to wait before ShmSetup returns
         *                a failure.
         *
         * \return SharedArray::SUCCESS, SharedArray::FAILED
         * \note This function must be called after the SharedArray::member_cnt is
         *       set.
         */
        RC IsPosixShmSetupDone(const unsigned int size);

        /**
         * \brief Destroy the internal SHM segment
         *
         * This function detachs from the SHM segment and then remove the
         * SHM id from system if it is called by SHM master.
         *
         * \return SharedArray::SUCCESS
         */
        RC PosixShmDestroy();

        // Member variables
        unsigned int  member_cnt; ///< Number of members on the local node.
        bool          is_leader;  ///< Indicates if this is a leader task.
        int           shm_id;     ///< The shm id for the created segment.
        string       *shm_str;    ///< The shm string for POSIX shm
        int           shm_size;   ///< The POSIX shm size
        void*         shm_seg;    ///< Pointer to the SHM segment.

    private:
        enum SETUP_STATE {
            ST_NONE = 0,
            ST_ATTACHED,
            ST_PASSED,
            ST_FAILED
        } setup_state;
        struct ShmCtrlBlock {
            volatile unsigned int ref_cnt; // number of member currently attached
        };
        ShmCtrlBlock* ctrl_block;
        SharedArray::SETUP_STATE PosixShmAllAttached();
        SharedArray::SETUP_STATE PosixShmAttach(const unsigned int size);

};

inline SharedArray::SharedArray():
                  setup_state(ST_NONE),
                  member_cnt(0),
                  is_leader(false),
                  shm_id(-1),
                  shm_seg(NULL),
                  ctrl_block(NULL){};

inline bool SharedArray::IsLeader() const {
    return this->is_leader;
}
#endif /* _SHAREDARRAY_H */
