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
/// \file components/devices/bgq/mu/ResourceManager.cc
///
/// \brief Resource Manager implementations.
///
////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stddef.h>

#include "components/devices/bgq/mu/ResourceManager.h"
#include "Global.h"

#ifndef TRACE
#define TRACE(x) printf x
#endif

uint8_t XMI::Device::MU::ResourceManager::_funcIDs[MUSPI_MAX_FUNCTION_IDS];

void XMI::Device::MU::ResourceManager::
getInfo()
{
  uint32_t i;

  // \todo Determine the environment and set mode
  _mode                 = MINIMAL;

// \todo P2P_TYPPE and COLLECTIVE_TYPE (_type) configuration will differ in general

  switch ( _mode )
    {
      case MINIMAL:


        //_numEagerFifos        = MINIMAL_NUM_EAGER_FIFOS;
        //_firstEagerFifoNumber = MINIMAL_FIRST_EAGER_FIFO_NUMBER;
        //_collectiveFifoNumber = MINIMAL_COLLECTIVE_FIFO_NUMBER;

        _numInjFifos  = MINIMAL_INJ_NUM_SUBGROUPS * MINIMAL_INJ_SUBGROUP_NUM_FIFOS;

        // Allocate space for the resource structure.  At the end of the structure
        // are MINIMAL_INJ_NUM_SUBGROUPS Resource_SubGroup_t structures defining
        // each of the subgroups in this channel, specifying the subGroupIds and
        // number of elements (injFifos) in each subgroup.
        _injFifoInfo = (Resource_t*)malloc ( sizeof(Resource_t) +
                                             (MINIMAL_INJ_NUM_SUBGROUPS - 1) *
                                             sizeof (Resource_SubGroup_t) );
        XMI_assert ( _injFifoInfo != NULL );
        _injFifoInfo->numSubGroups = MINIMAL_INJ_NUM_SUBGROUPS;

	//        TRACE(("My Coords are: A=%d, B=%d, C=%d, D=%d, E=%d, T=%d, P=%d, aSize=%d, bSize=%d, cSize=%d, dSize=%d, eSize=%d, tSize=%d, pSize=%d\n",
	//_mapping->a(), _mapping->b(), _mapping->c(), _mapping->d(), _mapping->e(), _mapping->t(), _mapping->p(), _mapping->aSize(), _mapping->bSize(), _mapping->cSize(), _mapping->dSize(), _mapping->eSize(), _mapping->tSize(), _mapping->pSize()))
        //;

        for (i = 0; i < MINIMAL_INJ_NUM_SUBGROUPS; i++)
          {
            /// \todo For now, use 'p' coordinate (processor id) as the subgroup number
            _injFifoInfo->subGroupInfo[i].subGroupId  = __global.mapping.p();
            _injFifoInfo->subGroupInfo[i].numElements = MINIMAL_INJ_SUBGROUP_NUM_FIFOS;
          }

        // Allocate space for the resource structure.  At the end of the structure
        // are MINIMAL_INJ_NUM_SUBGROUPS Resource_SubGroup_t structures defining
        // each of the subgroups in this channel, specifying the subGroupIds and
        // number of elements (recFifos) in each subgroup.
        _recFifoInfo = (Resource_t*)malloc ( sizeof(Resource_t) +
                                             (MINIMAL_REC_NUM_SUBGROUPS - 1) *
                                             sizeof (Resource_SubGroup_t) );
        XMI_assert ( _recFifoInfo != NULL );
        _recFifoInfo->numSubGroups = MINIMAL_REC_NUM_SUBGROUPS;

        for (i = 0; i < MINIMAL_REC_NUM_SUBGROUPS; i++)
          {
            /// \todo For now, use 'p' coordinate (processor id) as the subgroup number
            _recFifoInfo->subGroupInfo[i].subGroupId  = __global.mapping.p();
            _recFifoInfo->subGroupInfo[i].numElements = MINIMAL_REC_SUBGROUP_NUM_FIFOS;
          }


        // Allocate space for the resource structure.  At the end of the structure
        // are MINIMAL_INJ_NUM_SUBGROUPS Resource_SubGroup_t structures defining
        // each of the subgroups in this channel, specifying the subGroupIds and
        // number of elements (BAT entries) in each subgroup.
        _batInfo = (Resource_t*)malloc ( sizeof(Resource_t) +
                                         (MINIMAL_BAT_NUM_SUBGROUPS - 1) *
                                         sizeof (Resource_SubGroup_t) );
        XMI_assert ( _batInfo != NULL );
        _batInfo->numSubGroups = MINIMAL_BAT_NUM_SUBGROUPS;

        for (i = 0; i < MINIMAL_BAT_NUM_SUBGROUPS; i++)
          {
            _batInfo->subGroupInfo[i].subGroupId  = __global.mapping.p();
            _batInfo->subGroupInfo[i].numElements = MINIMAL_BAT_SUBGROUP_NUM_ENTRIES;
          }

        break;

      case DEFAULT:
      default:
        break;

    };

}


int XMI::Device::MU::ResourceManager::
init ( ResourceType_t  type,
       SysDep *sd,
       dispatch_t      *dispatch )
{
  int rc = 0;
  uint32_t i, j;

  _type = type;
  _sysdep = sd;

  // Initialize the resource structures for the particular environment we are
  // running in.
  getInfo ();

  //////////////////////////////////////////////////////////////////////////////
  //
  // Initialize the remote-get injection fifos:
  //
  //////////////////////////////////////////////////////////////////////////////

  // - Allocate space for the InjFifoSubGroup object for this subgroup and
  //   run its constructor.
  char * fifoPtr[1];
  uint32_t fifoSize[1];
  Kernel_InjFifoAttributes_t fifoAttr[1];

  rc = posix_memalign ( (void**) & fifoPtr[0],
                        INJ_FIFO_ALIGNMENT,
                        DEFAULT_INJ_FIFO_DESC_COUNT *
                        (sizeof(MUHWI_Descriptor_t) +
                         sizeof(torus_packet_payload_t)));
  XMI_assert ( rc == 0 );

  fifoSize[0] = DEFAULT_INJ_FIFO_DESC_COUNT * sizeof (MUHWI_Descriptor_t);
  fifoAttr[0].RemoteGet = 1;
  fifoAttr[0].System    = 0;

  rc = _rgetInjFifoSubgroup.init ( 67, 1, (char **) & fifoPtr[0], &fifoSize[0], &fifoAttr[0] );



  //////////////////////////////////////////////////////////////////////////////
  //
  // Initialize the injection fifos:
  // - Allocate space for the array of pointers to the InjFifoSubGroups
  //
  //////////////////////////////////////////////////////////////////////////////

  _injFifoSubGroups = (InjFifoSubGroup**)malloc ( _injFifoInfo->numSubGroups *
                                                  sizeof ( InjFifoSubGroup * ) );
  XMI_assert ( _injFifoSubGroups != NULL );
  TRACE(("_injFifoSubGroups=%p\n", _injFifoSubGroups));

  // - Loop through each subgroup, initializing it.

  for ( i = 0; i < _injFifoInfo->numSubGroups; i++ )
    {
      uint32_t numElements = _injFifoInfo->subGroupInfo[i].numElements;

      // - Allocate space for the InjFifoSubGroup object for this subgroup and
      //   run its constructor.
      _injFifoSubGroups[i] = (InjFifoSubGroup*)malloc ( sizeof(InjFifoSubGroup) );
      XMI_assert ( _injFifoSubGroups[i] != NULL );
      new (_injFifoSubGroups[i]) InjFifoSubGroup;
      TRACE(("_injFifoSubGroups[%u]=%p\n", i, _injFifoSubGroups[i]));

      // - Allocate space for the fifo pointers.
      // - Allocate space for the fifo sizes.

      char **fifoPtrs = (char**)malloc ( numElements * sizeof ( char* ) );
      XMI_assert ( fifoPtrs != NULL );

      uint32_t *fifoSizes = (uint32_t*)malloc ( numElements * sizeof ( uint32_t ) );
      XMI_assert ( fifoSizes != NULL );

      Kernel_InjFifoAttributes_t *fifoAttrs =
        (Kernel_InjFifoAttributes_t*)malloc ( numElements * sizeof ( Kernel_InjFifoAttributes_t ) );
      XMI_assert ( fifoAttrs != NULL );
      TRACE(("fifoPtrs=%p, fifoSizes=%p, fifoAttrs=%p\n", fifoPtrs, fifoSizes, fifoAttrs));

      // - Allocate space for each fifo - and the single packet payload buffer
      // - Set each fifo's size.

      for ( j = 0; j < numElements; j++ )
        {
          rc = posix_memalign ( (void**) & fifoPtrs[j],
                                INJ_FIFO_ALIGNMENT,
                                DEFAULT_INJ_FIFO_DESC_COUNT *
                                (sizeof(MUHWI_Descriptor_t) +
                                 sizeof(torus_packet_payload_t)));
          XMI_assert ( rc == 0 );

          fifoSizes[j] = DEFAULT_INJ_FIFO_DESC_COUNT * sizeof (MUHWI_Descriptor_t);
          fifoAttrs[j].RemoteGet = 0;
          fifoAttrs[j].System    = 0;

          TRACE(("fifoPtrs[%u]=%p, fifoSizes=%u\n", j, fifoPtrs[j], fifoSizes[j]));
        }

      // - Initialize the InjFifoSubGroup.

      rc = _injFifoSubGroups[i]->init ( _injFifoInfo->subGroupInfo[i].subGroupId,
                                        numElements,
                                        fifoPtrs,
                                        fifoSizes,
                                        fifoAttrs );

      if (rc) break;

      TRACE(("Initialized InjFifoSubGroups for subgroup %u\n", _injFifoInfo->subGroupInfo[i].subGroupId));

      free ( fifoSizes );
      free ( fifoPtrs  );
      free ( fifoAttrs );

    } // End:  Loop through each subgroup, initializing it.

  if (rc) return rc;

  //////////////////////////////////////////////////////////////////////////////
  //
  // Initialize the reception fifos:
  // - Allocate space for the array of pointers to the RecFifoSubGroups
  //
  //////////////////////////////////////////////////////////////////////////////

  _recFifoSubGroups = (RecFifoSubGroup**)malloc ( _recFifoInfo->numSubGroups *
                                                  sizeof ( RecFifoSubGroup * ) );
  XMI_assert ( _recFifoSubGroups != NULL );
  TRACE(("_recFifoSubGroups=%p\n", _recFifoSubGroups));

  // - Loop through each subgroup, initializing it.

  for ( i = 0; i < _recFifoInfo->numSubGroups; i++ )
    {
      uint32_t numElements = _recFifoInfo->subGroupInfo[i].numElements;

      // - Allocate space for the RecFifoSubGroup object for this subgroup and
      //   run its constructor.
      _recFifoSubGroups[i] = (RecFifoSubGroup*)malloc ( sizeof(RecFifoSubGroup) );
      XMI_assert ( _recFifoSubGroups[i] != NULL );
      new (_recFifoSubGroups[i]) RecFifoSubGroup;
      TRACE(("_recFifoSubGroups[%u]=%p\n", i, _recFifoSubGroups[i]));

      // - Allocate space for the fifo pointers.
      // - Allocate space for the fifo sizes.

      char **fifoPtrs = (char**)malloc ( numElements * sizeof ( char* ) );
      XMI_assert ( fifoPtrs != NULL );

      uint32_t *fifoSizes = (uint32_t*)malloc ( numElements * sizeof ( uint32_t ) );
      XMI_assert ( fifoSizes != NULL );

      Kernel_RecFifoAttributes_t *fifoAttrs =
        (Kernel_RecFifoAttributes_t*)malloc ( numElements * sizeof ( Kernel_RecFifoAttributes_t ) );
      XMI_assert ( fifoAttrs != NULL );
      TRACE(("fifoPtrs=%p, fifoSizes=%p, fifoAttrs=%p\n", fifoPtrs, fifoSizes, fifoAttrs));

      // - Allocate space for each fifo.
      // - Set each fifo's size.

      for ( j = 0; j < numElements; j++ )
        {
          rc = posix_memalign ( (void**) & fifoPtrs[j], REC_FIFO_ALIGNMENT, DEFAULT_REC_FIFO_SIZE );
          XMI_assert ( rc == 0 );

          fifoSizes[j] = DEFAULT_REC_FIFO_SIZE;
          fifoAttrs[j].System = 0;
          TRACE(("fifoPtrs[%u]=%p, fifoSizes=%u\n", j, fifoPtrs[j], fifoSizes[j]));
        }

      // - Initialize the RecFifoSubGroup.

      rc = _recFifoSubGroups[i]->init ( _recFifoInfo->subGroupInfo[i].subGroupId,
                                        numElements,
                                        fifoPtrs,
                                        fifoSizes,
                                        fifoAttrs,
                                        dispatch );

      if (rc) break;

      TRACE(("Initialized RecFifoSubGroups for subgroup %u\n", _recFifoInfo->subGroupInfo[i].subGroupId));

      free ( fifoSizes );
      free ( fifoPtrs  );
      free ( fifoAttrs );

    } // End:  Loop through each subgroup, initializing it.

  if (rc) return rc;

  //////////////////////////////////////////////////////////////////////////////
  //
  // Initialize the base address tables:
  // - Allocate space for the array of pointers to the BaseAddressTableSubGroups
  //
  //////////////////////////////////////////////////////////////////////////////

  _batSubGroups = (BaseAddressTableSubGroup**)malloc ( _batInfo->numSubGroups *
                                                       sizeof ( BaseAddressTableSubGroup * ) );
  XMI_assert ( _batSubGroups != NULL );
  TRACE(("_batSubGroups=%p\n", _batSubGroups));

  // - Loop through each subgroup, initializing it.

  for ( i = 0; i < _batInfo->numSubGroups; i++ )
    {
      uint32_t numElements = _batInfo->subGroupInfo[i].numElements;

      // - Allocate space for the BaseAddressTableSubGroup object for this subgroup
      //   and run its constructor.
      _batSubGroups[i] = (BaseAddressTableSubGroup*)malloc ( sizeof(BaseAddressTableSubGroup) );
      XMI_assert ( _batSubGroups[i] != NULL );
      new (_batSubGroups[i]) BaseAddressTableSubGroup();
      TRACE(("_batSubGroups[%u]=%p\n", i, _batSubGroups[i]));

      // - Initialize the BaseAddressTableSubGroup.

      rc = _batSubGroups[i]->init ( _batInfo->subGroupInfo[i].subGroupId,
                                    numElements );

      if (rc) break;

      TRACE(("Initialized BatSubGroups for subgroup %u\n", _batInfo->subGroupInfo[i].subGroupId));

    } // End:  Loop through each subgroup, initializing it.


  if (rc) return rc;

  // Set the first subgroup's Base Address Table Entry slot 0 to have a base
  // address of 0.

  rc = _batSubGroups[0]->setBaseAddress ( BAT_DEFAULT_ENTRY_NUMBER,
                                          NULL );

  // Allocate a "shared counter"
  uint64_t * shared_counter_va = (uint64_t *) malloc (sizeof(uint64_t) + sizeof(Kernel_MemoryRegion_t));


  Kernel_MemoryRegion_t * memRegion = (Kernel_MemoryRegion_t *)(shared_counter_va + 1);
  rc = Kernel_CreateMemoryRegion (memRegion, shared_counter_va, sizeof(uint64_t));
  XMI_assert ( rc == 0 );

  uint64_t shared_counter_pa = (uint64_t)memRegion->BasePa +
                               ((uint64_t)shared_counter_va - (uint64_t)memRegion->BaseVa);

  rc = _batSubGroups[0]->setBaseAddress ( BAT_SHAREDCOUNTER_ENTRY_NUMBER,
                                          (void *)MUSPI_GetAtomicAddress (shared_counter_pa, MUHWI_ATOMIC_OPCODE_STORE_ADD_COHERENCE_ON_ZERO) );


  return rc;

} // End: init()
