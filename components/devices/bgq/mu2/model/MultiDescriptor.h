/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu2/MultiDescriptor.h
 * \brief ???
 */
#ifndef __components_devices_bgq_mu2_MultiDescriptor_h__
#define __components_devices_bgq_mu2_MultiDescriptor_h__

#include <spi/include/mu/DescriptorBaseXX.h>

namespace PAMI
{
  namespace Device
  {
    namespace MU
    {
      template <unsigned T_Num>
      class MultiDescriptor
      {
        public:

          inline MultiDescriptor ()
          {
	    memset(desc, 0, sizeof(desc));
	    
            MUSPI_BaseDescriptorInfoFields_t base;
	    memset(&base, 0, sizeof(base));

            base.Pre_Fetch_Only  = MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO;
            base.Payload_Address = 0;
            base.Message_Length  = 0;
            base.Torus_FIFO_Map  = 0;
            base.Dest.Destination.Destination = 0;

            size_t i;
            for (i = 0; i < T_Num; i++)
              {
                desc[i].setBaseFields (&base);
              }
          };

          inline ~MultiDescriptor () {};

          inline void clone (MultiDescriptor<T_Num> * clone)
          {
            size_t i;
            for (i = 0; i < T_Num; i++)
              {
                desc[i].clone (clone->desc[i]);
              }
          };


          MUSPI_DescriptorBase   desc[T_Num] __attribute__((__aligned__(64)));

      }; // PAMI::Device::MU::MultiDescriptor class

      typedef MultiDescriptor<1> Descriptor;

    };   // PAMI::Device::MU namespace
  };     // PAMI::Device namespace
};       // PAMI namespace

#endif // __components_devices_bgq_mu2_MultiDescriptor_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//

