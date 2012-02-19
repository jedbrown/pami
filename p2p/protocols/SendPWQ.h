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
///
/// \file p2p/protocols/SendPWQ.h
/// \brief Defines a protocol extension for sends using a PWQ
///
#ifndef __p2p_protocols_SendPWQ_h__
#define __p2p_protocols_SendPWQ_h__

#include <pami.h> // \todo remove? PAMI_Context_post()

#include "p2p/protocols/Send.h" // assumed basis of T_Protocol

#include "util/trace.h"

#include "util/ccmi_debug.h"

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
  namespace Protocol
  {
    namespace Send
    {
      static char send_trace_once = DO_TRACE_ENTEREXIT;
      ///
      /// \brief Template class to extend point-to-point send with PWQ support.
      ///
      /// Send side only.  Receive/dispatch still uses the protocol's
      /// flat buffer dispatch.
      ///
      /// T_Protocol is assumed to be some send protocol:
      /// \see PAMI::Protocol::Send::Send
      ///
      /// This class is NOT constructed, it is an extension of an existing
      /// protocol that is used by casting the protocol to this class.
      ///
      /// This was done because protocol constructor and factory generates
      /// are not standard so SendPWQ can't overload them.
      ///
      /// Restrictions:
      /// Because it is NOT constructed, it may not define member data or
      /// new virtual member functions in this class
      ///
      /// \todo Pass in a generic work device for posting instead of
      /// using the c function - PAMI_Context_post()?  Hard to do without
      /// member data. Could be passed on each call and stored in sendpwq_t?
      ///
      /// Example of usage:
      ///
      /// // Define a protocol
      /// typedef Protocol::Send::...<> MyProtocol;
      ///
      /// // Define a SendPWQ over this protocol
      /// typedef PAMI::Protocol::Send::SendPWQ < MyProtocol > MySendPWQ;
      ///
      /// // Generate the protocol
      /// MySendPWQ *protocol = (MySendPWQ*) MySendPWQ::generate(...);
      ///
      /// NOTE: generate will call MyProtocol::generate and return MyProtocol*,
      /// so it IS  necessary to cast it to MySendPWQ*.  This constructs
      /// MyProtocol but does not actually call any MySendPWQ constructor,
      /// thus the restrictions note above.
      ///
      /// You may then call member functions on MySendPWQ or MyProtocol
      ///
      /// protocol->immediatePWQ();  // Call new PWQ immediate on MySendPWQ.
      ///
      /// protocol->immediate();    // Call regular p2p immediate send.
      ///
      /// protocol->otherProtocolFunction(); // Call other protocol functions.
      ///
      template < class T_Protocol >
      class SendPWQ : public T_Protocol
      {
      public:
        typedef struct
        {
          pami_work_t               work;
          enum {SIMPLE,IMMEDIATE}   type;
          union
          {
            pami_send_immediate_t       immediate;
            pami_send_t                 simple;
          }                         send;
          PAMI::PipeWorkQueue      *pwq;
          SendPWQ                  *pthis;
          PAMI::Topology            dst_participants;
          pami_client_t             client;
          size_t                    contextid;
          size_t                    clientid;
        } sendpwq_t;


        ///
        /// \brief Async work function. Try to resend the data in the pwq if it's ready
        ///
        /// \param[in]  context   Work context
        /// \param[in]  cookie    Original send state to resend.
        ///
        static pami_result_t work_function(pami_context_t context, void *cookie)
        {
          pami_result_t result = PAMI_EAGAIN;
          if(send_trace_once) TRACE_FN_ENTER();
          sendpwq_t * state = (sendpwq_t*)cookie;

          if(send_trace_once) TRACE_FORMAT( "<%p> context %p, cookie %p",state->pthis,context, cookie);

          if (state->type == sendpwq_t::IMMEDIATE)
          {
//            if(state->pwq->bytesAvailableToConsume() < state->send.immediate->data.iov_len)
//              return PAMI_EAGAIN; // this doesn't actually re-queue the work?
            result = state->pthis->immediatePWQ(state, context);
          }
          else
          {
//            if(state->pwq->bytesAvailableToConsume() < state->send.simple->send.data.iov_len)
//              return PAMI_EAGAIN; // this doesn't actually re-queue the work?
            result = state->pthis->simplePWQ(state, context);
          }
          if(send_trace_once) TRACE_FORMAT( "<%p> context %p, cookie %p",state->pthis,context, cookie);
          if(send_trace_once) TRACE_FN_EXIT();
          return PAMI_SUCCESS;
        }
        ///
        /// \brief Start a new immediate send message with PWQ. If there is no
        /// data ready, post an async work function to retry later.
        ///
        /// \param[in]  sendpwq_t      *state     state storage and parameters:
        /// 
        ///             pami_work_t               work; 
        ///             enum {SIMPLE,IMMEDIATE}   type;
        ///             union
        ///             {
        ///               pami_send_immediate_t       immediate;
        ///               pami_send_t                 simple;
        ///             }                         send;
        ///             PAMI::PipeWorkQueue      *pwq;
        ///             SendPWQ                  *pthis;
        ///             PAMI::Topology            dst_participants;
        ///             pami_client_t             client;
        ///             size_t                    contextid;
        ///             size_t                    clientid;
        /// 
        /// \param[in]  pami_context_t  context
        ///
        pami_result_t immediatePWQ(sendpwq_t* state, pami_context_t context)
        {
          if(send_trace_once) TRACE_FN_ENTER();
          pami_result_t result = PAMI_EAGAIN;
          pami_send_immediate_t * parameters = &state->send.immediate;

          PAMI::PipeWorkQueue * pwq = state->pwq;

          if(send_trace_once) TRACE_FORMAT( "<%p> state %p, context %p, parameters %p, pwq %p, ndest %zu",this, state, context, parameters, pwq, state->dst_participants.size());
          size_t length = pwq? pwq->bytesAvailableToConsume() : 0;
          void* payload = pwq?(void*)pwq->bufferToConsume(): NULL;
          if(send_trace_once) TRACE_FORMAT( "<%p> length %zd/%zd, payload %p  data[%.2u..%.2u]",this, length,(size_t)parameters->data.iov_len, payload, *(char*)payload,*(char*)((char*)payload+length-1));

          // send it now if there is enough data in the pwq
          if (length >= parameters->data.iov_len)
          {
            if(!send_trace_once) TRACE_FN_ENTER();
            if(!send_trace_once) TRACE_FORMAT( "<%p> state %p, context %p, parameters %p, pwq %p, ndest %zu",this, state, context, parameters, pwq, state->dst_participants.size());
            if(!send_trace_once) TRACE_FORMAT( "<%p> length %zd/%zd, payload %p data[%.2u..%.2u]",this, length,(size_t)parameters->data.iov_len, payload, payload?*(char*)payload:-1,payload?*(char*)((char*)payload+length-1):-1);
            parameters->data.iov_base = payload;
            parameters->data.iov_len = length;
            size_t size = state->dst_participants.size();
            for (unsigned i = 0; i < size; ++i)
            {
              parameters->dest = state->dst_participants.index2Endpoint(i);

              TRACE_FORMAT( "<%p> send(%u(%zu))", this, parameters->dest, state->contextid);
              result = this->immediate (parameters);
              TRACE_FORMAT( "<%p> result %u", this, result);

            }
            TRACE_FN_EXIT(); send_trace_once = DO_TRACE_ENTEREXIT;
            return result;

          }
          // not enough data to send yet, post it to the context work queue for later processing
          if(send_trace_once) TRACE_FORMAT( "<%p> queue it on context %p",this,context);
          state->type = sendpwq_t::IMMEDIATE;
          state->pthis = this;

          /// \todo Pass in a generic/work device so we can directly post

          if(send_trace_once) TRACE_FN_EXIT(); send_trace_once = 0;
          PAMI_Context_post (context,(pami_work_t*)state, work_function, (void *) state);
          return result;

          // circular header dependencies if I try to use Context
//          PAMI::Context * ctx = (PAMI::Context *) context;
//          return ctx->post(state, work_function, (void *) state)
        }

        ///
        /// \brief Start a new simple P2P send with PWQ.
        ///
        /// \param[in]  context    Send context
        /// \param[in]  dest       Destination endpoint
        /// \param[in]  header_len Send header length in bytes.
        /// \param[in]  header     A pointer to header data
        /// \param[in]  bytes      Send data length in bytes.
        /// \param[in]  pwq        A pipe work queue to use for the data buffer
        /// \param[in]  events     Send completion events/cookie structure
        /// \param[in]  dispatch   Dispatch id
        ///
        pami_result_t simplePWQ (
          pami_context_t       context,
          pami_endpoint_t      dest,
          size_t               header_length,
          void                *header,
          size_t               bytes,
          PAMI::PipeWorkQueue *pwq,
          pami_send_event_t   *events,
          size_t               dispatch)
        {
          TRACE_FN_ENTER();
          pami_result_t result = PAMI_EAGAIN;

          TRACE_FORMAT( "<%p> context %p, pwq %p, bytes %zu, dest %zu",this, context, pwq, bytes, (size_t)dest);
          size_t length = pwq? pwq->bytesAvailableToConsume() : 0;
          void* payload = pwq?(void*)pwq->bufferToConsume(): NULL;
          TRACE_FORMAT( "<%p> length %zd, payload %p",this, length,payload);

          // send it now if there is enough data in the pwq
          if (length >= bytes)
          {
            pami_send_t s;
            memset(&s.send.hints, 0, sizeof(s.send.hints));
            s.send.dest = dest;
            s.events = *events;
            s.send.dispatch = dispatch;
            s.send.header.iov_base = header;
            s.send.header.iov_len = header_length;
            s.send.data.iov_base = payload;
            s.send.data.iov_len = length;
            TRACE_FORMAT( "<%p> send(%u(%p))", this, s.send.dest, context);
            //fprintf(stderr, "CCMI:: SHORT send to EP%d\n", dest);
            result =  this->simple(&s);
            TRACE_FORMAT( "<%p> result %u", this, result);
            TRACE_FN_EXIT();
            return result;
          }
          // \todo not enough data to send yet, ...
          TRACE_FORMAT( "<%p> result %u", this, result);
          TRACE_FN_EXIT();
          return result; // PAMI_EAGAIN
        }
        ///
        /// \brief Start a new simple send message with PWQ.  If there is no
        /// data ready, post an async work function to retry later.
        ///
        /// \param[in]  sendpwq_t      *state     state storage and parameters:
        /// 
        ///             pami_work_t               work; 
        ///             enum {SIMPLE,IMMEDIATE}   type;
        ///             union
        ///             {
        ///               pami_send_immediate_t       immediate;
        ///               pami_send_t                 simple;
        ///             }                         send;
        ///             PAMI::PipeWorkQueue      *pwq;
        ///             SendPWQ                  *pthis;
        ///             PAMI::Topology            dst_participants;
        ///             pami_client_t             client;
        ///             size_t                    contextid;
        ///             size_t                    clientid;
        /// 
        /// \param[in]  pami_context_t  context
        ///
        pami_result_t simplePWQ (sendpwq_t* state, pami_context_t context)
        {
          if(send_trace_once) TRACE_FN_ENTER();
          pami_result_t result = PAMI_EAGAIN;
          pami_send_t * parameters = &state->send.simple;

          PAMI::PipeWorkQueue * pwq = state->pwq;

          if(send_trace_once) TRACE_FORMAT( "<%p> state %p, context %p, parameters %p, pwq %p, ndest %zu",this, state, context, parameters, pwq, state->dst_participants.size());
          size_t length = pwq? pwq->bytesAvailableToConsume() : 0;
          void* payload = pwq?(void*)pwq->bufferToConsume(): NULL;
          if(send_trace_once) TRACE_FORMAT( "<%p> length %zd/%zd, payload %p  data[%.2u..%.2u]",this, length,(size_t)parameters->send.data.iov_len, payload, payload?*(char*)payload:-1,payload?*(char*)((char*)payload+length-1):-1);

          // send it now if there is enough data in the pwq
          if (length >= parameters->send.data.iov_len)
          {
            if(!send_trace_once) TRACE_FN_ENTER();
            if(!send_trace_once) TRACE_FORMAT( "<%p> state %p, context %p, parameters %p, pwq %p, ndest %zu",this, state, context, parameters, pwq, state->dst_participants.size());
            if(!send_trace_once) TRACE_FORMAT( "<%p> length %zd/%zd, payload %p  data[%.2u..%.2u]",this, length,(size_t)parameters->send.data.iov_len, payload, *(char*)payload,*(char*)((char*)payload+length-1));
            parameters->send.data.iov_base = payload;
            parameters->send.data.iov_len = length;
            size_t size = state->dst_participants.size();
            for (unsigned i = 0; i < size; ++i)
            {
              parameters->send.dest = state->dst_participants.index2Endpoint(i);

              TRACE_FORMAT( "<%p> send(%u(%zu))", this, parameters->send.dest, state->contextid);

              result =  this->simple (parameters);
              TRACE_FORMAT( "<%p> result %u", this, result);
            }
            TRACE_FN_EXIT();send_trace_once = DO_TRACE_ENTEREXIT;
            return result;


          }
          // not enough data to send yet, post it to the context work queue for later processing
          if(send_trace_once) TRACE_FORMAT( "<%p> queue it on context %p",this, context);
          state->type = sendpwq_t::SIMPLE;
          state->pthis = this;

          /// \todo Pass in a generic/work device so we can directly post
          if(send_trace_once) TRACE_FN_EXIT(); send_trace_once = 0;
          PAMI_Context_post (context, (pami_work_t*)state, work_function, (void *) state);
          return result;
          // circular header dependencies if I try to use Context
//          PAMI::Context * ctx = (PAMI::Context *) context;
//          return ctx->post(state, work_function, (void *) state);
        }

      }; // PAMI::Protocol::SendPWQ class

      template < class T_Protocol >
      class SendWrapperPWQ : public T_Protocol, public SendPWQ<PAMI::Protocol::Send::Send>
      {
      };

    }; // PAMI::Protocol::Send namespace
  };   // PAMI::Protocol namespace
};     // PAMI namespace
 #undef DO_TRACE_ENTEREXIT
 #undef DO_TRACE_DEBUG

#endif // __pami_p2p_protocols_SendPWQ_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
