/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/cau/caudevice.h
 * \brief ???
 */

#ifndef __components_devices_cau_caudevice_h__
#define __components_devices_cau_caudevice_h__

#include "components/devices/BaseDevice.h"
#include "components/devices/cau/caumessage.h"

namespace PAMI
{
  namespace Device
  {
    class CAUDevice: public PAMI::Device::Interface::BaseDevice<CAUDevice>
    {
    public:
      CAUDevice():
	_lapi_state(NULL)
	{}
	inline void          init(lapi_state_t *lapi_state) {_lapi_state=lapi_state;}
	inline lapi_state_t *getState() { return _lapi_state;}
    private:
	lapi_state_t                          *_lapi_state;
    };

  };
};
#endif // __components_devices_cau_caupacketdevice_h__
