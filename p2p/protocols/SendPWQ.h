/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
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

/// \todo put this trace facility somewhere common
#include "util/trace.h"

#include "util/ccmi_debug.h"

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
      static char trace_once = DO_TRACE_ENTEREXIT;
      ///
      /// \brief Template class to extend point-to-point send with PWQ support.
      ///
      /// \details Send side only.  Receive/dispatch still uses the protocol's
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
      /// \example of usage:
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
          if(trace_once) TRACE_FN_ENTER();
          sendpwq_t * state = (sendpwq_t*)cookie;
          if(trace_once) TRACE_FORMAT( "<%p> context %p, cookie %p",state->pthis,context, cookie);

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
          if(trace_once) TRACE_FORMAT( "<%p> context %p, cookie %p",state->pthis,context, cookie);
          if(trace_once) TRACE_FN_EXIT();
          return PAMI_SUCCESS;
        }
        ///
        /// \brief Start a new immediate send message with PWQ. If there is no
        /// data ready, post an async work function to retry later.
        ///
        /// \param[in]  task      Destination task.
        /// \param[in]  src       Send data buffer.
        /// \param[in]  bytes     Send data length in bytes.
        /// \param[in]  msginfo   Opaque application header information.
        /// \param[in]  mbytes    Number of opaque application header bytes.
        ///
        /// \param[in]  pwq       A pipe work queue to use for the data buffer
        ///
        pami_result_t immediatePWQ(sendpwq_t* state, pami_context_t context)
        {
          if(trace_once) TRACE_FN_ENTER();
          pami_result_t result = PAMI_EAGAIN;
          pami_send_immediate_t * parameters = &state->send.immediate;

          PAMI::PipeWorkQueue * pwq = state->pwq;

          if(trace_once) TRACE_FORMAT( "<%p> state %p, context %p, parameters %p, pwq %p, ndest %zu",this, state, context, parameters, pwq, state->dst_participants.size());
          size_t length = pwq? pwq->bytesAvailableToConsume() : 0;
          void* payload = pwq?(void*)pwq->bufferToConsume(): NULL;
          if(trace_once) TRACE_FORMAT( "<%p> length %zd/%zd, payload %p",this, length,(size_t)parameters->data.iov_len, payload);

          // send it now if there is enough data in the pwq
          if (length >= parameters->data.iov_len)
          {
            if(!trace_once) TRACE_FN_ENTER();
            if(!trace_once) TRACE_FORMAT( "<%p> state %p, context %p, parameters %p, pwq %p, ndest %zu",this, state, context, parameters, pwq, state->dst_participants.size());
            if(!trace_once) TRACE_FORMAT( "<%p> length %zd/%zd, payload %p",this, length,(size_t)parameters->data.iov_len, payload);
            parameters->data.iov_base = payload;
            parameters->data.iov_len = length;
            size_t size = state->dst_participants.size();
            for (unsigned i = 0; i < size; ++i)
            {
              pami_task_t task = state->dst_participants.index2Rank(i);

              if(task == __global.mapping.task()) /// \todo don't use global? remove myself from topo in caller?
                continue;

              result = PAMI_Endpoint_create ((pami_client_t) state->client, /// \todo client is ignored on the endpoint?  client isn't a pami_client_t
                                             task,
                                             state->contextid, /// \todo what context do I target?
                                             &parameters->dest);

              TRACE_FORMAT( "<%p> send(%u(%zu,%zu))", this, parameters->dest, (size_t) task, state->contextid);
              result = this->immediate (parameters);
              TRACE_FORMAT( "<%p> result %u", this, result);

            }
            TRACE_FN_EXIT(); trace_once = DO_TRACE_ENTEREXIT;
            return result;

          }
          // not enough data to send yet, post it to the context work queue for later processing
          if(trace_once) TRACE_FORMAT( "<%p> queue it on context %p",this,context);
          state->type = sendpwq_t::IMMEDIATE;
          state->pthis = this;

          /// \todo Pass in a generic/work device so we can directly post
          if(trace_once) TRACE_FN_EXIT(); trace_once = 0;
          PAMI_Context_post (context,(pami_work_t*)state, work_function, (void *) state);
          return result;

          // circular header dependencies if I try to use Context
//          PAMI::Context * ctx = (PAMI::Context *) context;
//          return ctx->post(state, work_function, (void *) state)
        }

        ///
        /// \brief Start a new simple send message with PWQ.  If there is no
        /// data ready, post an async work function to retry later.
        ///
        /// \param[in]  local_fn  Callback to invoke on local node when
        ///                       message local source data is completely sent.
        /// \param[in]  remote_fn Callback to invoke on local node when
        ///                       message is completely received on remote node.
        /// \param[in]  cookie    Opaque application callback data.
        /// \param[in]  task      Destination task.
        /// \param[in]  bytes     Send data length in bytes.
        /// \param[in]  src       Send data buffer.
        /// \param[in]  msginfo   Opaque application header information.
        /// \param[in]  mbytes    Number of msginfo bytes.
        ///
        /// \param[in]  pwq       A pipe work queue to use for the data buffer
        ///
        pami_result_t simplePWQ (sendpwq_t* state, pami_context_t context)
        {
          if(trace_once) TRACE_FN_ENTER();
          pami_result_t result = PAMI_EAGAIN;
          pami_send_t * parameters = &state->send.simple;

          PAMI::PipeWorkQueue * pwq = state->pwq;

          if(trace_once) TRACE_FORMAT( "<%p> state %p, context %p, parameters %p, pwq %p, ndest %zu",this, state, context, parameters, pwq, state->dst_participants.size());
          size_t length = pwq? pwq->bytesAvailableToConsume() : 0;
          void* payload = pwq?(void*)pwq->bufferToConsume(): NULL;
          if(trace_once) TRACE_FORMAT( "<%p> length %zd/%zd, payload %p",this, length,(size_t)parameters->send.data.iov_len, payload);

          // send it now if there is enough data in the pwq
          if (length >= parameters->send.data.iov_len)
          {
            if(!trace_once) TRACE_FN_ENTER();
            if(!trace_once) TRACE_FORMAT( "<%p> state %p, context %p, parameters %p, pwq %p, ndest %zu",this, state, context, parameters, pwq, state->dst_participants.size());
            if(!trace_once) TRACE_FORMAT( "<%p> length %zd/%zd, payload %p",this, length,(size_t)parameters->send.data.iov_len, payload);
            parameters->send.data.iov_base = payload;
            parameters->send.data.iov_len = length;
            size_t size = state->dst_participants.size();
            for (unsigned i = 0; i < size; ++i)
            {
              pami_task_t task = state->dst_participants.index2Rank(i);

              if(task == __global.mapping.task()) /// \todo don't use global? remove myself from topo in caller?
                continue;

              result = PAMI_Endpoint_create ((pami_client_t) state->client, /// \todo client is ignored on the endpoint?  client isn't a pami_client_t
                                             task,
                                             state->contextid, /// \todo what context do I target?
                                             &parameters->send.dest);

              TRACE_FORMAT( "<%p> send(%u(%zu,%zu))", this, parameters->send.dest, (size_t) task, state->contextid);
              result =  this->simple (parameters);
              TRACE_FORMAT( "<%p> result %u", this, result);

            }
             TRACE_FN_EXIT();trace_once = DO_TRACE_ENTEREXIT;
            return result;

          }
          // not enough data to send yet, post it to the context work queue for later processing
          if(trace_once) TRACE_FORMAT( "<%p> queue it on context %p",this, context);
          state->type = sendpwq_t::SIMPLE;
          state->pthis = this;

          /// \todo Pass in a generic/work device so we can directly post
          if(trace_once) TRACE_FN_EXIT(); trace_once = 0;
          PAMI_Context_post (context, (pami_work_t*)state, work_function, (void *) state);
          return result;
          // circular header dependencies if I try to use Context
//          PAMI::Context * ctx = (PAMI::Context *) context;
//          return ctx->post(state, work_function, (void *) state);
        }

      }; // PAMI::Protocol::SendPWQ class
    }; // PAMI::Protocol::Send namespace
  };   // PAMI::Protocol namespace
};     // PAMI namespace

#endif // __pami_p2p_protocols_SendPWQ_h__
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
