/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/MultisyncModel.h
 * \brief ???
 */

#ifndef __components_devices_MultisyncModel_h__
#define __components_devices_MultisyncModel_h__

#include <pami.h>
#include "util/common.h"

namespace PAMI
{
  namespace Device
  {
    namespace Interface
    {
      ///
      /// \todo Need A LOT MORE documentation on this interface and its use
      /// \param T_Model   Multisync model template class
      ///
      /// \see Multisync::Model
      ///
      template <class T_Model,class T_Device,unsigned T_StateBytes>
      class MultisyncModel
      {
      public:
        /// \param[in] device                Multisync device reference
        MultisyncModel (T_Device &device, pami_result_t &status)
          {
            COMPILE_TIME_ASSERT(T_Model::sizeof_msg == T_StateBytes);
            status = PAMI_SUCCESS;
          };
        ~MultisyncModel ()
          {

          };
        inline pami_result_t postMultisync(uint8_t (&state)[T_StateBytes],
                                          pami_multisync_t *msync);
      }; // class MultisyncModel

      template <class T_Model,class T_Device, unsigned T_StateBytes>
      pami_result_t MultisyncModel<T_Model,T_Device,T_StateBytes>::postMultisync(uint8_t (&state)[T_StateBytes],
                                                                        pami_multisync_t *msync)
      {
        return static_cast<T_Model*>(this)->postMultisync_impl(state, msync);
      }

    }; // namespace Interface
  }; // namespace Device
}; // namespace PAMI
#endif // __components_devices_MultisyncModel_h__
