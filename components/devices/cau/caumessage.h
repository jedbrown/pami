/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/cau/caumessage.h
 * \brief ???
 */

#ifndef __components_devices_cau_caumessage_h__
#define __components_devices_cau_caumessage_h__

#include <pami.h>
#include "util/queue/MatchQueue.h"
#include "util/queue/Queue.h"
#include "components/devices/MulticastModel.h"
#include "components/devices/MulticombineModel.h"
#include "components/devices/MultisyncModel.h"
#include "math/math_coremath.h"
#include "algorithms/protocols/allreduce/ReduceFunctions.h"
#include "TypeDefs.h"

#ifdef TRACE
#undef TRACE
#define TRACE(x) //fprintf x
#else
#define TRACE(x) //fprintf x
#endif


namespace PAMI
{
namespace Device
{
//CAUDevice forward declaration.
class CAUDevice ;

class CAUGeometryInfo
{
public:
  CAUGeometryInfo(int cau_id, int geometry_id):
    _cau_id(cau_id),
    _geometry_id(geometry_id),
    _seqno(0),
    _ueBar(),
    _postedBar(),
    _seqnoRed(0),
    _ueRed(),
    _postedRed(),
    _seqnoBcast(0),
    _postedBcast(),
    _ueBcast()
  {}
  int             _cau_id;
  int             _geometry_id;
  unsigned        _seqno;
  MatchQueue      _ueBar;
  MatchQueue      _postedBar;

  unsigned        _seqnoRed;
  MatchQueue      _ueRed;
  MatchQueue      _postedRed;

  unsigned        _seqnoBcast;
  MatchQueue      _postedBcast;
  MatchQueue      _ueBcast;

//      PAMI::Topology *_topo;
};

class CAUMsyncMessage : public MatchQueueElem
{
public:

  static void cau_red_send_done(lapi_handle_t *hndl, void * completion_param)
  {
    (void)hndl;(void)completion_param;
    TRACE((stderr, "cau_red_send_done\n"));
  }

  static void cau_mcast_send_done(lapi_handle_t *hndl, void * completion_param)
  {
    (void)hndl;
    TRACE((stderr, "cau_mcast_send_done\n"));
    // This means that the synchronization is complete on the root node
    CAUMsyncMessage     *m          = (CAUMsyncMessage *)completion_param;
    void                *ms         = m->_toFree;
    pami_event_function  cleanup_fn = m->_cleanup_fn;
    m->_user_done_fn(m->_context,
                     m->_user_cookie,
                     PAMI_SUCCESS);

    if(cleanup_fn)//This will only be true if the message was allocated using memory allocator
      cleanup_fn(ms, m, PAMI_SUCCESS);
  }

  inline void advanceNonRoot()
  {
    CheckLapiRC(lapi_cau_reduce(_lapi_hdl,                   // lapi handle
                                _geometryInfo->_cau_id,      // group id
                                _dispatch_red_id,            // dispatch id
                                &_xfer_data[0],               // header
                                sizeof(_xfer_data),          // header_len
                                &_reduce_val,                 // data
                                sizeof(_reduce_val),         // data size
                                _red,                        // reduction op
                                cau_red_send_done,           // send completion handler
                                this));
  }

  inline void advanceRoot()
  {
    CheckLapiRC(lapi_cau_multicast(_lapi_hdl,                   // lapi handle
                                   _geometryInfo->_cau_id,      // group id
                                   _dispatch_mcast_id,          // dispatch id
                                   &_xfer_data[0],               // header
                                   sizeof(_xfer_data),          // header len
                                   &_reduce_val,                 // data
                                   sizeof(_reduce_val),         // data size
                                   cau_mcast_send_done,         // done cb
                                   this));                      // clientdata
  }


  CAUMsyncMessage(double               init_val,
                  cau_reduce_op_t      red,
                  pami_context_t       context,
                  pami_event_function  done_fn,
                  void                *user_cookie,
                  pami_event_function  cleanup_fn,
                  CAUGeometryInfo     *geometryInfo,
                  int                  dispatch_mcast_id,
                  int                  dispatch_red_id,
                  lapi_handle_t        lapi_handle,
                  void                *toFree):
    MatchQueueElem(geometryInfo->_seqno),
    _geometryInfo(geometryInfo),
    _reduce_val(init_val),
    _red(red),
    _user_done_fn(done_fn),
    _cleanup_fn(cleanup_fn),
    _user_cookie(user_cookie),
    _toFree(toFree),
    _context(context),
    _dispatch_mcast_id(dispatch_mcast_id),
    _dispatch_red_id(dispatch_red_id),
    _lapi_hdl(lapi_handle)
  {
  }
  CAUGeometryInfo     *_geometryInfo;
  double               _reduce_val;
  cau_reduce_op_t      _red;
  pami_event_function  _user_done_fn;
  pami_event_function  _cleanup_fn;
  void                *_user_cookie;
  void                *_toFree;
  pami_context_t       _context;
  int                  _xfer_data[3];
  int                  _dispatch_mcast_id;
  int                  _dispatch_red_id;
  lapi_handle_t        _lapi_hdl;
};

// CAU Multicombine Message Class
class CAUMcombineMessage : public MatchQueueElem
{
public:
  class               Header
  {
  public:
    unsigned           dispatch_id: 16;
    unsigned           geometry_id: 16;
    unsigned           seqno      : 32;
    unsigned           pktsize    : 7;
    unsigned           msgsize    : 25;
  }  __attribute__((__packed__));
  // When the reduction is done on the non-root nodes, we increment the bytes
  // consumed out of the user's buffer, we won't be accessing that any more
  // so update the pwq from the buffer and increment the total bytes consumed
  // Also, toggle the phase back to reduce
  static void cau_red_send_done(lapi_handle_t *hndl, void * completion_param)
  {
    (void)hndl;
    CAUMcombineMessage *m = (CAUMcombineMessage*)completion_param;
    unsigned bytesConsumed;

    if(m->_sizeoftype == 4)
      bytesConsumed = m->_reducePktBytes >> 1;
    else
      bytesConsumed = m->_reducePktBytes;

    m->_srcpwq->consumeBytes(bytesConsumed);
    m->_totalBytesConsumed += bytesConsumed;
    m->_reducePktBytes      = 0;
    m->_phase               = BROADCAST;
    TRACE((stderr, "cau_reduce_done: bc=%d tbc=%ld\n",
           bytesConsumed, m->_totalBytesConsumed));
    return;
  }

  // When the multicast is finished on the root node, we can
  // increment the bytes produced, indicating we are done with the
  // buffer.  We then move into the reduction phase again to accept
  // the next chunk of data from the non-root tasks.
  static void cau_mcast_send_done(lapi_handle_t *hndl, void * completion_param)
  {
    (void)hndl;
    CAUMcombineMessage *m = (CAUMcombineMessage*)completion_param;
    unsigned bytesProduced;

    if(m->_sizeoftype == 4)
      bytesProduced = m->_resultPktBytes >> 1;
    else
      bytesProduced = m->_resultPktBytes;

    m->_dstpwq->produceBytes(bytesProduced);
    m->_totalBytesProduced += bytesProduced;
    m->_resultPktBytes      = 0;
    m->_phase               = REDUCE;
    m->_resultPktBytes      = 0;
    TRACE((stderr, "cau_mcast_send_done: bc=%d tbp=%ld\n",
           bytesProduced, m->_totalBytesProduced));
    return;
  }

  typedef enum
  {
    NOOP = 0,
    BROADCAST,
    REDUCE
  } mcombine_phase_t;

  // These routines are here to implement a pesky CAU design feature
  // that requires 4 byte operands to be strided onto 8 byte boundaries
  inline void packBuf4(char *to, char * from, int count)
  {
    int i, j;
    unsigned  *fromCast = (unsigned*)from;
    unsigned  *toCast   = (unsigned*)to;

    for(i = 0, j = 0; i < count; i++, j += 2)
      toCast[j]   = fromCast[i];
  }
  inline void unpackBuf4(char *to, char * from, int count)
  {
    int i, j;
    unsigned *fromCast = (unsigned*)from;
    unsigned *toCast   = (unsigned*)to;

    for(i = 0, j = 0; i < count; i++, j += 2)
      toCast[i] = fromCast[j];
  }

  inline int pami_to_lapi_dt(pami_dt dt)
  {
    switch(dt)
      {
        case PAMI_SIGNED_INT:
          return CAU_SIGNED_INT;
          break;

        case PAMI_SIGNED_LONG:
#if defined(__64BIT__)
          return CAU_SIGNED_LONGLONG;
#else
          return CAU_SIGNED_INT;
#endif
          break;

        case PAMI_UNSIGNED_INT:
          return CAU_UNSIGNED_INT;
          break;

        case PAMI_UNSIGNED_LONG:
#if defined(__64BIT__)
          return CAU_UNSIGNED_LONGLONG;
#else
          return CAU_UNSIGNED_INT;
#endif
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

  CAUMcombineMessage(CAUDevice       *device,
                     CAUGeometryInfo *geometryInfo,
                     int              dispatch_red_id,
                     int              dispatch_mcast_id,
                     lapi_handle_t    lapi_hdl,
                     pami_context_t   context,
                     int              searchKey = -1):
    MatchQueueElem(searchKey),
    _geometryInfo(geometryInfo),
    _isInit(false),
    _isPosted(false),
    _dispatch_red_id(dispatch_red_id),
    _dispatch_mcast_id(dispatch_mcast_id),
    _device(device),
    _lapi_hdl(lapi_hdl),
    _context(context)
  {
  }
  void init(pami_multicombine_t *mcomb, pami_task_t taskid)
  {
    TRACE((stderr, "CAU Mcombine Msg:  Init\n"));
    _srcpwq = (PipeWorkQueue*)mcomb->data;
    _dstpwq = (PipeWorkQueue*)mcomb->results;
    _srctopo = (Topology*)mcomb->data_participants;
    _dsttopo = (Topology*)mcomb->results_participants;
    _red.operand_type = pami_to_lapi_dt(mcomb->dtype);
    _red.operation    = pami_to_lapi_op(mcomb->optor);
    CCMI::Adaptor::Allreduce::getReduceFunction(mcomb->dtype, mcomb->optor,
                                                _sizeoftype, _math_func);
    _totalCount = mcomb->count;
    _totalBytes = mcomb->count * _sizeoftype;
    _totalBytesConsumed = 0;
    _totalBytesProduced = 0;
    _reducePktBytes     = 0;
    _resultPktBytes     = 0;
    _cb_done            = mcomb->cb_done;
    _connection_id      = mcomb->connection_id;
    _key                = _geometryInfo->_seqnoRed++;

    if(_dsttopo->index2Endpoint(0) == taskid)
      _amRoot = true;
    else
      _amRoot = false;

    _phase = REDUCE;
    _isInit = true;
  }

  inline void advanceRoot()
  {
    if(!_isInit) return;

    unsigned  bytesAvailToConsume    = _srcpwq->bytesAvailableToConsume();
    unsigned  bytesAvailToProduce    = _dstpwq->bytesAvailableToProduce();
    char     *bufToConsume           = _srcpwq->bufferToConsume();
    char     *bufToProduce           = _dstpwq->bufferToProduce();

    if(_phase == REDUCE && _reducePktBytes)
      {
        unsigned actualDataInPacket;

        if(_sizeoftype == 4)
          actualDataInPacket = _reducePktBytes >> 1;
        else
          actualDataInPacket = _reducePktBytes;

        if(actualDataInPacket == 0 ||
            actualDataInPacket > bytesAvailToConsume ||
            actualDataInPacket > bytesAvailToProduce)
          return;

        if(_sizeoftype == 4)
          {
            char                    tmpPkt[64];
            unsigned                countInPacket = actualDataInPacket >> 2;
            packBuf4(tmpPkt, bufToConsume, countInPacket);
            void * inputs[] = {tmpPkt, _reducePkt};
            // Reduce the entire packet, forget the holes
            _math_func(_resultPkt, inputs, 2, countInPacket << 1);
          }
        else
          {
            void * inputs[] = {bufToConsume, _reducePkt};
            _math_func(_resultPkt, inputs, 2, actualDataInPacket >> 3);
          }

        _srcpwq->consumeBytes(actualDataInPacket);
        _totalBytesConsumed += actualDataInPacket;
        _resultPktBytes      = _reducePktBytes;
        _reducePktBytes      = 0;
        _phase = BROADCAST;
      }

    if(_phase == BROADCAST && bytesAvailToProduce && _resultPktBytes)
      {
        int bytesToBroadcast;

        if(_sizeoftype == 4)
          {
            unpackBuf4(bufToProduce, _resultPkt, _resultPktBytes >> 3);
            bytesToBroadcast = _resultPktBytes >> 1;
          }
        else
          {
            memcpy(bufToProduce, _resultPkt, _resultPktBytes);
            bytesToBroadcast = _resultPktBytes;
          }

        _xfer_header.dispatch_id = _dispatch_mcast_id;
        _xfer_header.geometry_id = _geometryInfo->_geometry_id;
        _xfer_header.seqno       = this->_key;
        _xfer_header.pktsize     = bytesToBroadcast;
        _xfer_header.msgsize     = _totalBytes;
        TRACE((stderr, "--->CAU Mcombine Advance Root: cau_multicast(inject) "
               "h.did=%d h.gid=%d h.seq=%d h.pktsz=%d, h.msz=%d\n",
               _xfer_header.dispatch_id, _xfer_header.geometry_id,
               _xfer_header.seqno, _xfer_header.pktsize,
               _xfer_header.msgsize));
        CheckLapiRC(lapi_cau_multicast(_lapi_hdl,              // lapi handle
                                       _geometryInfo->_cau_id, // group id
                                       _dispatch_mcast_id,     // dispatch id
                                       &_xfer_header,          // header
                                       sizeof(_xfer_header),   // header len
                                       bufToProduce,           // data
                                       bytesToBroadcast,       // data size
                                       cau_mcast_send_done,    // done cb
                                       this));                 // clientdata
      }

    return;
  }
  inline void advanceNonRoot()
  {
    if(!_isInit) return;

    unsigned  bytesAvailToConsume    = _srcpwq->bytesAvailableToConsume();
    unsigned  bytesAvailToProduce    = _dstpwq->bytesAvailableToProduce();
    char     *bufToConsume           = _srcpwq->bufferToConsume();
    char     *bufToProduce           = _dstpwq->bufferToProduce();

    if(_phase == REDUCE && _reducePktBytes == 0)
      {
        if(bytesAvailToConsume == 0) return;
        if(_sizeoftype == 4)
          {
            bytesAvailToConsume    = MIN(32, bytesAvailToConsume);
            unsigned countInPacket = bytesAvailToConsume >> 2;
            _reducePktBytes        = bytesAvailToConsume << 1;
            packBuf4(_reducePkt, bufToConsume, countInPacket);
          }
        else
          {
            bytesAvailToConsume    = MIN(64, bytesAvailToConsume);
            memcpy(_reducePkt, bufToConsume, bytesAvailToConsume);
            _reducePktBytes        = bytesAvailToConsume;
          }

        _xfer_header.dispatch_id = _dispatch_red_id;
        _xfer_header.geometry_id = _geometryInfo->_geometry_id;
        _xfer_header.seqno       = this->_key;
        _xfer_header.pktsize     = _reducePktBytes;
        _xfer_header.msgsize     = _totalBytes;
        TRACE((stderr, "--->CAU Mcombine Advance Nonroot: cau_reduce(inject) "
               "h.did=%d h.gid=%d h.seq=%d h.pktsz=%d, h.msz=%d\n",
               _xfer_header.dispatch_id, _xfer_header.geometry_id,
               _xfer_header.seqno, _xfer_header.pktsize,
               _xfer_header.msgsize));
        CheckLapiRC(lapi_cau_reduce(_lapi_hdl,                   // lapi handle
                                    _geometryInfo->_cau_id,      // group id
                                    _dispatch_red_id,            // dispatch id
                                    &_xfer_header,               // header
                                    sizeof(_xfer_header),        // header_len
                                    _reducePkt,                  // data
                                    _reducePktBytes,             // data size
                                    _red,                        // reduction op
                                    cau_red_send_done,           // send completion handler
                                    this));
      }

    if(_phase == BROADCAST && _resultPktBytes && bytesAvailToProduce)
      {
        if(bytesAvailToProduce == 0) return;

        unsigned actualDataInPacket;
        actualDataInPacket   = _resultPktBytes;
        memcpy(bufToProduce, _resultPkt, actualDataInPacket);
        _resultPktBytes      = 0;
        _totalBytesProduced += actualDataInPacket;
        _phase               = REDUCE;
        TRACE((stderr, "--->NonRoot:  producing %d bytes, bytesAvailToProduce=%d totbyteprod=%ld\n",
               actualDataInPacket, bytesAvailToProduce, _totalBytesProduced));
        _dstpwq->produceBytes(actualDataInPacket);
      }

    return;
  }
  // This message is inserted into the generic device, and the advance routine is called
  // for each message posted.  Each message toggles between a reduce phase and a
  // broadcast phase for each packet injected on the network.  Pipelining is handled
  // by returning EAGAIN when no buffer space is available to consume or produce, depending
  // on the current phase of the operation.
  inline pami_result_t advance()
  {
    PAMI_assert(_isInit == true);

    if(_amRoot) advanceRoot();
    else        advanceNonRoot();

    if(_totalBytesProduced == _totalBytes && _totalBytesConsumed == _totalBytes)
      {
        TRACE((stderr, "CAU Mcombine Advance:  Message Complete\n"));

        if(_isPosted) _geometryInfo->_postedRed.deleteElem(this);

        if(_cb_done.function)
          _cb_done.function(_context, _cb_done.clientdata, PAMI_SUCCESS);

        return PAMI_SUCCESS;
      }

    return PAMI_EAGAIN;
  }
  CAUGeometryInfo        *_geometryInfo;
  bool                    _isInit;
  bool                    _isPosted;
  int                     _dispatch_red_id;
  int                     _dispatch_mcast_id;
  PipeWorkQueue          *_srcpwq;
  PipeWorkQueue          *_dstpwq;
  Topology               *_srctopo;
  Topology               *_dsttopo;
  cau_reduce_op_t         _red;
  coremath                _math_func;
  unsigned                _sizeoftype;
  size_t                  _totalCount;
  unsigned                _totalBytes;
  unsigned                _totalBytesConsumed;
  unsigned                _totalBytesProduced;
  unsigned                _reducePktBytes;
  unsigned                _resultPktBytes;
  mcombine_phase_t        _phase;
  pami_callback_t         _cb_done;
  Generic::GenericThread *_workfcn;
  unsigned                _connection_id;
  bool                    _amRoot;
  Header                  _xfer_header;
  char                    _reducePkt[64];
  char                    _resultPkt[64];
  CAUDevice              *_device;
  lapi_handle_t           _lapi_hdl;
  pami_context_t          _context;
};

class CAUMcastMessage : public MatchQueueElem
{
public:
  class               Header
  {
  public:
    unsigned           dispatch_id: 16;
    unsigned           geometry_id: 16;
    unsigned           seqno      : 32;
    unsigned           pktsize    : 7;
    unsigned           msgsize    : 25;
  }  __attribute__((__packed__));

  class IncomingPacket: public Queue::Element
  {
  public:
    int   _size;
    char  _data[64];
  };

  static void cau_mcast_send_done(lapi_handle_t *hndl, void * completion_param)
  {
    (void)hndl;
    CAUMcastMessage *m      = (CAUMcastMessage*)completion_param;
    unsigned         bytes  = m->_xfer_header.pktsize;
    m->_totalBytesConsumed += bytes;
    m->_injectReady         = true;
    m->_srcpwq->consumeBytes(bytes);
    TRACE((stderr, "CAU Root Mcast Done:  bytes=%d tbc=%ld\n",
           bytes, m->_totalBytesConsumed));
    return;
  }
  CAUMcastMessage(CAUDevice       *device,
                  CAUGeometryInfo *geometryInfo,
                  int              dispatch_mcast_id,
                  lapi_handle_t    lapi_hdl,
                  pami_context_t   context,
                  int              searchKey = -1):
    MatchQueueElem(searchKey),
    _geometryInfo(geometryInfo),
    _isInit(false),
    _isPosted(false),
    _dispatch_mcast_id(dispatch_mcast_id),
    _injectReady(true),
    _device(device),
    _lapi_hdl(lapi_hdl),
    _context(context),
    _pkt_allocator(NULL)
  {
  }
  void init(pami_multicast_t *mcast, pami_task_t taskid, void *pkt_allocator)
  {
    TRACE((stderr, "CAU Mcast Msg:  Init\n"));
    _srcpwq             = (PipeWorkQueue*)mcast->src;
    _dstpwq             = (PipeWorkQueue*)mcast->dst;
    _srctopo            = (Topology*)mcast->src_participants;
    _dsttopo            = (Topology*)mcast->dst_participants;
    _totalBytes         = mcast->bytes;
    _totalBytesConsumed = 0;
    _totalBytesProduced = 0;
    _cb_done            = mcast->cb_done;
    _connection_id      = mcast->connection_id;
    _pkt_allocator      = pkt_allocator;
    _key                = _geometryInfo->_seqnoBcast++;

    if(_srctopo->index2Endpoint(0) == taskid)
      {
        _amRoot = true;
        _totalBytesProduced = _totalBytes;
      }
    else
      {
        _amRoot = false;
        _totalBytesConsumed = _totalBytes;
      }

    _isInit  = true;
  }

  inline void advanceRoot()
  {
    if(!_isInit) return;

    unsigned  bytesAvailToConsume    = _srcpwq->bytesAvailableToConsume();
    char     *bufToConsume           = _srcpwq->bufferToConsume();

    while(bytesAvailToConsume && _injectReady)
      {
        TRACE((stderr, "Advance Root (%p):  bac=%d buf=%p, injectReady=%d\n",
               this, bytesAvailToConsume, bufToConsume, _injectReady));
        unsigned bytesToBroadcast= MIN(bytesAvailToConsume, 64);
        _xfer_header.dispatch_id = _dispatch_mcast_id;
        _xfer_header.geometry_id = _geometryInfo->_geometry_id;
        _xfer_header.seqno       = this->_key;
        _xfer_header.pktsize     = bytesToBroadcast;
        _xfer_header.msgsize     = _totalBytes;
        TRACE((stderr, "CAU Root Mcast:  buf=%p bytes=%d\n",
               bufToConsume, bytesToBroadcast));
        PAMI_assert(bytesAvailToConsume >= bytesToBroadcast);
        _injectReady = false; // Set this here in case multicast done is called in inject
        CheckLapiRC(lapi_cau_multicast(_lapi_hdl,              // lapi handle
                                       _geometryInfo->_cau_id, // group id
                                       _dispatch_mcast_id,     // dispatch id
                                       &_xfer_header,          // header
                                       sizeof(_xfer_header),   // header len
                                       bufToConsume,           // data
                                       bytesToBroadcast,       // data size
                                       cau_mcast_send_done,    // done cb
                                       this));                 // clientdata
        bytesAvailToConsume    = _srcpwq->bytesAvailableToConsume();
        bufToConsume           = _srcpwq->bufferToConsume();
      }

    return;
  }
  inline void advanceNonRoot()
  {
    if(!_isInit) return;

    unsigned  bytesAvailToProduce    = _dstpwq->bytesAvailableToProduce();
    char     *bufToProduce           = _dstpwq->bufferToProduce();

    while(bytesAvailToProduce && _packetQueue.size())
      {
        TRACE((stderr, "Advance NonRoot(%p):  bap=%d buf=%p, pqsz=%d\n",
               this, bytesAvailToProduce, bufToProduce, _packetQueue.size()));
        IncomingPacket * pkt = (IncomingPacket*)_packetQueue.dequeue();
        memcpy(bufToProduce, pkt->_data, pkt->_size);
        PAMI_assert(bytesAvailToProduce >= (unsigned)pkt->_size);
        _dstpwq->produceBytes(pkt->_size);
        _totalBytesProduced += pkt->_size;
        bytesAvailToProduce = _dstpwq->bytesAvailableToProduce();
        bufToProduce        = _dstpwq->bufferToProduce();
        ((PAMI::MemoryAllocator<sizeof(IncomingPacket), 16> *)_pkt_allocator)->returnObject(pkt);
      }

    return;
  }
  inline pami_result_t advance()
  {
    PAMI_assert(_isInit == true);

    if(_amRoot) advanceRoot();
    else        advanceNonRoot();

    if(_totalBytesProduced == _totalBytes && _totalBytesConsumed == _totalBytes)
      {
        TRACE((stderr, "CAU Mcombine Advance:  Message Complete\n"));

        if(_isPosted) _geometryInfo->_postedBcast.deleteElem(this);

        if(_cb_done.function)
          _cb_done.function(_context, _cb_done.clientdata, PAMI_SUCCESS);

        return PAMI_SUCCESS;
      }

    return PAMI_EAGAIN;
  }
  CAUGeometryInfo        *_geometryInfo;
  bool                    _isInit;
  bool                    _isPosted;
  int                     _dispatch_mcast_id;
  PipeWorkQueue          *_srcpwq;
  PipeWorkQueue          *_dstpwq;
  Topology               *_srctopo;
  Topology               *_dsttopo;
  unsigned                _sizeoftype;
  unsigned                _totalBytes;
  unsigned                _totalBytesConsumed;
  unsigned                _totalBytesProduced;
  pami_callback_t         _cb_done;
  Generic::GenericThread *_workfcn;
  unsigned                _connection_id;
  bool                    _amRoot;
  bool                    _injectReady;
  Header                  _xfer_header;
  Queue                   _packetQueue;
  CAUDevice              *_device;
  lapi_handle_t           _lapi_hdl;
  pami_context_t          _context;
  void                   *_pkt_allocator;
};
};
};

#endif // __components_devices_cau_caubasemessage_h__
