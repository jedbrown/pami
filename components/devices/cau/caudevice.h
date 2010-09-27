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
    #define DISPATCH_ID_START 48

    extern std::map<int,int> _g_lapi_mcast_to_pami_mcast;
    extern std::map<int,int> _g_pami_mcast_to_lapi_mcast;
    extern std::map<lapi_handle_t,void*> _g_id_to_device_table;


    class CAUDevice: public Interface::BaseDevice<CAUDevice>
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
                                size_t         client_id,
                                pami_context_t context,
                                size_t         context_id)
        {
          _lapi_state  = lapi_state;
          _lapi_handle = lapi_handle;
          _client      = client;
          _client_id   = client_id;
          _context     = context;
          _context_id  = context_id;
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

      void           setGenericDevices(Generic::Device *generics)
        {
          _generics = generics;
        }

      Generic::GenericThread * postWork( pami_work_function work_fn, void *cookie)
        {
          Generic::GenericThread *work =
            (Generic::GenericThread *) _work_alloc.allocateObject();
          work = new (work) Generic::GenericThread(work_fn, cookie);
          work->setStatus(Ready);
          _generics[_context_id].postThread(work);
          return work;
        }

      void freeWork(Generic::GenericThread *work)
        {
          _work_alloc.returnObject(work);
        }


      inline const bool   multicombine_model_op_support(pami_dt dt, pami_op op)
        {
          const bool support[PAMI_DT_COUNT][PAMI_OP_COUNT] =
          {
        //  PAMI_UNDEFINED_OP, PAMI_NOOP, PAMI_MAX, PAMI_MIN, PAMI_SUM, PAMI_PROD, PAMI_LAND, PAMI_LOR, PAMI_LXOR, PAMI_BAND, PAMI_BOR, PAMI_BXOR, PAMI_MAXLOC, PAMI_MINLOC, PAMI_USERDEFINED_OP,
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_UNDEFINED_DT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_SIGNED_CHAR
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_UNSIGNED_CHAR
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_SIGNED_SHORT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_UNSIGNED_SHORT
            {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_SIGNED_INT
            {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_UNSIGNED_INT
            {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_SIGNED_LONG_LONG
            {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_UNSIGNED_LONG_LONG
            {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_FLOAT
            {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_DOUBLE
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LONG_DOUBLE
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOGICAL
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_SINGLE_COMPLEX
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_DOUBLE_COMPLEX
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_2INT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_SHORT_INT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_FLOAT_INT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_DOUBLE_INT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_2FLOAT
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_2DOUBLE
            {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false} //PAMI_USERDEFINED_DT
          };
          TRACE((stderr, "Multicombine::multicombine_model_op_support(%d, %d) = %s\n", dt, op, support[dt][op] ? "true" : "false"));
          return(support[dt][op]);
        }



      inline int pami_to_lapi_dt(pami_dt dt)
        {
          switch(dt)
            {
              case PAMI_SIGNED_INT:
                return CAU_SIGNED_INT;
                break;
              case PAMI_UNSIGNED_INT:
                return CAU_UNSIGNED_INT;
                break;
              case PAMI_SIGNED_LONG_LONG:
                return CAU_SIGNED_LONGLONG;
                break;
              case PAMI_UNSIGNED_LONG_LONG:
                return CAU_UNSIGNED_LONGLONG;
                break;
              case PAMI_FLOAT:
                return CAU_FLOAT;
                break;
              case PAMI_DOUBLE:
                return CAU_DOUBLE;
                break;
              default:
                return -1;
                break;
            }
          return -1;
        }

      inline int pami_to_lapi_op(pami_op op)
        {
          switch(op)
            {
              case PAMI_SUM:
                return CAU_SUM;
                break;
              case PAMI_MIN:
                return CAU_MIN;
                break;
              case PAMI_MAX:
                return CAU_MAX;
                break;
              case PAMI_BAND:
                return CAU_AND;
                break;
              case PAMI_BXOR:
                return CAU_XOR;
                break;
              case PAMI_BOR:
                return CAU_OR;
                break;
              default:
                return -1;
                break;
            }
          return -1;
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
      lapi_state_t                                              *_lapi_state;
      lapi_handle_t                                              _lapi_handle;
      pami_client_t                                              _client;
      pami_context_t                                             _context;
      size_t                                                     _client_id;
      size_t                                                     _context_id;
      int                                                        _dispatch_id;
      pami_task_t                                                _taskid;
      Generic::Device                                           *_generics;
      PAMI::MemoryAllocator<sizeof(Generic::GenericThread), 16>  _work_alloc;
    };
  };
};
#endif // __components_devices_cau_caupacketdevice_h__
