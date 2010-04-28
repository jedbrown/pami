/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

/**
 * \file components/devices/bgp/collective_network/CNAllreduce.cc
 * \brief Collective Network allreduce implementation.
 */

#include "config.h"
#include <pami.h>
#include <util/common.h>
#include "components/devices/bgp/collective_network/CNAllreduce.h"
#include "components/devices/bgp/collective_network/CollectiveNetworkLib.h"
#include "spi/bgp_SPI.h"

extern "C" size_t _g_num_active_nodes;

namespace PAMI {
namespace Device {
namespace BGP {

        CNAllreduceSetup CNAllreduceSetup::CNAllreduceSetupCache[PAMI_OP_COUNT][PAMI_DT_COUNT];

        void CNAllreduceSetup::initCNAS() {
                for (unsigned op = PAMI_UNDEFINED_OP; op < PAMI_OP_COUNT; ++op) {
                for (unsigned dt = PAMI_UNDEFINED_DT; dt < PAMI_DT_COUNT; ++dt) {
                        new (&CNAllreduceSetupCache[op][dt])
                                CNAllreduceSetup((pami_dt)dt, (pami_op)op);
                }}
        }

      void preprocSum1PDouble(void *out, void *in, int c)
      {
        int count = 0;
        char *inptr = (char*)in;
        char *outptr= (char*)out;

        for(count = 0; count<c; count++)
          {
            double2uint((double*)inptr, BGPCN_PKT_SIZE / sizeof(double), (uint64_t*)outptr);
            inptr+=sizeof(double);
            outptr+=BGPCN_PKT_SIZE;
          }
      }
      void postprocSum1PDouble(void *out, void *in, int c)
      {
        int count = 0;
        char *inptr = (char*)in;
        char *outptr= (char*)out;
        for(count = 0; count<c; count++)
          {
            uint2double(_g_num_active_nodes,
                        (double*)outptr,
                        BGPCN_PKT_SIZE / sizeof(double),
                        (uint64_t*)inptr);

            inptr+=BGPCN_PKT_SIZE;
            outptr+=sizeof(double);
          }
      }

      void preprocSum1PFloat(void *out, void *in, int c)
      {
        int count = 0;
        char *inptr = (char*)in;
        char *outptr= (char*)out;

        float  tempF;
        double tempD;
        for(count = 0; count<c; count++)
          {
            tempF = *((float*)inptr);
            tempD = (double) tempF;
            double2uint(&tempD, BGPCN_PKT_SIZE / sizeof(double), (uint64_t*)outptr);
            inptr+=sizeof(float);
            outptr+=BGPCN_PKT_SIZE;
          }
      }
      void postprocSum1PFloat(void *out, void *in, int c)
      {
        int count = 0;
        char *inptr = (char*)in;
        char *outptr= (char*)out;
        float  tempF;
        double tempD;

        for(count = 0; count<c; count++)
          {
            uint2double(_g_num_active_nodes,
                        &tempD,
                        BGPCN_PKT_SIZE / sizeof(double),
                        (uint64_t*)inptr);
            tempF = (double) tempD;
            *((float*)outptr) = tempF;
            inptr+=BGPCN_PKT_SIZE;
            outptr+=sizeof(float);
          }
      }

}; // namespace BGP
}; // namespace Device
}; // PAMI
