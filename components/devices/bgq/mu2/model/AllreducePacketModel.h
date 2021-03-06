/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/model/AllreducePacketModel.h
 * \brief A simple model for Allreduce MU collective packets
 */
#ifndef __components_devices_bgq_mu2_model_AllreducePacketModel_h__
#define __components_devices_bgq_mu2_model_AllreducePacketModel_h__

#include "components/devices/bgq/mu2/model/CollectivePacketModelBase.h"

#include "util/ccmi_debug.h"
#include "util/ccmi_util.h"
#include "util/trace.h"

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

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
      class AllreducePacketModel : public CollectivePacketModelBase<AllreducePacketModel, MUHWI_COLLECTIVE_TYPE_ALLREDUCE, PAMI_MU_CR_PKT_VC>
      {
        public :

          /// \see PAMI::Device::Interface::CollectivePacketModel::CollectivePacketModel
          inline AllreducePacketModel (MU::Context & context) :
              CollectivePacketModelBase<AllreducePacketModel, MUHWI_COLLECTIVE_TYPE_ALLREDUCE, PAMI_MU_CR_PKT_VC> (context)
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

#undef DO_TRACE_ENTEREXIT
#undef DO_TRACE_DEBUG

#endif // __components_devices_bgq_mu2_model_AllreducePacketModel_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
