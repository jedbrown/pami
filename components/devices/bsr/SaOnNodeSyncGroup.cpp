#include "SaOnNodeSyncGroup.h"
#include <stdlib.h>
#include <unistd.h>
#include <new>
#include <string.h>
#include "ClassDump.h"
#include "util/common.h"

#ifdef POWER_ARCH
#define EIEIO __asm__ ("\n\t eieio")
#else
#define EIEIO
#endif

static volatile bool bsr_failover_informed = false;

const unsigned long long SaOnNodeSyncGroup::mask[2] = { 0x0000000000000000LL,   // 8 bytes 0x00
                                                        0x0101010101010101LL }; // 8 bytes 0x01

SyncGroup::RC SaOnNodeSyncGroup::CheckInitDone(SaType *dev_type)
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
            ITRC(IT_BSR, "SaOnNodeSyncGroup: ORIG_ST->SHM_ST is_leader=%d "
                    "job_key=%u mem_id=%d seq=%u\n",
                    is_leader, job_key, member_id, seq);
            s_state = SHM_ST;
            // fall through
        case SHM_ST:
            /*
             * Initialize ShmArray first. It can be used for failover or checkpoint
             * support. This should not fail. If fails, there is nothing we can do
             * but return FAIL.
             */
            sa_rc = shm_sa->CheckInitDone(job_key, member_id, seq);
            if ( SharedArray::SUCCESS == sa_rc ) {
                ITRC(IT_BSR,
                        "SaOnNodeSyncGroup: shm_sa init done; try Bsr init SHM_ST->BSR_ST\n");
                /* try initialize Bsr next */
                s_state = BSR_ST;
                // fall through
            } else if (SharedArray::PROCESSING == sa_rc ) {
                /* in progress */
                break;
            } else if (SharedArray::FAILED == sa_rc ) {
                ITRC(IT_BSR, "SaOnNodeSyncGroup: ShmArray init failed SHM_ST->FAIL_ST\n");
                s_state = FAIL_ST;
                return FAILED;
            } else { assert(0 && "Should not be here"); }
        case BSR_ST:
            sa_rc = bsr_sa->CheckInitDone(job_key, member_id, seq);
            if ( SharedArray::SUCCESS == sa_rc ) {
                ITRC(IT_BSR, "SaOnNodeSyncGroup(%s): bsr_sa init done BSR_ST->DONE_ST\n",
                        (is_leader)?"LEADER":"FOLLOWER");
                group_desc = "SharedArray:Bsr";
                sa      = bsr_sa;
                *dev_type = sa_type = SA_TYPE_BSR;
                s_state = DONE_ST;
                ITRC(IT_BSR, "SaOnNodeSyncGroup: using BSR\n");
                return SUCCESS;
            } else if (SharedArray::PROCESSING == sa_rc ) {
                /* in progress */
                break;
            } else if (SharedArray::FAILED == sa_rc ) {
                ITRC(IT_BSR, "SaOnNodeSyncGroup(%s): bsr_sa init failed BSR_ST->DONE_ST\n",
                        (is_leader)?"LEADER":"FOLLOWER");
                /* clean up bsr_sa */
                delete bsr_sa;
                bsr_sa = NULL;
                if (!bsr_failover_informed && is_leader)
                {
                    char host[256] = "";
                    gethostname(host, sizeof(host));
                    fprintf(stderr, "ATTENTION: BSR resource is NOT ready on %s.\n", host);
                    bsr_failover_informed = true;
                }
                group_desc = "SharedArray:ShmArray";
                sa      = shm_sa;
                *dev_type = sa_type = SA_TYPE_SHMARRAY;
                s_state = DONE_ST;
                ITRC(IT_BSR, "SaOnNodeSyncGroup: using ShmArray\n");
                return SUCCESS;
            } else { assert(0 && "Should not be here"); }
        case FAIL_ST:
            assert(0 && "should not be in FAIL_ST");
        case DONE_ST:
            assert(0 && "should not be in DONE_ST");
        default:
            assert(0 && "Invalid state for SaOnNodeSyncGroup");
    }

    return PROCESSING;
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

bool SaOnNodeSyncGroup::Checkpoint() {
    assert (false == ckpt_info.in_checkpoint);
    ckpt_info.in_checkpoint = true;
    int last_cnt = fetch_and_add((atomic_p)&(ctrl_block->ckpt_ref_cnt), 1);(void)last_cnt;
    ITRC(IT_INITTERM,
            "SaOnNodeSyncGroup::Checkpoint() cnt=%d state=%d in_term=%d "
            "sa=%p bsr_sa=%p shm_sa=%p mem_id=%d\n",
            last_cnt+1, s_state, ctrl_block->in_term, sa, bsr_sa, shm_sa, member_id);

    bool rc = true;
    /* save the s_state */
    ckpt_info.ckpt_state = s_state;

    if (bsr_sa) {
        /* When sa_type is set to SA_TYPE_BSR, it means bsr_sa is ready to use */
        if (s_state == DONE_ST) {
            ASSERT(SA_TYPE_BSR == sa_type);
            /* Save bsr_sa byte to shm_sa for failover */
            unsigned char byte = bsr_sa->Load1(member_id);
            shm_sa->Store1(member_id, byte);
        }
        rc = bsr_sa->Checkpoint(member_id);
        PAMI::Memory::sync();
    }

    /* Don't need worry about shm_sa */

    /* sa should be set at resume/restart */
    sa = NULL;
    return rc;
}

template <bool FOR_RESUME>
void SaOnNodeSyncGroup::ReInitSa() {
  const char *action = (FOR_RESUME)?"Resume":"Restart";(void)action;
    if (bsr_sa) {
        if (ctrl_block->in_term) {
            /*
             * If some member has called destructor before checkpoint, it is not able to
             * re-initialze Bsr due to missing members. We would use shm_sa only.
             */
            ITRC(IT_INITTERM|IT_BSR,
                    "SaOnNodeSyncGroup::%s(), in termination using shm_sa\n",
                    action);
            sa_type = SA_TYPE_SHMARRAY;
            sa      = shm_sa;
        } else {
            /* Re-initialize Bsr */
            bool rc;
            if (FOR_RESUME)
                rc = bsr_sa->Resume(member_id);
            else
                rc = bsr_sa->Restart(member_id);

            if (true == rc) {
                sa      = bsr_sa;
            } else {
                delete bsr_sa;
                bsr_sa = NULL;
                /* Bsr_sa failed to reinitialize, use shm_sa */
                ITRC(IT_INITTERM|IT_BSR, "SaOnNodeSyncGroup::%s() bsr_sa->%s() "
                        "failed using shm_sa instead\n", action, action);
                sa_type = SA_TYPE_SHMARRAY;
                sa      = shm_sa;
            }
        }
    } else {
        ASSERT(s_state == DONE_ST);
        ASSERT(sa_type == SA_TYPE_SHMARRAY);
        sa = shm_sa;
    }
}

bool SaOnNodeSyncGroup::Resume() {
    ITRC(IT_INITTERM|IT_BSR,
            "SaOnNodeSyncGroup::Resume() Enters state=%d in_term=%d "
            "sa=%p bsr_sa=%p shm_sa=%p mem_id=%d\n",
             s_state, ctrl_block->in_term, sa, bsr_sa, shm_sa, member_id);
    assert (ckpt_info.in_checkpoint);
    assert (s_state == ckpt_info.ckpt_state);

    ReInitSa<true>();

    ckpt_info.in_checkpoint = false;
    int last_cnt = fetch_and_add((atomic_p)&(ctrl_block->ckpt_ref_cnt), -1);
    assert (last_cnt > 0);

    ITRC(IT_INITTERM|IT_BSR,
            "SaOnNodeSyncGroup::Resume() remaining cnt=%d\n", last_cnt - 1);

    PAMI::Memory::sync();
    /* Wait everybody finishes the resume */
    while (ctrl_block->ckpt_ref_cnt);
    ITRC(IT_INITTERM|IT_BSR, "SaOnNodeSyncGroup::Resume() Exists\n");

    return true;
}

bool SaOnNodeSyncGroup::Restart() {
    ITRC(IT_INITTERM|IT_BSR,
            "SaOnNodeSyncGroup::Restart() Enters state=%d in_term=%d "
            "sa=%p bsr_sa=%p shm_sa=%p mem_id=%d\n",
             s_state, ctrl_block->in_term, sa, bsr_sa, shm_sa, member_id);
    assert (ckpt_info.in_checkpoint);
    assert (s_state == ckpt_info.ckpt_state);

    ReInitSa<false>();

    ckpt_info.in_checkpoint = false;
    int last_cnt = fetch_and_add((atomic_p)&(ctrl_block->ckpt_ref_cnt), -1);
    assert (last_cnt > 0);

    ITRC(IT_INITTERM|IT_BSR,
            "SaOnNodeSyncGroup::Restart() cnt=%d\n", last_cnt - 1);

    PAMI::Memory::sync();
    /* Wait everybody finishes the restart */
    while (ctrl_block->ckpt_ref_cnt);
    ITRC(IT_INITTERM|IT_BSR, "SaOnNodeSyncGroup::Restart() Exists\n");

    return true;
}

