/**
 * \file components/devices/bsr/SyncGroup.h
 * \brief Class declaration for SyncGroup class.
 */
#ifndef __components_devices_bsr_SyncGroup_h__
#define __components_devices_bsr_SyncGroup_h__

#include <stdlib.h>
#include <string>
#include "util/common.h"
using namespace std;
/**
 * \brief SyncGroup class
 *
 * An object that represents a synchronizable group.
 */
class SyncGroup {
    public:
        /**
         * \brief typedef for progress callback function
         */
        typedef void (*ProgressCb_t)(void* param);

        /**
         * \brief Return codes for api functions.
         */
        enum RC {
            SUCCESS = 0, ///< Operation successed.
            PROCESSING,  ///< Operation in process
            FAILED       ///< Operation failed.
        };

        SyncGroup();
        virtual      ~SyncGroup();
        virtual RC   CheckInitDone(const unsigned int   member_cnt,
                                   const unsigned int   job_key, 
                                   const uint64_t       unique_key,
                                   const unsigned int   member_id, 
                                   void*                extra_arg)=0;
        virtual void BarrierEnter() =0;
        virtual void BarrierExit()  =0;
        void         Barrier();
        unsigned int GetMemberId() const;
        unsigned int GetMemberCount() const;
        bool         IsInitialized() const;
        void         SetProgressCb(ProgressCb_t progress_cb, void* progress_cb_info);
        virtual void _Dump() const;

    protected:
        unsigned int  member_cnt;
        unsigned int  group_id;
        unsigned int  member_id;
        bool          initialized;
        string        group_desc;       // description of the group
        ProgressCb_t  progress_cb;      // Function pointer for progress driving
        void*         progress_cb_info; // Pointer be passed into progress_cb
};

// Inline functions

/**
 * \brief Default constructor.
 *
 * This function only sets the initialized flag to false. User has to call
 * the SyncGroup::Init function prior any other operations.
 */
inline
SyncGroup::SyncGroup():
    member_cnt(0),
    group_id(0),
    member_id(0),
    initialized(false),
    group_desc("No description."),
    progress_cb(NULL),
    progress_cb_info(NULL)
{
};

/**
 * \fn virtual SyncGroup::RC SyncGroup::Init(const int member_cnt, const int group_id, const int member_id, void* extra_arg) =0
 *
 * \brief Initialize function.
 * Initalize internal variables and allocate mempry spaces. Every member
 * prcess that want to be in the group has to initialize the SyncGroup with the
 * same \em member_cnt and \em group_id values.
 *
 * \param member_cnt Number of members will be in the group.
 * \param group_id   An UNIQUE id that identifies the group and will be used
 *                   as a key to create internal shared memory segment.
 * \param member_id  An unique id to identify the member who creates the
 *                   object. The value should be in the range of
 *                   [0..member_cnt)
 * \param extra_arg  A pointer to any user defined data. Can be NULL if not
 *                   needed.
 *
 * \return SyncGroup::SUCCESS SyncGroup::FAILED
 */


/**
 * \brief Default destroctor.
 *
 * By default, it does nothing. Derived class should use it to handle the
 * releasing of resources when the object is being destroyed.
 */
inline
SyncGroup::~SyncGroup()
{
};

/**
 * \fn virtual void SyncGroup::BarrierEnter() =0
 * \brief BarrierEnter function.
 *
 * This function won't return for the root task unless all
 * members have entered the barrier. For the rest of tasks,
 * this function returns right away.
 */

/**
 * \fn virtual void SyncGroup::BarrierExit() =0
 * \brief BarrierExit function.
 *
 * This function won't return for non-root tasks unless
 * root task has entered this function. For the root task, this
 * function returns right away.
 */

/**
 * \brief Barrier function.
 *
 * This function won't return unless all members have entered.
 */
inline
void SyncGroup::Barrier()
{
    PAMI_assert(initialized && "SyncGroup: Object is not initialized");
    BarrierEnter();
    BarrierExit();
}

/**
 * \brief Accessor function to check if the object has been initialied.
 * \return true, false
 */
inline
bool SyncGroup::IsInitialized() const
{
    return this->initialized;
}

/**
 * \brief Accessor function to get the number of members in this group.
 *
 * \return A unsigned integer.
 */
inline
unsigned int SyncGroup::GetMemberCount() const {
    return this->member_cnt;
}

/**
 *\brief Accessor function to get the member_id value.
 *
 *\return A unsigned integer in range of 0 to (member_cnt-1).
 */
inline
unsigned int SyncGroup::GetMemberId() const {
    return this->member_id;
}

/**
 * \brief Set the progress callback function.  This progress callback function
 *        will be used whenever there is a wait loop. If the callback is not
 *        set, nothing will be done in the wait loop.
 */
inline
void SyncGroup::SetProgressCb(SyncGroup::ProgressCb_t progress_cb, void* progress_cb_info) {
    this->progress_cb = progress_cb;
    this->progress_cb_info = progress_cb_info;
}

/**
 * \internal
 * \brief A function to dump object current states to stdout for debugging.
 */
inline
void SyncGroup::_Dump() const {
    printf("SyncGroup dump:\n");
    printf("\tObject address (this) = %p\n", this);
    printf("\tthis->member_cnt      = %u\n", this->member_cnt);
    printf("\tthis->group_id        = %0x\n", this->group_id);
    printf("\tthis->member_id       = %0x\n", this->member_id);
    printf("\tthis->initialized     = %s\n", (this->initialized)?"true":"false");
    // don't pirnt out the group_desc in base class
    //printf("\tthis->group_desc      = \"%s\"\n", this->group_desc);
    printf("\tthis->progress_cb     = %p\n", this->progress_cb);
    printf("\tthis->progress_cb_info= %p\n", this->progress_cb_info);
}
#endif /* _SYNCGROUP_H_ */
