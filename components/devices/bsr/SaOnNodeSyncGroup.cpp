#include "SaOnNodeSyncGroup.h"
#include "BsrP6.h"
#include "ShmArray.h"
#include <stdlib.h>
#include <new>
#include <string.h>
#include "lapi_itrace.h"

SyncGroup::RC SaOnNodeSyncGroup::Init(
        const unsigned int member_cnt, const unsigned int group_id,
        const unsigned int member_id, void* param)
{
    ASSERT (param != NULL);
    Param_t* in_param = (Param_t*) param;
    this->multi_load = in_param->multi_load;
    if(member_cnt == 0) {
        ITRC(IT_SGRP, "SaOnNodeSyncGroup: Invalid member_cnt %d\n", member_cnt);
        return FAILED;
    }

    this->member_cnt = member_cnt;
    this->group_id   = group_id;
    this->member_id  = member_id;
    /* if there is only one member in the group, we do nothing. */
    if (member_cnt == 1) {
        initialized = true;
        return SUCCESS;
    }
    try {
        mask[0] = new unsigned char[member_cnt];
        memset(mask[0], 0, member_cnt);
        mask[0][0] = 1;
        mask[1] = new unsigned char[member_cnt];
        memset(mask[1], (unsigned char)0x1, member_cnt);
        mask[1][0] = 0;
    } catch (std::bad_alloc e) {
        // out of memory; should still work as long as multi-load is not used.
        ITRC(IT_SGRP,
                "SaOnNodeSyncGroup: Warning!! "
                "Out of memory. Multi-byte load is not allowed.\n");
        delete [] mask[0];
        delete [] mask[1];
    }

    // initialize SharedArray object
    ///TODO: How do we distinguish the group_id for BsrP6, BsrP7, or ShmArray
    // temporary fix start
    const unsigned int BSRP6ID    = 0x00000096; //p6
    const unsigned int SHMARRAYID = 0x00000054; //SA
    const bool         is_leader  = (this->member_id == 0);
    // temporary fix end
    SharedArray::RC sa_rc;
    try {
        sa = new BsrP6;
        // we don't support checkpoint and BSR
        if (!in_param->use_shm_only) {
            sa_rc = sa->Init(member_cnt, (group_id + BSRP6ID), is_leader);
        } else {
            sa_rc = SharedArray::NOT_AVAILABLE;
        }
        if (SharedArray::SUCCESS == sa_rc) {
            ITRC(IT_SGRP, "(%d)SaOnNodeSyncGroup: Using BsrP6\n", member_id);
            this->group_desc = "SharedArray:BsrP6";
        } else {
            delete sa;
            sa = NULL;
            // Cannot create a BsrP6 object, then try a ShmArray object
            sa = new ShmArray;
            if (SharedArray::SUCCESS ==
                    sa->Init(member_cnt, (group_id + SHMARRAYID), is_leader)) {
                ITRC(IT_SGRP, "(%d)SaOnNodeSyncGroup: Using ShmArray\n",
                        member_id);
                this->group_desc = "SharedArray:ShmArray";
            } else {
                delete sa;
                sa = NULL;
                // Cannot create a ShmArray object. No SharedArray object
                // available for using.
                ITRC(IT_SGRP,
                        "(%d)SaOnNodeSyncGroup: Cannot create SharedArray obj\n",
                        member_id);
                delete [] mask[0];
                delete [] mask[1];
                return FAILED;
            }
        }
    } catch (std::bad_alloc e) {
        sa = NULL;
        // TODO: if both BsrP6 and ShmArray failed, we should set sa=NULL
        // and use on node FIFO flow instead.
        ITRC(IT_SGRP, "(%d)SaOnNodeSyncGroup: Out of memory.\n", member_id);
        return FAILED;
    } catch (...) {
        delete [] mask[0];
        delete [] mask[1];
        ITRC(IT_SGRP, "SaOnNodeSyncGroup: Unexpected exception caught.\n");
        return FAILED;
    }

    ITRC(IT_SGRP, "SaOnNodeSyncGroup: Initialized with member_cnt=%d\n", member_cnt);
    initialized = true;
    return SUCCESS;
}

/*!
 * \brief Default destructor.
 */
SaOnNodeSyncGroup::~SaOnNodeSyncGroup()
{
    delete [] mask[0];
    delete [] mask[1];
    delete sa;
}

void SaOnNodeSyncGroup::BarrierEnter()
{
    ITRC(IT_SGRP, "SaOnNodeSyncGroup: Entering BarrierEnter()\n");
    ASSERT(this->initialized);
    // if only one member in group
    if (this->member_cnt == 1) {
        ITRC(IT_SGRP, "SaOnNodeSyncGroup: Leaving BarrierEnter()\n");
        return;
    }

    // flip the sequence number
    seq = !seq;
    if (member_id == 0) {
        if (multi_load) {
            ITRC(IT_SGRP,
                    "SaOnNodeSyncGroup: Wait respons from follower (multi_load)\n");
            // assume multi_load is always supported
            unsigned int i=0;
            if (progress_cb) {
                for (i = 0; i + 8 <= member_cnt; i += 8)
                    while (sa->Load8(i) != *(unsigned long long*)(mask[seq]+i))
                        (*progress_cb)(progress_cb_info);

                for (; i + 4 <= member_cnt; i+= 4)
                    while (sa->Load4(i) != *(unsigned int*)(mask[seq]+i))
                        (*progress_cb)(progress_cb_info);

                for (; i + 2 <= member_cnt; i+= 2)
                    while (sa->Load2(i) != *(unsigned short*)(mask[seq]+i))
                        (*progress_cb)(progress_cb_info);

                // use 1-byte loads to finish the rest
                if (i == 0) i = 1;
                for (; i < member_cnt; i++)
                    while (sa->Load1(i) != seq)
                        (*progress_cb)(progress_cb_info);
            } else {
                for (i = 0; i + 8 <= member_cnt; i += 8)
                    while (sa->Load8(i) != *(unsigned long long*)(mask[seq]+i));

                for (; i + 4 <= member_cnt; i+= 4)
                    while (sa->Load4(i) != *(unsigned int*)(mask[seq]+i));

                for (; i + 2 <= member_cnt; i+= 2)
                    while (sa->Load2(i) != *(unsigned short*)(mask[seq]+i));

                // use 1-byte loads to finish the rest
                if (i == 0) i = 1;
                for (; i < member_cnt; i++)
                    while (sa->Load1(i) != seq);
            }
            ITRC(IT_SGRP,
                    "SaOnNodeSyncGroup: Got respons from follower (multi_load)\n");
        } else {
            // use 1 byte load
            ITRC(IT_SGRP,
                    "SaOnNodeSyncGroup: Wait respons from follower (single_load)\n");
            if (progress_cb) {
                for (unsigned int i = 1; i < member_cnt; i++) {
                    while (sa->Load1(i) != seq)
                        (*progress_cb)(progress_cb_info);
                }
            } else {
                for (unsigned int i = 1; i < member_cnt; i++) {
                    while (sa->Load1(i) != seq);
                }
            }
            ITRC(IT_SGRP,
                    "SaOnNodeSyncGroup: Got respons from follower (single_load)\n");
        }
    } else {
        ITRC(IT_SGRP, "SaOnNodeSyncGroup: Store1(%d, %d) called\n",
                member_id, seq);
        sa->Store1(member_id, seq);
    }
    ITRC(IT_SGRP, "SaOnNodeSyncGroup: Leaving BarrierEnter()\n");
}

void SaOnNodeSyncGroup::BarrierExit()
{
    ITRC(IT_SGRP, "SaOnNodeSyncGroup: Entering BarrierExit()\n");
    ASSERT(this->initialized);
    // if only one member in group
    if (this->member_cnt == 1) {
        ITRC(IT_SGRP, "SaOnNodeSyncGroup: Leaving BarrierExit()\n");
        return;
    }
    if (member_id == 0) {
        // notify others about barrier exit
        sa->Store1(member_id, seq);
        ITRC(IT_SGRP, "SaOnNodeSyncGroup: Store1(%d, %d) called\n",
                member_id, seq);
    } else {
        // wait for barrier exit
        ITRC(IT_SGRP, "SaOnNodeSyncGroup: Wait response from leader\n");
        if (progress_cb) {
            while (sa->Load1(0) != seq)
                (*progress_cb)(progress_cb_info);
        } else {
            while (sa->Load1(0) != seq);
        }
        ITRC(IT_SGRP, "SaOnNodeSyncGroup: Got response from leader\n");
    }
    ITRC(IT_SGRP, "SaOnNodeSyncGroup: Leaving BarrierExit()\n");
}

bool SaOnNodeSyncGroup::IsNbBarrierDone()
{
    // if only one member in group
    if (this->member_cnt == 1) {
        return true;
    }

    if(nb_barrier_stage == 0) { /* entering reduce stage */
        // flip the sequence number
        seq = !seq;
        if (member_id == 0) {
            if (multi_load) {
                unsigned int i=0;
                for (i = 0; i + 8 <= member_cnt; i += 8)
                    if (sa->Load8(i) != *(unsigned long long*)(mask[seq]+i))
                        return false;

                for (; i + 4 <= member_cnt; i+= 4)
                    if (sa->Load4(i) != *(unsigned int*)(mask[seq]+i))
                        return false;

                for (; i + 2 <= member_cnt; i+= 2)
                    if (sa->Load2(i) != *(unsigned short*)(mask[seq]+i))
                        return false;

                // use 1-byte loads to finish the rest
                if (i == 0) i = 1;
                for (; i < member_cnt; i++)
                    if (sa->Load1(i) != seq)
                        return false;
            } else {
                // use 1 byte load
                for (unsigned int i = 1; i < member_cnt; i++) {
                    if (sa->Load1(i) != seq)
                        return false;
                }
            }
        } else {
            sa->Store1(member_id, seq);
        }
        nb_barrier_stage = 1; /* reduce done */
    }

    if (nb_barrier_stage == 1) { /* enter broadcast stage */
        if (member_id == 0) {
            // notify others about barrier exit
            sa->Store1(member_id, seq);
        } else {
            // wait for barrier exit
            if (sa->Load1(0) != seq)
                return false;
        }
        nb_barrier_stage = 2;
    }

    if (nb_barrier_stage == 2)
        return true;
}

void SaOnNodeSyncGroup::_Dump() const {
    // call base class function first
    SyncGroup::_Dump();
    printf("\tthis->group_desc      = \"%s\"\n", this->group_desc.c_str());
    printf("\tthis->seq             = %d\n", this->seq);
    printf("\tthis->sa              = %p\n", this->sa);
    printf("\tthis->multi_load      = %s\n", (this->multi_load)?"true":"false");
    printf("\tthis->mask[0]         = %p\n", this->mask[0]);
    for (unsigned int i = 0; this->mask[0] != NULL && i < this->member_cnt; ++i) {
        if (i == 0) printf("\t");
        printf("%d ", this->mask[0][i]);
        if (i == this->member_cnt - 1) printf("\n");
    }
    printf("\tthis->mask[1]         = %p\n", this->mask[1]);
    for (unsigned int i = 0; this->mask[1] != NULL && i < this->member_cnt; ++i) {
        if (i == 0) printf("\t");
        printf("%d ", this->mask[1][i]);
        if (i == this->member_cnt - 1) printf("\n");
    }
}