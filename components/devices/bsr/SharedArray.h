#ifndef _SHAREDARRAY_H
#define _SHAREDARRAY_H

#include <stdlib.h>
/*!
 * \file SharedArray.h
 *
 * \brief SharedArray class definition.
 */

/*!
 * \brief SharedArray class
 *
 * A base class to encapsulate shared array data structure operations..
 */
class SharedArray
{
    public:
        /*!
         * \brief Return codes for api functions.
         */
        enum RC {
            SUCCESS = 0, ///< Operation successed.
            FAILED,      ///< Operation failed.
            NOT_AVAILABLE///< Shared array is not available.
        };

        /*!
         * \brief The status of the SharedArray object.
         *
         * NOT_READY is the default value when the object is constructed.
         */
        enum STATUS {
            NOT_READY = 0, /*!< Object is not ready to use. */
            READY = 1      /*!< Object is ready to use. */
        };

        /*!
         * \brief typedef for progress callback function
         */
        typedef void (*ProgressCb_t)(void* param);

        /*!
         * \brief SharedArray default constructor
         *
         * This constructor initializes member variables to default values.
         */
        SharedArray();

        /*!
         * \brief Default destructor.
         *
         * The function frees all the resources that associate with
         * this object.
         *
         */
        virtual ~SharedArray();

        /*!
         * \brief Initialization function.
         *
         * If this call returns Bsr::SUCCESS, then the object's status should
         * be SharedArray::READY. Otherwise, the status is
         * SharedArray::NOT_READY.
         *
         * \param member_cnt Number of members on the local node
         * \param key An unique key that will be used to establish a shared
         *            memory segment for internal data exchange.
         * \param is_leader Specifies if this is a leader on member.
         *
         * \return SharedArray::SUCCESS, SharedArray::FAILED
         */
        virtual RC Init(const unsigned int member_cnt, const unsigned int key,
                const bool is_leader)=0;

        /*!
         * \brief Loads one byte data from the array.
         *
         * \param offset a constant offset to indicate from which byte we
         *        retrieve the one byte of the data.
         */
        virtual unsigned char      Load1(const int offset) const =0;

        /*!
         * \brief Loads two bytes data from the array.
         *
         * \param offset a constant offset to indicate from which byte we
         *        retrieve the two bytes of the data.
         */
        virtual unsigned short     Load2(const int offset) const =0;

        /*!
         * \brief Loads four bytes data from the array.
         *
         * \param offset a constant offset to indicate from which byte we
         *        retrieve the four bytes of the data.
         */
        virtual unsigned int      Load4(const int offset) const =0;

        /*!
         * \brief Loads eight bytes data from the array.
         *
         * \param offset a constant offset to indicate from which byte we
         *        retrieve the eight bytes of the data.
         */
        virtual unsigned long long Load8(const int offset) const =0;

        /*!
         * \brief Store one byte data to the array.
         *
         * \param offset a constant offset to indicate starts from where
         *        we store the data.
         *
         * \param val the value to be stored.
         */
        virtual void Store1(const int offset, const unsigned char val)=0;

        /*!
         * \brief Store two bytes data to the array.
         *
         * \param offset a constant offset to indicate starts from where
         *        we store the data.
         *
         * \param val the value to be stored.
         */
        virtual void Store2(const int offset, const unsigned short val)=0;

        /*!
         * \brief Store four bytes data to the array.
         *
         * \param offset a constant offset to indicate starts from where
         *        we store the data.
         *
         * \param val the value to be stored.
         */
        virtual void Store4(const int offset, const unsigned int val)=0;

        /*!
         * \brief Store eight bytes data to the array.
         *
         * \param offset a constant offset to indicate starts from where
         *        we store the data.
         *
         * \param val the value to be stored.
         */
        virtual void Store8(const int offset, const unsigned long long val)=0;

        /*!
         * \brief Accessor function to the status of this object.
         */
        STATUS Status() const;

        /*!
         * \brief Accessor function to the is_leader flag of this object.
         */
        bool IsLeader() const;

        /*!
         * \brief Set the progress callback function.  This progress callback function
         *        will be used whenever there is a wait loop. If the callback is not
         *        set, nothing will be done in the wait loop.
         */
        void SetProgressCb(ProgressCb_t progress_cb, void* progress_cb_info);

    protected:
        /// Memory cache line size in bytes.
        static const int CACHE_LINE_SZ = 128;

        // Helper functions for derived classes:
        /*!
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
        RC ShmSetup(const unsigned int shm_key, const unsigned int size,
                const unsigned int timeout);

        /*!
         * \brief Destory the internal SHM segment
         *
         * This function detachs from the SHM segment and then remove the
         * SHM id from system if it is called by SHM master.
         *
         * \return SharedArray::SUCCESS
         */
        RC ShmDestory();

        // Member variables
        STATUS        status;     ///< Current object status.
        unsigned int  member_cnt; ///< Number of members on the local node.
        bool          is_leader;  ///< Indicates if this is a leader task.
        int           shm_id;     ///< The shm id for the created segment.
        void*         shm_seg;    ///< Pointer to the SHM segment.
        ProgressCb_t  progress_cb;  ///< Function pointer to idle processing.
        void*         progress_cb_info; ///< Pointer be passed into idel_proc.

    private:
        struct ShmCtrlBlock {
            volatile unsigned int ref_cnt; // number of member currently attached
        };
        ShmCtrlBlock* ctrl_block;
};

inline SharedArray::SharedArray():status(NOT_READY),
                  member_cnt(0),
                  is_leader(false),
                  shm_id(-1),
                  shm_seg(NULL),
                  progress_cb(NULL),
                  progress_cb_info(NULL),
                  ctrl_block(NULL){};

inline SharedArray::STATUS SharedArray::Status() const {
    return this->status;
}

inline bool SharedArray::IsLeader() const {
    return this->is_leader;
}

inline void
SharedArray::SetProgressCb(ProgressCb_t progress_cb, void* progress_cb_info) {
    this->progress_cb = progress_cb;
    this->progress_cb_info = progress_cb_info;
}

inline
SharedArray::~SharedArray()
{
};
#endif /* _SHAREDARRAY_H */
