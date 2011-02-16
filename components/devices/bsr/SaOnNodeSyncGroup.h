/*!
 * \file SaOnNodeSyncGroup.h
 * \brief Class declaration for SaOnNodeSyncGroup class.
 */
#ifndef _SAONNODESYNCGROUP_H_
#define _SAONNODESYNCGROUP_H_

#include "SyncGroup.h"
#include "SharedArray.h"
#include "lapi_assert.h"

/*!
 * \brief SaOnNodeSyncGroup class
 *
 * An object that represents an on-node synchronizable group using shared
 * array.
 *
 */
class SaOnNodeSyncGroup : public SyncGroup {
    public:
        /*!
         * \brief Data structure to hold extra infomation (hints) to initialize
         *        the object.
         */
        typedef struct {
            bool multi_load;///< Flag to indicate if multi-byte load should be used.
            bool use_shm_only;/*!< Flag to indicate we want to use ShmArray only.
                               * If it is set, no other method, e.g.
                               * BSR, will be tried. */
        } Param_t;

        SaOnNodeSyncGroup();
        ~SaOnNodeSyncGroup();
        /*!
         * \brief Initialize function.
         * Initalize internal variables and allocate mempry spaces. Every member
         * prcess that want to be in the group has to initialize the SaOnSyncGroup
         * with the same \em member_cnt and \em group_id values.
         *
         * \param member_cnt Number of members will be in the group.
         * \param group_id   An UNIQUE id that identifies the group and will be used
         *                   as a key to create internal shared memory segment.
         * \param member_id  An unique id to identify the member who creates the
         *                   object. The value should be in the range of
         *                   [0..member_cnt)
         * \param param      A pointer to SaOnNodeSyncGroup::Param structure.
         *                   Required for SaOnNodeSyncGroup.
         *
         * \return SyncGroup::SUCCESS SyncGroup::FAILED
         */
        RC   Init(const unsigned int member_cnt, const unsigned int group_id,
                const unsigned int member_id, void* param);
        void BarrierEnter();
        void BarrierExit();
        bool IsNbBarrierDone();/* to check if the non-blocking Barrier finishes */
        void NbBarrier();     /* issue non-blocking barrier */
        void _Dump() const;

    private:
        int           seq;
        SharedArray   *sa;      // could be BSRs, ShmArray
        unsigned char *mask[2];
        bool          multi_load;
        int           nb_barrier_stage; /* stages for non-blocking barrier.
                                         * (0) init stage
                                         * (1) for reduce done;
                                         * (2) for broadcast done */

};

inline
void SaOnNodeSyncGroup::NbBarrier()
{
    nb_barrier_stage = 0;
}

inline
SaOnNodeSyncGroup::SaOnNodeSyncGroup():
    seq(0),
    sa(NULL),
    multi_load(false),
    nb_barrier_stage(0)
{
    mask[0] = NULL;
    mask[1] = NULL;
};
#endif /* _SAONNODESYNCGROUP_H_ */
