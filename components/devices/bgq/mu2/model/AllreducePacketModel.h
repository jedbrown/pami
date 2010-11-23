/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/model/AllreducePacketModel.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_model_AllreducePacketModel_h__
#define __components_devices_bgq_mu2_model_AllreducePacketModel_h__

#include "components/devices/bgq/mu2/model/CollectivePacketModelBase.h"

#include "util/ccmi_debug.h"
#include "util/ccmi_util.h"
#include "util/trace.h"

#ifdef CCMI_TRACE_ALL
 #define DO_TRACE_ENTEREXIT 1
 #define DO_TRACE_DEBUG     1
#else
 #define DO_TRACE_ENTEREXIT 0
 #define DO_TRACE_DEBUG     0
#endif


namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      class AllreducePacketModel : public CollectivePacketModelBase<AllreducePacketModel, MUHWI_COLLECTIVE_TYPE_ALLREDUCE, MUHWI_PACKET_VIRTUAL_CHANNEL_USER_SUB_COMM>
      {
        public :

          /// \see PAMI::Device::Interface::CollectivePacketModel::CollectivePacketModel
          inline AllreducePacketModel (MU::Context & context) :
              CollectivePacketModelBase<AllreducePacketModel, MUHWI_COLLECTIVE_TYPE_ALLREDUCE, MUHWI_PACKET_VIRTUAL_CHANNEL_USER_SUB_COMM> (context)
          {
            TRACE_FN_ENTER();
            TRACE_FN_EXIT();
          };

          /// \see PAMI::Device::Interface::CollectivePacketModel::~CollectivePacketModel
          inline ~AllreducePacketModel ()
          {
            TRACE_FN_ENTER();
            TRACE_FN_EXIT();
          };

      }; // PAMI::Device::MU::AllreducePacketModel class

    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace


#endif // __components_devices_bgq_mu2_model_AllreducePacketModel_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
