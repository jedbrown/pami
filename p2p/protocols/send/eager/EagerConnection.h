/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/eager/EagerConnection.h
/// \brief Connection class for reliable, deterministic eager protocols
///
#ifndef __p2p_protocols_send_eager_EagerConnection_h__
#define __p2p_protocols_send_eager_EagerConnection_h__

#include <stdlib.h>

#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  namespace Protocol
  {
    namespace Send
    {
      template <class T_Device, unsigned T_Size = 100>
      class EagerConnection
      {
        private:
	struct eager_connection_t
          {
	    public:
            xmi_client_t     client;
            size_t     context;
            void           ** array;
          };

        public:
          EagerConnection (T_Device & device) :
            _device (device)
          {
          };

          ~EagerConnection () {}

          /// \todo Do this in a threadsafe way
          inline void ** getConnectionArray (xmi_client_t client, size_t context)
          {
            void ** connection = NULL;

            unsigned i;
            unsigned first_empty_connection = 0;
            bool found_empty_connection = false;
            bool found_previous_connection = false;
            for (i=0; i<T_Size; i++)
            {
              if (_eager_connection[i].array == NULL)
              {
                if (!found_empty_connection)
                {
                  first_empty_connection = i;
                  found_empty_connection = true;
                }
              }
              else if (_eager_connection[i].client == client &&
              		_eager_connection[i].context == context)
              {
                found_previous_connection = true;
                connection = _eager_connection[i].array;
              }
            }

            if (!found_previous_connection)
            {
              if (!found_empty_connection)
              {
                return NULL;
              }
              _eager_connection[first_empty_connection].client = client;
              _eager_connection[first_empty_connection].context = context;
              _eager_connection[first_empty_connection].array =
                (void **) calloc (_device.peers(), sizeof(void *));
              connection = _eager_connection[first_empty_connection].array;
            }

            return connection;
          }

        private:

          T_Device & _device;
          static eager_connection_t _eager_connection[T_Size];
      };

      template <class T_Device, unsigned T_Size>
      struct EagerConnection<T_Device,T_Size>::eager_connection_t EagerConnection<T_Device,T_Size>::_eager_connection[T_Size];
    };
  };
};



#undef TRACE_ERR
#endif // __xmi_p2p_protocol_send_eager_eagerconnection_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
