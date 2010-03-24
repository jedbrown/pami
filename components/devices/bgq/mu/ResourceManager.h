/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
////////////////////////////////////////////////////////////////////////////////
///
/// \file components/devices/bgq/mu/ResourceManager.h
///
/// \brief Resource Manager definitions.
///
////////////////////////////////////////////////////////////////////////////////
#ifndef __components_devices_bgq_mu_ResourceManager_h__
#define __components_devices_bgq_mu_ResourceManager_h__

#include <hwi/include/bqc/MU_Descriptor.h>

#include "components/devices/bgq/mu/InjFifoSubGroup.h"
#include "components/devices/bgq/mu/RecFifoSubGroup.h"
#include "components/devices/bgq/mu/BaseAddressTableSubGroup.h"
#include "components/devices/bgq/mu/Dispatch.h"

#include "SysDep.h"

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      ////////////////////////////////////////////////////////////////////////////
      ///
      /// \brief Resource SubGroup Info Structure
      ///
      /// Defines a resource subgroup:
      /// - subgroupid
      /// - number of elements (fifos, bat entries).
      ///
      ////////////////////////////////////////////////////////////////////////////

      typedef struct Resource_SubGroup
      {
        uint32_t subGroupId;
        uint32_t numElements;

      } Resource_SubGroup_t;


      ////////////////////////////////////////////////////////////////////////////
      ///
      /// \brief Resource Structure
      ///
      /// Defines a resource (injection fifo, reception fifo, base address
      /// table).
      ////////////////////////////////////////////////////////////////////////////

      typedef struct Resource
      {
        uint32_t numSubGroups;
        Resource_SubGroup_t subGroupInfo[1]; // There are numSubGroups of these.

      } Resource_t;



      class ResourceManager
      {
        public:

          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Base Address Table Default Entry Number
          ///
          /// The entry number in the first base address table subgroup that
          /// contains a zero base physical address.
          ///
          //////////////////////////////////////////////////////////////////////////

          const static uint32_t BAT_DEFAULT_ENTRY_NUMBER = 0;


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Base Address Table Shared Counter Entry Number
          ///
          /// The entry number in the first base address table subgroup that
          /// contains the physical address of the node shared counter.
          ///
          //////////////////////////////////////////////////////////////////////////

          const static uint32_t BAT_SHAREDCOUNTER_ENTRY_NUMBER = BAT_DEFAULT_ENTRY_NUMBER + 1;


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Base Address Table Default Entry Value
          ///
          /// The value (zero) in the BAT_DEFAULT_ENTRY_NUMBER of the first base
          /// address table subgroup.
          ///
          //////////////////////////////////////////////////////////////////////////

          const static uint64_t BAT_DEFAULT_ENTRY_VALUE = 0x0000000000000000ULL;


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief MINIMAL Resource Configuration
          ///
          //////////////////////////////////////////////////////////////////////////

          const static uint32_t MINIMAL_INJ_NUM_SUBGROUPS = 1;
          const static uint32_t MINIMAL_REC_NUM_SUBGROUPS = 1;
          const static uint32_t MINIMAL_BAT_NUM_SUBGROUPS = 1;

          const static uint32_t MINIMAL_INJ_SUBGROUP_NUM_FIFOS   = 1;
          const static uint32_t MINIMAL_REC_SUBGROUP_NUM_FIFOS   = 1;
          const static uint32_t MINIMAL_BAT_SUBGROUP_NUM_ENTRIES = 2;

          //const static uint32_t MINIMAL_NUM_EAGER_FIFOS         = 1;
          //const static uint32_t MINIMAL_FIRST_EAGER_FIFO_NUMBER = 0;
          //const static uint32_t MINIMAL_COLLECTIVE_FIFO_NUMBER  = 0;


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Default fifo sizes
          ///
          //////////////////////////////////////////////////////////////////////////

          const static uint32_t TORUS_PACKET_PAYLOAD_MAX_BYTES = 512;
          const static uint32_t DEFAULT_INJ_FIFO_DESC_COUNT = 1024;
//	1024 * sizeof ( MUHWI_Descriptor_t);

          const static uint32_t DEFAULT_REC_FIFO_SIZE = 64 * 1024; // \todo Enlarge?


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Fifo Alignment Requirements
          ///
          //////////////////////////////////////////////////////////////////////////

          const static uint32_t INJ_FIFO_ALIGNMENT = 64;
          const static uint32_t REC_FIFO_ALIGNMENT = 32;


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Resource Mode
          ///
          /// Defines the different modes that resources may have.
          ///
          //////////////////////////////////////////////////////////////////////////

          typedef enum ResourceMode
          {
            MINIMAL,
            DEFAULT
          } ResourceMode_t;


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Resource Type
          ///
          /// Defines the different resource types
          ///
          //////////////////////////////////////////////////////////////////////////

          typedef enum ResourceType
          {
            P2P_TYPE,
            COLL_TYPE
          } ResourceType_t;



          ResourceManager () {TRACE((stderr, "ResourceManager::ctor\n")); }

          int init ( ResourceType_t ,
                     PAMI::Memory::MemoryManager *mm,
                     dispatch_t      *dispatch
                   );

          int advanceInjectionFifos()
          {
            uint32_t numSubGroups = _injFifoInfo->numSubGroups;
            int events = 0;

            for ( uint32_t i = 0; i < numSubGroups; i++ )
              {
                events += _injFifoSubGroups[i]->advance();
              }

            return events;
          }


          int advanceReceptionFifos()
          {
            uint32_t numSubGroups = _recFifoInfo->numSubGroups;
            //TRACE((stderr, ">> advanceReceptionFifos() .. numSubGroups = %d\n", numSubGroups));
            int events = 0;

            for ( uint32_t i = 0; i < numSubGroups; i++ )
              {
                events += _recFifoSubGroups[i]->advance();
              }

            //TRACE((stderr, "<< advanceReceptionFifos() .. events = %d\n", events));
            return events;
          }


          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief FIFO pinning: assign a unique send FIFO based on
          /// destination coordinates for a P2P message.
          ///
          /// If there are multiple injection fifo subgroups, the returned
          /// fifo number is spans the subgroups.  For example, if there
          /// are 2 subgroups, one subgroup with 2 fifos and the second
          /// subgroup with 3 fifos, then the fifo numbers range from
          /// 0 to 4, with 0 and 1 in the first subgroup, and 2, 3, and 4
          /// in the second.
          ///
          /// \todo For now we will only use one fifo
          ///
          //////////////////////////////////////////////////////////////////////////

          uint32_t pinFifo ( int destrank )
          {
            //return _firstEagerFifoNumber;
            return 0; // everything goes into a single inj fifo
          }

          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief FIFO pinning: assign a unique send FIFO for an omni-directional
          /// (collective) message
          ///
          /// If there are multiple injection fifo subgroups, the returned
          /// fifo number is spans the subgroups.  For example, if there
          /// are 2 subgroups, one subgroup with 2 fifos and the second
          /// subgroup with 3 fifos, then the fifo numbers range from
          /// 0 to 4, with 0 and 1 in the first subgroup, and 2, 3, and 4
          /// in the second.
          ///
          /// \todo For now we will only use one fifo
          ///
          //////////////////////////////////////////////////////////////////////////

          uint32_t pinFifo ()
          {
            return 0; // everything goes into a single inj fifo
          }



          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Map a Fifo Number to an InjFifoSubGroup and a Relative Fifo
          ///        Number
          ///
          /// \param[in]  fifoNum  The input fifo number.  This spans the subgroups.
          /// \param[in,out] injFifoSubGroup  Input is a pointer to the output
          ///                                 injection fifo subgroup pointer.
          ///                                 The specified fifo belongs to this
          ///                                 injection fifo subgroup.
          /// \param[in,out] relativeFifoNum  Input is a pointer to the output
          ///                                 fifo number that is relative to the
          ///                                 injection fifo subgroup to which the
          ///                                 specified fifo belongs
          ///
          //////////////////////////////////////////////////////////////////////////

          void getSubGroupAndRelativeFifoNum ( uint32_t          fifoNum,
                                               InjFifoSubGroup **injFifoSubGroup,
                                               uint32_t         *relativeFifoNum )
          {
            uint32_t numSubGroups = _injFifoInfo->numSubGroups;
            Resource_SubGroup_t *subGroupInfo = &(_injFifoInfo->subGroupInfo[0]);
            uint32_t numElements;

            for ( uint32_t i = 0; i < numSubGroups; i++ )
              {
                numElements = subGroupInfo[i].numElements;

                if ( fifoNum < numElements )
                  {
                    *relativeFifoNum = fifoNum;
                    *injFifoSubGroup = _injFifoSubGroups[i];
                    break;
                  }

                fifoNum -= numElements;
              }
          }

          //////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Get Reception Fifo ID
          ///
          /// \param[in]  t  T coordinate
          ///
          /// \retval  recFifoID  Reception Fifo ID (global number across all
          ///                     subgroups).
          ///
          //////////////////////////////////////////////////////////////////////////

          uint32_t getRecFifoIdForDescriptor ( int t )
          {
            // \todo Map "t" to a subgroup and a fifo within that subgroup.
            //       For now, use the subgroup corresponding to our T and the first
            //       fifo within that subgroup.
            uint32_t subGroupNumber = t;
            uint32_t fifoNum        = 0;

            // Convert the subgroup and fifo number to a global fifo number.
            // The input "t" may be different than ours.  But, it is assumed
            // that the real fifo numbers in each subgroup are the same for all
            // subgroups.  This allows us to get the real fifo number from our
            // subgroup and use it in the calculation.

            return ( ( subGroupNumber * BGQ_MU_NUM_REC_FIFOS_PER_SUBGROUP ) +
                     _recFifoSubGroups[0]->getRealFifoNumber(fifoNum) );
          }

          //unsigned  getNumEagerFifos () { return _numEagerFifos; }
          unsigned  getNumInjFifos () { return _numInjFifos; }

          //int32_t  getCollectiveFifo () { return _collectiveFifoNumber; }

          static uint32_t registerReceiveFunction (MUSPI_RecvFunction_t    function,
                                                   void                    * arg)
          {
            uint32_t id;

            // Find a free registration id
            for ( id = 0; id < MUSPI_MAX_FUNCTION_IDS; id++ )
              {
                if ( _funcIDs[id] == 0 ) break;
              }

            PAMI_assert ( id < MUSPI_MAX_FUNCTION_IDS );

            MUSPI_RegisterRecvFunction ( function, arg, id );

            _funcIDs[id] = 1;

            return id;
          }


          //private:

          void getInfo();

          InjFifoSubGroup          **_injFifoSubGroups;
          RecFifoSubGroup          **_recFifoSubGroups;
          BaseAddressTableSubGroup **_batSubGroups;
          PAMI::Memory::MemoryManager *_mm;

          // Declare pointers to resource structures that will be allocated and
          // initialized by getInfo(), containing info about each of the resources
          // for the particular environment we are running in.

          Resource_t *_injFifoInfo, *_recFifoInfo, *_batInfo;

          // Resource mode
          ResourceMode_t _mode;

          // Resource type
          ResourceType_t _type;

          // Number of Eager Fifos
          //uint32_t _numEagerFifos;
          // Number of Injection Fifos
          uint32_t _numInjFifos;

          // First Eager Fifo Number
          //uint32_t _firstEagerFifoNumber;

          // Collective Fifo Number
          //uint32_t _collectiveFifoNumber;

          // Injection FIFO subgroup for remote get transfers.
          InjFifoSubGroup _rgetInjFifoSubgroup;

          ////////////////////////////////////////////////////////////////////////
          /// _funcIDs : global array (paired with the global MUSPI funcID array)
          ///   Array of indicators specifying which reception fifo registration
          ///   IDs are being used, and which are free.  The array index is the
          ///   funcID.  Each element is
          ///   - 0 when the ID is free
          ///   - 1 when the ID is in use.
          ///
          ////////////////////////////////////////////////////////////////////////

          static uint8_t  _funcIDs[MUSPI_MAX_FUNCTION_IDS];

      }; // PAMI::Device::MU::ResourceManager class
    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace
#undef TRACE
#endif   // __components_devices_bgq_mu_resourcemanager_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
