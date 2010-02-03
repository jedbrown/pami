/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUCollDevice.h
 * \brief Collective subclass of the MU device
 */

#ifndef __components_devices_bgq_mu_MUCollDevice_h__
#define __components_devices_bgq_mu_MUCollDevice_h__

#include "components/devices/bgq/mu/MUDevice.h"
#ifdef	_KERNEL_CNK_COLLECTIVE_IMPL_H_ /* Prevent multiple inclusion */
#warning NO CNK support for classroute collectives and it doesnt link
#include <hwi/include/bqc/classroute.h>
#include <spi/include/kernel/collective.h>
#endif

#ifdef TRACE
#undef TRACE
#endif
#define TRACE(x) //fprintf x

#define ENABLE_MAMBO_WORKAROUNDS


namespace XMI
  {

    namespace Device
      {

        namespace MU
          {

            void dumpDescriptor(const char * pstring, const MUHWI_Descriptor_t *desc);
            void dumpHexData(const char * pstring, const uint32_t *buffer, size_t n_ints);


            class MUCollDevice : public MUDevice//, public Interface::BaseDevice<MUCollDevice, SysDep>
              {

                public:
                  MUCollDevice() : MUDevice (),
                      //Interface::BaseDevice<MUCollDevice, SysDep> (),
                      _fnum(0),
                      _injFifoSubGroup(NULL),
                      _relativeFnum(0)
                  {
                    TRACE((stderr, "<%p>MUCollDevice::ctor \n", this));
                  };

                  ~MUCollDevice()
                  {
                    TRACE((stderr, "<%p>MUCollDevice::dtor \n", this));
                  };

                  ///
                  /// \see init
                  ///
                  /// Uggh.  I want to hook into MUDevice's init_impl (through BaseDevices's init()).  So hide init() with my own init().
                  inline int init (SysDep        * sysdep,
                                   xmi_context_t   context,
                                   size_t          offset)
                  {
                    TRACE((stderr, "<%p>MUCollDevice::init() \n", this));
                    return MUCollDevice::init_impl(sysdep, context, offset);
                  };

//////////////////////////////////////////////////////
//  Uggh alternative to above Uggh.
//  One way to hook into init_impl is multi-inheritence but then I have to hide these BaseDevice functions and explicitly call the parent that I want.
//      int init(SysDep* sysdep)
//      {
//        return XMI::Device::Interface::BaseDevice<MUCollDevice, SysDep>::init(sysdep);
//      };
//      int advance()
//      {
//        return XMI::Device::Interface::BaseDevice<MUDevice, SysDep>::advance();
//      }
//      size_t peers()
//      {
//        return XMI::Device::Interface::BaseDevice<MUDevice, SysDep>::peers();
//      }
//      size_t task2peer(size_t task)
//      {
//        return XMI::Device::Interface::BaseDevice<MUDevice, SysDep>::task2peer(task);
//      }
//////////////////////////////////////////////////////

#warning Clean these up
      static inline MUCollDevice *create(size_t clientid, size_t num_ctx, void *not_used_yet) {
	size_t x;
	MUCollDevice *devs;
	int rc = posix_memalign((void **)&devs, 16, sizeof(*devs) * num_ctx);
	XMI_assertf(rc == 0, "posix_memalign failed for MUCollDevice[%zd], errno=%d\n", num_ctx, errno);
	for (x = 0; x < num_ctx; ++x) {
		new (&devs[x]) MUCollDevice();
	}
	return devs;
      }

	inline void init(SysDep *sd, size_t client, size_t num_ctx, xmi_context_t context, size_t contextid) {
		MUDevice *dev = &this[contextid];
		dev->init_impl(sd, context, contextid);
	}

                  ///
                  /// \see advanceInjectionFifoDescriptorTail
                  ///
                  inline bool nextInjectionDescriptor (MUSPI_InjFifo_t    ** injfifo,
                                                       MUHWI_Descriptor_t ** desc,
                                                       void               ** payloadVa,
                                                       void               ** payloadPa)
                  {
                    TRACE((stderr, "<%p>MUCollDevice::nextInjectionDescriptor() \n", this));

                    if (!_colChannel->isEmptyMsgQ(_fnum)) return false;

                    bool success = _injFifoSubGroup->nextDescriptor (_relativeFnum, injfifo, desc, payloadVa, payloadPa);

                    return success;
                  }

                  inline void addToDoneQ (MUDescriptorWrapper * wrapper)
                  {
                    TRACE((stderr, "<%p>MUCollDevice::addToDoneQ() \n", this));
                    _injFifoSubGroup->addToDoneQ(_relativeFnum, wrapper);
                  }

                  inline void addToSendQ (XMI::Queue::Element * msg)
                  {
                    TRACE((stderr, "<%p>MUCollDevice::addToSendQ() \n", this));
                    _injFifoSubGroup->addToSendQ(_relativeFnum, msg);
                  }

                  //////////////////////////////////////////////////////////////////////////
                  ///
                  /// \brief Get Reception Fifo ID
                  ///
                  /// \param[in]  p  P coordinate
                  ///
                  /// \retval  recFifoID  Reception Fifo ID (global number across all
                  ///                     subgroups).
                  ///
                  //////////////////////////////////////////////////////////////////////////

                  uint32_t getRecFifoIdForDescriptor ( int p )
                  {
                    TRACE((stderr, "<%p>MUCollDevice::getRecFifoIdForDescriptor() \n", this));
                    return _colChannel->getRecFifoIdForDescriptor ( p );
                  }

                  uint16_t getRgetInjFifoId (size_t target_rank)
                  {
                    TRACE((stderr, "<%p>MUCollDevice::getRgetInjFifoId() \n", this));
                    return _colChannel->getRgetInjFifoId (target_rank);
                  }

                  int init_impl (SysDep * sysdep, xmi_context_t context, size_t contextid)
                  {
                    TRACE((stderr, "<%p>MUCollDevice::init_impl() \n", this));
                    int rc = MUDevice::init_impl(sysdep, context, contextid);

                    XMI_assert(_initialized);

                    _fnum = _colChannel->pinFifo (0);
                    getSubGroupAndRelativeFifoNum (_fnum,
                                                   &_injFifoSubGroup,
                                                   &_relativeFnum);

                    /// \todo Implement when CNK supports class routes
#ifdef _KERNEL_CNK_COLLECTIVE_IMPL_H_ /* Prevent multiple inclusion */
#warning NO CNK support for classroute
                    // Set up class route to have a local contribution from this node with no output.
                    ClassRoute_t classRouteInfo;
                    memset(&classRouteInfo, 0x00, sizeof(classRouteInfo));
                    classRouteInfo.id    = 2;  /// \todo global class route always 2 (arbitrary)?
                    classRouteInfo.input = BGQ_COLL_CLASS_INPUT_LINK_LOCAL | BGQ_COLL_CLASS_INPUT_VC_USER;
                    classRouteInfo.output = 0;
                    rc = Kernel_AllocateClassRoute ( &classRouteInfo );

                    if ( rc )
                      {
                        XMI_abortf("AllocateClassRoute failed with rc %d\n", rc);
                      }

#endif
                    return rc;
                  }

                protected:
                  uint32_t          _fnum;
                  InjFifoSubGroup * _injFifoSubGroup;
                  uint32_t          _relativeFnum;

                private:
              }; // XMI::Device::MU::MUCollDevicee class
          };   // XMI::Device::MU namespace
      };     // XMI::Device namespace
  };       // XMI namespace


#undef TRACE

#endif // __components_devices_bgq_mu_mucolldevice_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
