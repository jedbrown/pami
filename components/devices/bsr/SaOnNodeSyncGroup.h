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
#include "lapi_itrace.h"
#include "atomics.h"

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

        typedef enum {
          SA_TYPE_NONE = 0,   ///< NO device is ready
          SA_TYPE_BSR,        ///< BSR device is ready
          SA_TYPE_SHMARRAY    ///< SHM (BSR failover) device is ready
        } SaType;

        /**
         * \brief Check Initialization Status function.
         * \return SyncGroup::SUCCESS, SyncGroup::PROCESSING, SyncGroup::FAILED
         */
        RC   CheckInitDone(SaType *dev_type);
        void BarrierEnter();
        void BarrierExit();
        bool IsNbBarrierDone();/* to check if the non-blocking Barrier finishes */
        void NbBarrier();      /* issue non-blocking barrier */
        bool IsInitialized();  /* if s_state == DONE_ST */

        /* for Checkpoint support */
        bool Checkpoint();
        bool Restart();
        bool Resume();

    private:
        // seq = 0, use mask[0] 
        // seq = 1, use mask[1]
        static const unsigned long long mask[2];
        enum SetupState {
            ORIG_ST = 0,
            SHM_ST,     // try to initialize ShmArray
            BSR_ST,     // try to initialize BSR
            FAIL_ST,
            DONE_ST
        } s_state;
        unsigned int  is_leader;
        unsigned int  job_key;
        unsigned int  seq;
        Bsr          *bsr_sa;
        ShmArray     *shm_sa;  // for failover use
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

        SaType        sa_type;

        void AllocSa();
        void SetDoneFlag();
        template <bool> void ReInitSa();

        /*
         * All data in this structure would be zero initially.
         * The structure is on shared memory
         */
        struct CtrlBlock {
            volatile size_t       done_flag;   // Flag to signal upper layer to clean up the shm
                                               // done_flag has to be the 1st word
            volatile int          ref_cnt;
            volatile int          ckpt_ref_cnt;// num. of member in checkpoint (on shm)
            volatile bool         in_term;     // some member called ~SaOnNodeSyncGroup()
                                               // used for checkpoint/restart
        }                        *ctrl_block;
        class CheckpointInfo {
            public:
                CheckpointInfo():
                    in_checkpoint(false),
                    ckpt_state(ORIG_ST){};

                bool          in_checkpoint;
                SetupState    ckpt_state; // s_state before checkpoint
        };
        CheckpointInfo        ckpt_info;
};

inline
bool SaOnNodeSyncGroup::IsInitialized()
{
    return (s_state == DONE_ST);
}

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
    s_state(ORIG_ST),
    is_leader(mem_id == 0),
    job_key(job_key),
    seq(0),
    bsr_sa(NULL),
    shm_sa(NULL),
    sa(NULL),
    nb_barrier_stage(2),
    bsr_ctrl_block(NULL),
    bsr_ctrl_block_sz(0),
    shmarray_ctrl_block(NULL),
    shmarray_ctrl_block_sz(0),
    done_mask(mem_id+1),
    sa_type(SA_TYPE_NONE)
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

    /* allocate bsr_sa and shm_sa */
    AllocSa();

    /* increment reference count */
    int ref = fetch_and_add((atomic_p)&ctrl_block->ref_cnt, 1);
    (void)ref;
    ASSERT(ref <= (int)member_cnt);
};

/*!
 * \brief Default destructor.
 */
inline
SaOnNodeSyncGroup::~SaOnNodeSyncGroup()
{
    ITRC(IT_BSR, "~SaOnNodeSyncGroup() sa_type=%s bsr_sa=0x%p shm_sa=0x%p\n", 
            (sa_type == SA_TYPE_BSR)?"SA_TYPE_BSR":
            (sa_type == SA_TYPE_SHMARRAY)?"SA_TYPE_SHMARRAY":"SA_TYPE_NONE",
            bsr_sa, shm_sa);
    delete bsr_sa; bsr_sa = NULL;
    delete shm_sa; shm_sa = NULL;
    sa = NULL;

    int ref = fetch_and_add ((atomic_p)&ctrl_block->ref_cnt, -1);
    if (ref == 1) {
        /* The last guy to leave has to set done flag */
        /* Signal to remove the shared memory. No access is allowed after this point */
        SetDoneFlag();
        ITRC(IT_BSR, "~SaOnNodeSyncGroup() done_flag set to %zu\n", ctrl_block->done_flag);
    }
    ctrl_block->in_term = true;
    ASSERT(ref > 0);
}

inline
void SaOnNodeSyncGroup::AllocSa()
{
    try {
        bsr_sa = new Bsr(member_cnt, is_leader, bsr_ctrl_block, bsr_ctrl_block_sz);
        shm_sa = new ShmArray(member_cnt, is_leader, shmarray_ctrl_block, shmarray_ctrl_block_sz);
    } catch (std::bad_alloc e) {
        fprintf(stderr, "SaOnNodeSyncGroup: Out of memory.\n");
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Out of memory.\n");
        assert(0);
    } catch (...) {
        fprintf(stderr, "SaOnNodeSyncGroup: Unexpected exception caught.\n");
        ITRC(IT_BSR, "SaOnNodeSyncGroup: Unexpected exception caught.\n");
        assert(0);
    }
    ITRC(IT_BSR, "SaOnNodeSyncGroup::InitSa() bsr_sa=0x%p shm_sa=0x%p\n",
            bsr_sa, shm_sa);
}

inline
void SaOnNodeSyncGroup::SetDoneFlag() {
    /* member_id + 1 is the contracted mask with CAURegistration */
    ctrl_block->done_flag = done_mask;
}
#endif /* _SAONNODESYNCGROUP_H_ */

