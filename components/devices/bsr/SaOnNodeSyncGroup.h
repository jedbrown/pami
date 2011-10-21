/**
 * \file components/devices/bsr/SaOnNodeSyncGroup.h
 * \brief Class declaration for SaOnNodeSyncGroup class.
 */
#ifndef __components_devices_bsr_SaOnNodeSyncGroup_h__
#define __components_devices_bsr_SaOnNodeSyncGroup_h__

#include "SyncGroup.h"
#include "Bsr.h"
#include "ShmArray.h"
#include "lapi_assert.h"

class ClassDump;
/**
 * \brief SaOnNodeSyncGroup class
 *
 * An object that represents an on-node synchronizable group using shared
 * array.
 *
 */
class SaOnNodeSyncGroup : public SyncGroup {
    friend ClassDump & operator<< (ClassDump &dump, const SaOnNodeSyncGroup &s);
    public:
        SaOnNodeSyncGroup(unsigned int member_id, unsigned int mem_cnt,
                unsigned int job_key, void* shm_block, size_t shm_block_sz);
        ~SaOnNodeSyncGroup();
        /**
         * \brief Check Initialization Status function.
         * \return SyncGroup::SUCCESS, SyncGroup::PROCESSING, SyncGroup::FAILED
         */
        RC   CheckInitDone();
        void BarrierEnter();
        void BarrierExit();
        bool IsNbBarrierDone();/* to check if the non-blocking Barrier finishes */
        void NbBarrier();      /* issue non-blocking barrier */

    private:
        // seq = 0, use mask[0] 
        // seq = 1, use mask[1]
        static const unsigned long long mask[2];
        enum SetupState {
            ORIG_ST = 0,
            BSR_ST,     // try to initialize BSR
            SHM_ST,     // try to initialize ShmArray
            FAIL_ST,
            DONE_ST
        } s_state;
        template <class SA_TYPE>
            SharedArray::RC InitSa(void* ctrl_block, size_t ctrl_block_sz);
        unsigned int  is_leader;
        unsigned int  job_key;
        unsigned int  seq;
        SharedArray  *sa;      // could be BSRs, ShmArray
        int           nb_barrier_stage; /* stages for non-blocking barrier.
                                         * (0) init stage
                                         * (1) for reduce done;
                                         * (2) for broadcast done */
        void         *bsr_ctrl_block;
        size_t        bsr_ctrl_block_sz;
        void         *shmarray_ctrl_block;
        size_t        shmarray_ctrl_block_sz;
        size_t        done_mask;

        enum SaType {
            SA_TYPE_NONE = 0,
            SA_TYPE_BSR,
            SA_TYPE_SHMARRAY
        }             sa_type;
        /* for Checkpoint support */
        struct CtrlBlock {
            volatile size_t       done_flag;   // Flag to signal upper layer to clean up the shm
        }                        *ctrl_block;
        void SetDoneFlag() {
            /* member_id + 1 is the contracted mask with CAURegistration */
            ctrl_block->done_flag = done_mask;
        }
};

inline
void SaOnNodeSyncGroup::NbBarrier()
{
    ASSERT(nb_barrier_stage == 2);
    nb_barrier_stage = 0;
}

inline
SaOnNodeSyncGroup::SaOnNodeSyncGroup(unsigned int mem_id, unsigned int mem_cnt,
        unsigned int job_key, void* shm_block, size_t shm_block_sz):
    SyncGroup(mem_id, mem_cnt),
    is_leader(mem_id == 0),
    job_key(job_key),
    seq(0),
    sa(NULL),
    nb_barrier_stage(2),
    s_state(ORIG_ST),
    bsr_ctrl_block(NULL),
    bsr_ctrl_block_sz(0),
    shmarray_ctrl_block(NULL),
    shmarray_ctrl_block_sz(0),
    sa_type(SA_TYPE_NONE),
    done_mask(mem_id+1)
{
    assert(member_cnt > 0);
    // modify seq, the init value is 0 for all tasks
    // seq of leader remains 0 with seq of the rest of tasks turned to 1
    seq = (is_leader)? seq : (!seq);

    size_t ctrl_block_sz   =
        (sizeof(CtrlBlock) + sizeof(size_t)-1) & ~(sizeof(size_t)-1); // padding to word size
    bsr_ctrl_block_sz      = Bsr::GetCtrlBlockSz(member_cnt);
    shmarray_ctrl_block_sz = ShmArray::GetCtrlBlockSz(member_cnt);

    assert (shm_block_sz >= (ctrl_block_sz + bsr_ctrl_block_sz + shmarray_ctrl_block_sz));

    /*
     * we need to separate these control blocks, since it is hard to 
     * reinitialize to zero if used.
     * -----------------------------------------------------
     * [ ctrl_block | bsr_ctrl_block | shmarray_ctrl_block ]
     * -----------------------------------------------------
     */
    ctrl_block             = (CtrlBlock*)shm_block;
    bsr_ctrl_block         = (void*)((char*)ctrl_block + ctrl_block_sz);
    shmarray_ctrl_block    = (void*)((char*)bsr_ctrl_block + bsr_ctrl_block_sz);
};

#endif /* _SAONNODESYNCGROUP_H_ */

