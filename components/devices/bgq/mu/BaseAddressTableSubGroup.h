/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
////////////////////////////////////////////////////////////////////////////////
///
/// \file components/devices/bgq/mu/BaseAddressTableSubGroup.h
///
/// \brief Base Address Table SubGroup definitions.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef __components_devices_bgq_mu_BaseAddressTableSubGroup_h__
#define __components_devices_bgq_mu_BaseAddressTableSubGroup_h__

#include <errno.h>

#include <hwi/include/bqc/MU_Macros.h>
#include <spi/include/mu/Addressing.h>
#include <spi/include/mu/Addressing_inlines.h>

#include "util/common.h"

namespace XMI
{
  namespace Device
  {
    namespace MU
    {
      //////////////////////////////////////////////////////////////////////////
      ///
      /// \brief Base Address Table SubGroup Class
      ///
      /// This class provides interfaces for accessing a base address table
      /// subgroup.  It assumes that MU initialization has previously occurred,
      /// including job initialization.
      ///
      /// The following interfaces are provided:
      ///
      /// Constructor              - Prepares the class object for use.
      /// init                     - Initializes the base address table
      ///                            subgroup.
      /// setBaseAddress           - Given a virtual address, set its
      ///                            corresponding physical address into a
      ///                            base address table entry.
      ///
      //////////////////////////////////////////////////////////////////////////

      class BaseAddressTableSubGroup
      {
        public:

          ////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Construct a Base Address Table subgroup
          ///
          /// This constructor essentially does nothing but initialize
          /// member data.  The init() function must be called before
          /// this object can be usable.
          ///
          ////////////////////////////////////////////////////////////////////////

          BaseAddressTableSubGroup () {}


          ////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Initialize the Base Address Table SubGroup
          ///
          /// \param[in]  subGroupId  The subgroup ID to be managed by this
          ///                         Base Address Table Subgroup object (0..67).
          /// \param[in]  numEntries  Number of base address table entries to be
          ///                         allocated.
          ///
          /// \retval  0         Successful initialization
          /// \retval  non-zero  Value indicates the error
          ///
          ////////////////////////////////////////////////////////////////////////

          int init ( uint32_t subGroupId,
                     uint32_t numEntries );


          ////////////////////////////////////////////////////////////////////////
          ///
          /// \brief Set Base Address
          ///
          /// \param[in]  entryNum  Entry number to be set (0 to the number of
          ///                       entries managed by this object).
          /// \param[in]  va        Virtual address to be set.  This is converted
          ///                       into a physical address and set into the
          ///                       entry.  Note: A zero virtual address will
          ///                       be a zero physical address.
          ///
          /// \retval  0  Success
          /// \retval  -1 Error
          ///
          ////////////////////////////////////////////////////////////////////////

          inline int32_t setBaseAddress ( uint32_t  entryNum,
                                          void     *va )
          {
            XMI_assert_debug ( entryNum < BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP );

            MUHWI_BaseAddress_t pa;
            int32_t  rc;

            // A zero virtual address will be a zero physical address
            if ( va == NULL ) pa = 0;
            else
              {
                // Convert the virtual address to a physical one.
                // \todo: Add a call to Kernel_Virtual2Physical here.
                pa = (MUHWI_BaseAddress_t)va;
              }

            rc = MUSPI_SetBaseAddress ( &_baseAddressTableSubGroup,
                                        _entryNumbers[entryNum],
                                        pa );
            return rc;
          }


        private:

          ////////////////////////////////////////////////////////////////////////
          ///
          /// Private member data:
          ///
          /// _baseAddressTableSubGroup
          ///   The base address table subgroup structure.
          ///
          /// _numEntries
          ///   Number of entries in this subgroup being used by this object.
          ///
          /// _entryNumbers
          ///   The actual entry numbers in this group, as known by the MU.
          ///   Users of this object use entry numbers 0 through _numEntries-1.
          ///   This array maps from those entry numbers to the actual MU
          ///   entry numbers.
          ///
          ////////////////////////////////////////////////////////////////////////

          MUSPI_BaseAddressTableSubGroup_t _baseAddressTableSubGroup;
          uint32_t                         _numEntries;
          uint32_t           _entryNumbers[BGQ_MU_NUM_DATA_COUNTERS_PER_SUBGROUP];

      }; // XMI::Device::MU::BaseAddressTableSubGroup class
    };   // XMI::Device::MU namepsace
  };     // XMI::Device namespace
};       // XMI namespace

#endif   // __components_devices_bgq_mu_baseaddresstablesubgroup_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
