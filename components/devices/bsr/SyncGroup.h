/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
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
         * \brief Return codes for api functions.
         */
        enum RC {
            SUCCESS = 0, ///< Operation successed.
            PROCESSING,  ///< Operation in process
            FAILED       ///< Operation failed.
        };

        SyncGroup();
        SyncGroup(unsigned int mem_id, unsigned int mem_cnt);
        virtual     ~SyncGroup();
        virtual void BarrierEnter() =0;
        virtual void BarrierExit()  =0;
        void         Barrier();
        unsigned int GetMemberId() const;
        unsigned int GetMemberCount() const;
        bool         IsInitialized() const;

    protected:
        unsigned int  member_cnt;
        unsigned int  member_id;
        string        group_desc;       // description of the group
};

/**
 * \brief Default constructor.
 */
inline
SyncGroup::SyncGroup():
    member_cnt(0),
    member_id(0),
    group_desc("N/A")
{
};

inline
SyncGroup::SyncGroup(unsigned int mem_id, unsigned int mem_cnt):
    member_cnt(mem_cnt),
    member_id(mem_id),
    group_desc("N/A")
{
}

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
    BarrierEnter();
    BarrierExit();
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
#endif /* _SYNCGROUP_H_ */
