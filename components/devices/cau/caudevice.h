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
#include <map>

namespace PAMI
{
  extern std::map<unsigned, pami_geometry_t> geometry_map;
  namespace Device
  {
    #define DISPATCH_ID_START 48
    
    extern std::map<int,int> _g_lapi_mcast_to_pami_mcast;
    extern std::map<int,int> _g_pami_mcast_to_lapi_mcast;
    extern std::map<lapi_handle_t,void*> _g_id_to_device_table;


    class CAUDevice: public PAMI::Device::Interface::BaseDevice<CAUDevice>
    {
    public:
      CAUDevice():
	_lapi_state(NULL),
        _lapi_handle(0),
        _dispatch_id(DISPATCH_ID_START)
	{}
      inline void          init(lapi_state_t  *lapi_state,
                                lapi_handle_t  lapi_handle,
                                pami_client_t  client,
                                pami_context_t context)
        {
          _lapi_state  = lapi_state;
          _lapi_handle = lapi_handle;
          lapi_qenv(_lapi_handle, TASK_ID, (int *)&_taskid);
        }
      inline lapi_state_t  *getState() { return _lapi_state;}
      inline lapi_handle_t  getHdl() { return _lapi_handle;}
      inline int            registerSyncDispatch(hdr_hndlr_t *hdr,
                                                 void        *clientdata)
        {
          CheckLapiRC(lapi_addr_set(_lapi_handle,(void*)hdr,_dispatch_id));
          _g_id_to_device_table[_dispatch_id++] = clientdata;
          return _dispatch_id-1;
        }

      inline int            registerMcastDispatch(int          pami_dispatch_id,
                                                  hdr_hndlr_t *hdr,
                                                  void        *clientdata)
        {
          CheckLapiRC(lapi_addr_set(_lapi_handle,(void*)hdr,_dispatch_id));
          _g_id_to_device_table[_dispatch_id]            = clientdata;
          _g_pami_mcast_to_lapi_mcast[pami_dispatch_id]  = _dispatch_id;
          _g_lapi_mcast_to_pami_mcast[_dispatch_id]      = pami_dispatch_id;
          _dispatch_id++;
          return _dispatch_id-1;
        }
      
      inline pami_task_t          taskid()
        {
          return _taskid;
        }

      pami_context_t getContext()
        {
          return _context;
        }
      pami_context_t getClient()
        {
          return _context;
        }

      static inline void         *getClientData(int id)
        {
          return _g_id_to_device_table[id];
        }
      static pami_geometry_t mapidtogeometry (int comm)
        {
          pami_geometry_t g = geometry_map[comm];
          return g;
        }
      static inline int          getPamiId(int lapi_id)
        {
          return _g_lapi_mcast_to_pami_mcast[lapi_id];
        }
      static inline int          getLapiId(int pami_id)
        {
          return _g_pami_mcast_to_lapi_mcast[pami_id];
        }

    private:
      lapi_state_t                        *_lapi_state;
      lapi_handle_t                        _lapi_handle;
      pami_client_t                        _client;
      pami_context_t                       _context;
      int                                  _dispatch_id;
      pami_task_t                          _taskid;
    };
  };
};
#endif // __components_devices_cau_caupacketdevice_h__
