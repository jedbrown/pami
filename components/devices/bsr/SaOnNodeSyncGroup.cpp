#include "SaOnNodeSyncGroup.h"
#include <stdlib.h>
#include <new>
#include <string.h>
#include "atomics.h"
#include "ClassDump.h"
#include "lapi_itrace.h"
#include "util/common.h"

#ifdef POWER_ARCH
#define EIEIO __asm__ ("\n\t eieio")
#else
#define EIEIO
#endif

const unsigned long long SaOnNodeSyncGroup::mask[2] = { 0x0000000000000000LL,   // 8 bytes 0x00
                                                        0x0101010101010101LL }; // 8 bytes 0x01
template 
SharedArray::RC SaOnNodeSyncGroup::InitSa<Bsr>(void* ctrl_block, size_t ctrl_block_sz);
template
SharedArray::RC SaOnNodeSyncGroup::InitSa<ShmArray>(void* ctrl_block, size_t ctrl_block_sz);

template <class SA_TYPE>
SharedArray::RC SaOnNodeSyncGroup::InitSa(void* ctrl_block, size_t ctrl_block_sz)
{
    if (NULL == sa) {
        try {
            sa = new SA_TYPE(member_cnt, is_leader, done_mask, ctrl_block, ctrl_block_sz);
        } catch (std::bad_alloc e) {
            fprintf(stderr, "(%d)SaOnNodeSyncGroup(%s): Out of memory.\n", 
                    member_id, sa->name);
            ITRC(IT_BSR, "(%d)SaOnNodeSyncGroup(%s): Out of memory.\n", 
                    member_id, sa->name);
            assert(0);
        } catch (...) {
            fprintf(stderr, "(%d)SaOnNodeSyncGroup(%s): Unexpected exception caught.\n",
                    member_id, sa->name);
            ITRC(IT_BSR, "(%d)SaOnNodeSyncGroup(%s): Unexpected exception caught.\n",
                    member_id, sa->name);
            assert(0);
        }
        ITRC(IT_BSR, "SaOnNodeSyncGroup::InitSa<%s> ctrl_bloc=%p ctrl_bloc_sz=%zu\n",
                sa->name, ctrl_block, ctrl_block_sz);
    }
    return sa->CheckInitDone(job_key, member_id, seq);
}

SyncGroup::RC SaOnNodeSyncGroup::CheckInitDone( )
{
    SharedArray::RC sa_rc;
    switch (s_state) {
        case ORIG_ST:
            /* prepare internal states */
            if (member_cnt == 1) {
                /* if there is only one member in the group, we do nothing. */
                ITRC(IT_BSR, "SaOnNodeSyncGroup: Single task GEO done. member_cnt %d "
                        "ORIG_ST->DONE_ST\n", member_cnt);
                group_desc = "SharedArray:SingleTask";
                s_state = DONE_ST;
                return SUCCESS;
            }
            ITRC(IT_BSR, "SaOnNodeSyncGroup(BSR): ORIG_ST->BSR_ST\n");
            s_state = BSR_ST;
            // fall through
        case BSR_ST:
            ASSERT(s_state == BSR_ST);
            sa_rc = InitSa<Bsr>(bsr_ctrl_block, bsr_ctrl_block_sz);
            if ( SharedArray::SUCCESS == sa_rc ) {
                ITRC(IT_BSR, "(%d)SaOnNodeSyncGroup: Using Bsr BSR_ST->DONE_ST\n", 
                        member_id);
                group_desc = "SharedArray:Bsr";
                s_state = DONE_ST;
                sa_type = SA_TYPE_BSR;
                return SUCCESS;
            } else if (SharedArray::PROCESSING == sa_rc ) {
                /* in progress */
                break;
            } else if (SharedArray::FAILED == sa_rc ) {
                ITRC(IT_BSR, "(%d)SaOnNodeSyncGroup: BSR setup failed BSR_ST->SHM_ST\n",
                        member_id);
                /* try use ShmArray */
                s_state = SHM_ST;
                delete sa;
                sa = NULL;
                // fall through 
            } else { assert(0 && "Should not be here"); }
        case SHM_ST:
            sa_rc = InitSa<ShmArray>(shmarray_ctrl_block, shmarray_ctrl_block_sz);
            if ( SharedArray::SUCCESS == sa_rc ) {
                ITRC(IT_BSR, "(%d)SaOnNodeSyncGroup: Using ShmArray SHM_ST->DONE_ST\n", 
                        member_id);
                group_desc = "SharedArray:ShmArray";
                s_state = DONE_ST;
                sa_type = SA_TYPE_SHMARRAY;
                return SUCCESS;
            } else if (SharedArray::PROCESSING == sa_rc ) {
                /* in progress */
                break;
            } else if (SharedArray::FAILED == sa_rc ) {
                ITRC(IT_BSR, "(%d)SaOnNodeSyncGroup: ShmArray setup failed SHM_ST->FAIL_ST\n",
                        member_id);
                /* try use ShmArray */
                s_state = FAIL_ST;
                // fall through 
            } else { assert(0 && "Should not be here"); }
        case FAIL_ST:
            delete sa;
            sa = NULL;
            // No SharedArray object available for using.
            ITRC(IT_BSR, 
                    "(%d)SaOnNodeSyncGroup: Cannot create SharedArray obj\n",
                    member_id);
            return FAILED;
        case DONE_ST:
            return SUCCESS;
        default:
            assert(0 && "Invalid state for SaOnNodeSyncGroup");
            return SUCCESS;
    }

    return PROCESSING;
}

/*!
 * \brief Default destructor.
 */
SaOnNodeSyncGroup::~SaOnNodeSyncGroup()
{
    ITRC(IT_BSR, "~SaOnNodeSyncGroup() sa_type=%s\n", 
            (sa_type == SA_TYPE_BSR)?"SA_TYPE_BSR":
            (sa_type == SA_TYPE_SHMARRAY)?"SA_TYPE_SHMARRAY":"SA_TYPE_NONE");
    delete sa;

    volatile size_t flag;
    // No sync necessary because other threads/tasks will read the cleared value
    // of 0, or will read the updated control block value
    switch (sa_type) {
        case SA_TYPE_BSR:
            flag = *((volatile size_t*)bsr_ctrl_block);
            break;
        case SA_TYPE_SHMARRAY:
            flag = *((volatile size_t*)shmarray_ctrl_block);
            break;
        default:
          if(is_leader) flag = done_mask;
          else          flag = 0;
    }
    if (done_mask == flag || member_cnt == 1) {
        /* Signal to remove the shared memory. No access is allowed after this point */
        SetDoneFlag();
        ITRC(IT_BSR, "~SaOnNodeSyncGroup() done_flag set to %zu\n", ctrl_block->done_flag);
    }
}

void SaOnNodeSyncGroup::BarrierEnter()
{
    ITRC(IT_BSR, "SaOnNodeSyncGroup: Entering BarrierEnter()\n");
    // if only one member in group
    if (member_cnt == 1) {
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Leaving BarrierEnter()\n");
        return;
    }

    if (member_id == 0) {
        ITRC(IT_BSR,
                "SaOnNodeSyncGroup: Wait respons from follower\n");
        // using 8-byte load as many as possible 
        unsigned int byte_offset = 0;
        for (unsigned int i = 8; i <= member_cnt; i += 8) {
            while (sa->Load8(byte_offset) != mask[seq])
                ;
            byte_offset += 8;
        }   

        // use 1-byte loads to finish the rest
        for (; byte_offset < member_cnt; byte_offset++)
            while (sa->Load1(byte_offset) != seq);
        ITRC(IT_BSR,
                "SaOnNodeSyncGroup: Got respons from follower\n");
    } else {
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Store1(%d, %d) called\n",
                member_id, seq);
        sa->Store1(member_id, !seq);
    }
    ITRC(IT_BSR, "SaOnNodeSyncGroup: Leaving BarrierEnter()\n");
}

void SaOnNodeSyncGroup::BarrierExit()
{
    ITRC(IT_BSR, "SaOnNodeSyncGroup: Entering BarrierExit()\n");
    // if only one member in group                             
    if (member_cnt == 1) {                               
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Leaving BarrierExit()\n");
        return;                                                
    }                                                          
    if (member_id == 0) {                                      
        // need eieio to make sure correctness of BSR operations
        EIEIO;                                                 
        // notify others about barrier exit                    
        sa->Store1(member_id, !seq);                           
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Store1(%d, %d) called\n",
                member_id, seq);                               
    } else {                                                   
        // wait for barrier exit                               
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Wait response from leader\n");
        while (sa->Load1(0) != seq);                           
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Got response from leader\n");
    }                                                          

    // flip seq after barrier is done                          
    seq = !seq;                                                
    ITRC(IT_BSR, "SaOnNodeSyncGroup: Leaving BarrierExit()\n");
}

bool SaOnNodeSyncGroup::IsNbBarrierDone()
{
    // if only one member in group
    if (member_cnt == 1) {
        ITRC(IT_BSR, "SaOnNodeSyncGroup::IsNbBarrierDone() returns with member id: %d of %d, seq: %d\n", 
                member_id, member_cnt, seq);
        return true;
    }

    if(nb_barrier_stage == 0) { /* entering reduce stage */
        //        show_bsr("stage 0");
        if (member_id == 0) {
            // using 8-byte load as many as possible
            unsigned int byte_offset = 0;
            for (unsigned int i = 8; i <= member_cnt; i += 8) {
                if (sa->Load8(byte_offset) != mask[seq])
                    return false;
                byte_offset += 8;
            }
            // using 1-byte load for the remained
            for (; byte_offset < member_cnt; byte_offset ++) {
                if (sa->Load1(byte_offset) != seq)
                    return false;
            }
        } else {
            sa->Store1(member_id, !seq);
        }
        nb_barrier_stage = 1; /* reduce done */
    }

    if (nb_barrier_stage == 1) { /* enter broadcast stage */
        if (member_id == 0) {
            // need eieio to make sure correctness of BSR operations
            EIEIO;
            // notify others about barrier exit
            sa->Store1(member_id, !seq);
        } else {
            // wait for barrier exit
            if (sa->Load1(0) != seq)
                return false;
        }
        nb_barrier_stage = 2;
        seq = !seq;
    }

    return true;
}

ClassDump & operator<< (ClassDump &dump, const SaOnNodeSyncGroup &s) {
#define _F(NAME) _f(#NAME, s.NAME)
    return dump << ":SaOnNodeSyncGroup"
        << _f("group_desc", s.group_desc.c_str())
        << _F(member_cnt)
        << _F(member_id)
        << _F(is_leader)
        << _F(job_key)
        << _F(seq)
        << _F(sa)
        << _F(nb_barrier_stage)
        << _F(ctrl_block)
        << _F(bsr_ctrl_block)
        << _F(bsr_ctrl_block_sz)
        << _F(shmarray_ctrl_block)
        << _F(shmarray_ctrl_block_sz)
        << _F(done_mask);
#undef _F
}
