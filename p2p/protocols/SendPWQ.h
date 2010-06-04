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

#undef TRACE_ERR
#define TRACE_ERR(x) //fprintf x

namespace PAMI
{
  namespace Protocol
  {
    namespace Send
    {
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
            pami_send_immediate_t       *immediate;
            pami_send_t                 *simple;
          }                         send;
          PAMI::PipeWorkQueue      *pwq;
          SendPWQ                  *pthis;

        } sendpwq_t;

        ///
        /// \brief Async work function. Try to resend the data in the pwq if it's ready
        ///
        /// \param[in]  context   Work context        
        /// \param[in]  cookie    Original send state to resend.
        ///
        static pami_result_t work_function(pami_context_t context, void *cookie)
        {
          sendpwq_t * state = (sendpwq_t*)cookie;
          TRACE_ERR((stderr, "<%p>SendPWQ::work_function() context %p, cookie %p\n",state->pthis,context, cookie));

          if (state->type == sendpwq_t::IMMEDIATE)
          {
//            if(state->pwq->bytesAvailableToConsume() < state->send.immediate->data.iov_len)
//              return PAMI_EAGAIN; // this doesn't actually re-queue the work?
            state->pthis->immediatePWQ(state, context, state->send.immediate, state->pwq);
          }
          else
          {
//            if(state->pwq->bytesAvailableToConsume() < state->send.simple->send.data.iov_len)
//              return PAMI_EAGAIN; // this doesn't actually re-queue the work?
            state->pthis->simplePWQ(state, context, state->send.simple, state->pwq);
          }
          TRACE_ERR((stderr, "<%p>SendPWQ::work_function() exit context %p, cookie %p\n",state->pthis,context, cookie));
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
        pami_result_t immediatePWQ(sendpwq_t* state, pami_context_t context, pami_send_immediate_t * parameters, PAMI::PipeWorkQueue * pwq)
        {
          TRACE_ERR((stderr, "<%p>SendPWQ::immediate() state %p, context %p, parameters %p, pwq %p\n",this, state, context, parameters, pwq));
          size_t length = pwq? pwq->bytesAvailableToConsume() : 0;
          void* payload = pwq?(void*)pwq->bufferToConsume(): NULL;
          TRACE_ERR((stderr, "<%p>SendPWQ::immediate() length %zd, payload %p\n",this,length,payload));

          // send it now if there is enough data in the pwq
          if (length >= parameters->data.iov_len)
          {
            parameters->data.iov_base = payload;
            parameters->data.iov_len = length;

            return this->immediate (parameters);
          }
          // not enough data to send yet, post it to the context work queue for later processing
          TRACE_ERR((stderr, "<%p>SendPWQ::immediate() queue it on context %p\n",this,context));
          state->type = sendpwq_t::IMMEDIATE;
          state->pthis = this;
          state->send.immediate = parameters;
          state->pwq = pwq;

          /// \todo Pass in a generic/work device so we can directly post
          return PAMI_Context_post (context,(pami_work_t*)state, work_function, (void *) state);

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
        pami_result_t simplePWQ (sendpwq_t* state, pami_context_t context, pami_send_t * parameters, PAMI::PipeWorkQueue * pwq)
        {
          TRACE_ERR((stderr, "<%p>SendPWQ::simple() state %p, context %p, parameters %p, pwq %p\n",this, state, context, parameters, pwq));
          size_t length = pwq? pwq->bytesAvailableToConsume() : 0;
          void* payload = pwq?(void*)pwq->bufferToConsume(): NULL;
          TRACE_ERR((stderr, "<%p>SendPWQ::simple() length %zd, payload %p\n",this, length,payload));

          // send it now if there is enough data in the pwq
          if (length >= parameters->send.data.iov_len)
          {
            parameters->send.data.iov_base = payload;
            parameters->send.data.iov_len = length;

            return this->simple (parameters);
          }
          // not enough data to send yet, post it to the context work queue for later processing
          TRACE_ERR((stderr, "<%p>SendPWQ::simple() queue it on context %p\n",this, context));
          state->type = sendpwq_t::SIMPLE;
          state->pthis = this;
          state->send.simple = parameters;
          state->pwq = pwq;

          /// \todo Pass in a generic/work device so we can directly post
          return PAMI_Context_post (context, (pami_work_t*)state, work_function, (void *) state);

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
