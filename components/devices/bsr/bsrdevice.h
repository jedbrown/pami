/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bsr/bsrdevice.h
 * \brief ???
 */

#ifndef __components_devices_bsr_bsrdevice_h__
#define __components_devices_bsr_bsrdevice_h__

#include "components/devices/BaseDevice.h"
#include "components/devices/bsr/bsrmessage.h"

#ifdef TRACE
#undef TRACE
#define TRACE(x)// fprintf x
#else
#define TRACE(x)// fprintf x
#endif


namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;
  namespace Device
  {
    class BSRDevice: public Interface::BaseDevice<BSRDevice>
    {
    public:
      BSRDevice(): _initialized(false)
	{
        }

      inline void          init(pami_client_t  client,
                                pami_context_t context,
                                size_t         context_id,
                                pami_task_t    my_task_id)
        {
          _client     = client;
          _context    = context;
          _my_task_id = my_task_id;

          _initialized = true;
        }

      pami_context_t getContext_impl()
        {
          return _context;
        }
      pami_context_t getClient_impl()
        {
          return _client;
        }

      bool isInit_impl()
        {
          return _initialized;
        }

      pami_task_t taskid()
        {
          return _my_task_id;
        }

      void       setGenericDevices(Generic::Device *generics)
        {
          _generics = generics;
        }

      void postWork(BSRMsyncMessage *m)
        {
          m->setStatus(Ready);
          _generics[_context_id].postThread(m);
        }

    private:
      bool                      _initialized;
      pami_task_t               _my_task_id;
      pami_client_t             _client;
      size_t                    _context_id;
      pami_context_t            _context;
      Generic::Device          *_generics;
    };
  };
};
#endif // __components_devices_bsr_bsrpacketdevice_h__
