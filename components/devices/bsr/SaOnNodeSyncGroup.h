/**
 * \file components/devices/bsr/SaOnNodeSyncGroup.h
 * \brief Class declaration for SaOnNodeSyncGroup class.
 */
#ifndef __components_devices_bsr_SaOnNodeSyncGroup_h__
#define __components_devices_bsr_SaOnNodeSyncGroup_h__

#include "SyncGroup.h"
#include "SharedArray.h"
#include "lapi_assert.h"

/**
 * \brief SaOnNodeSyncGroup class
 *
 * An object that represents an on-node synchronizable group using shared
 * array.
 *
 */
class SaOnNodeSyncGroup : public SyncGroup {
    public:
        /**
         * \brief Data structure to hold extra infomation (hints) to initialize
         *        the object.
         */
        typedef struct {
            bool multi_load;///< Flag to indicate if multi-byte load should be used.
            bool use_shm_only;/**< Flag to indicate we want to use ShmArray only.
                               * If it is set, no other method, e.g.
                               * BSR, will be tried. */
        } Param_t;

        SaOnNodeSyncGroup();
        ~SaOnNodeSyncGroup();
        /**
         * \brief Check Initialization Status function.
         * Initalize internal variables and allocate mempry spaces. Every member
         * prcess that want to be in the group has to initialize the SaOnSyncGroup
         * with the same \em member_cnt and \em group_id values.
         *
         * \param member_cnt Number of members will be in the group.
         * \param job_key    Job key is used to communicate with PNSD and also
         *                   used to generate an unique SHM key 
         * \param unique_key A unique number generated at the time the geometry
         *                   is created to generate an unique SHM key
         * \param member_id  An unique id to identify the member who creates the
         *                   object. The value should be in the range of
         *                   [0..member_cnt)
         * \param param      A pointer to SaOnNodeSyncGroup::Param structure.
         *                   Required for SaOnNodeSyncGroup.
         *
         * \return SyncGroup::SUCCESS SyncGroup::FAILED
         */
        RC   CheckInitDone(const unsigned int   member_cnt,
                           const unsigned int   job_key, 
                           const uint64_t       unique_key,
                           const unsigned int   member_id, 
                           void*                param);
        void BarrierEnter();
        void BarrierExit();
        bool IsNbBarrierDone();/* to check if the non-blocking Barrier finishes */
        //bool IsNbBarrierDone();/* to check if the non-blocking Barrier finishes */
        void NbBarrier();     /* issue non-blocking barrier */
        void _Dump() const;
        void show_bsr(char*); /* for debugging purpose */

    private:
        enum SetupState {
            ORIG_ST = 0,
            BSR_ST,
            BSR_DONE_ST,
            BSR_FAIL_ST,
            SHM_ST,
            SHM_DONE_ST,
            FAIL_ST,
            DONE_ST
        } s_state;
        SaOnNodeSyncGroup::SetupState Init_bsr_step(const unsigned int  mem_cnt, 
                                                    const unsigned int  job_key, 
                                                    const uint64_t      unique_key,
                                                    const unsigned int  mem_id, 
                                                    void*               param);
        SaOnNodeSyncGroup::SetupState Init_shm_step();
        SaOnNodeSyncGroup::SetupState Init_shm();
        SaOnNodeSyncGroup::SetupState Init_bsr();
        unsigned int  is_leader;
        unsigned int  job_key;
        uint64_t      unique_key;
        unsigned int  seq;
        SharedArray   *sa;      // could be BSRs, ShmArray
        unsigned char mask[2][8];
        bool          multi_load;
        int           nb_barrier_stage; /* stages for non-blocking barrier.
                                         * (0) init stage
                                         * (1) for reduce done;
                                         * (2) for broadcast done */
        unsigned int  multi_byte_load_num;
        unsigned int  single_byte_load_num;

};

inline
void SaOnNodeSyncGroup::NbBarrier()
{
    assert(nb_barrier_stage == 2);
    nb_barrier_stage = 0;
}

inline
SaOnNodeSyncGroup::SaOnNodeSyncGroup():
    is_leader(0),
    job_key(0),
    unique_key(0),
    seq(0),
    sa(NULL),
    multi_load(false),
    nb_barrier_stage(2),
    s_state(ORIG_ST),
    multi_byte_load_num(0),
    single_byte_load_num(0)
{
};
#endif /* _SAONNODESYNCGROUP_H_ */
