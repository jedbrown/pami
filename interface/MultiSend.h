/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file protocols/prod/multisend/MultiSend.h
 * \brief ???
 */

#ifndef   __new_multisend_h__
#define   __new_multisend_h__

//#include "ll.h"

#include "ll_multisend.h"
#include "./Topology.h"
#include "./PipeWorkQueue.h"
#include "../include/lapi.h"
#include <map>
#include <vector>

using namespace std;

namespace LL
{
    namespace MultiSend
    {
        class MultisendProtocolFactory
	{
        protected:
            inline void setRoles(int numRoles, int replRole)
		{
		    _numRoles = numRoles;
		    _replRole = replRole;
		}
        public:
            MultisendProtocolFactory(lapi_handle_t ctxt, LL_Result &status):
                _numRoles(0),
                _replRole(-1),
		_ctxt(ctxt)
                    {

                    }

                LL_Result getRoles(int *numRoles, int *replRole)
		{
                    *numRoles = _numRoles;
                    *replRole = _replRole;
                    return LL_SUCCESS; // or should be void?
                }
		static int incrGetHndlr()
		    {
			static int currHndlr=0;
			currHndlr++;
			return currHndlr;
		    }
        protected:
                int           _numRoles;
                int           _replRole;
		lapi_handle_t _ctxt;
		int           _currHndlr;

        }; //-- MultisendProtocolFactory

        ///
        /// \brief Callback function for unexpected async mcasts
        ///
        ///
        /// \brief Callback function for unexpected async many to many
        /// operations
        ///
        /// \brief The NEW multicast interface
	class MulticastFactory : public MultisendProtocolFactory {
        protected:
        public:
            MulticastFactory(lapi_handle_t ctxt,LL_Result &status):
		MultisendProtocolFactory(ctxt,status)
		{}
	    LL_Result generate(LL_Multicast_t *mcast_info){}
	    map<int, vector<void*>* > _conn_map;
        };  //-- MulticombineFactory

        class MultisyncFactory : public MultisendProtocolFactory
	{
        public:
            MultisyncFactory(lapi_handle_t ctxt,LL_Result &status):
		MultisendProtocolFactory(ctxt,status){}
	    LL_Result generate(LL_Multisync_t *msync_info){}
	    //virtual unsigned getId() = 0;
	    map<int, vector<void*>* > _conn_map;
	    map<int, vector<void*>* > _early_conn_map;
	private:
        };

        class MulticombineFactory : public MultisendProtocolFactory {
        protected:
        public:
            MulticombineFactory(lapi_handle_t ctxt,LL_Result &status) :
                MultisendProtocolFactory(ctxt,status)
                {
                    //status = LL_SUCCESS;
                }
                LL_Result generate(LL_Multicombine_t *mcomb_info){}
        };  //-- MulticombineFactory


        class ManyToManyFactory  : public MultisendProtocolFactory {
        protected:

            ///
            /// \brief the number of incoming connections into this
            /// broadcast.  For example a 3 color synchronous broadcast
            /// will have 3 connections indentified by the color.
            ///

            unsigned numConnections;

            LL_RecvManytomany    _cb_async_manytomany;
            void             * _m2m_arg;

        public:

            /// \brief Constructor

            inline ManyToManyFactory (lapi_handle_t ctxt,
				      unsigned      nconn,
                                      LL_Result    &status):
		MultisendProtocolFactory(ctxt,status),
                numConnections (nconn) {}

                ///
                /// \brief Create a many-to-many message
                /// \param buf  : start of the buffer to scatter
                /// \param cb_done : Callback when the many-to-many is sent
                /// \param info : A 16 byte field that will
                ///               with the first packet. Info can be NULL if
                ///               a Recv has guaranteed to be posted.
                /// \param conn_id : The connection id for this message
                /// \param size : size of the many-to-many buffer
                /// \param ranks   : Ranks of the destinations
                /// \param nranks  : Number of destinations
                /// \param hints   : deposit bit bcast vs pt-to-pt
                ///


                /// Set the async head callback
                inline void setManyToManyCallback (LL_RecvManytomany cb_recv, void *arg) {
                    _cb_async_manytomany =  cb_recv;
                    _m2m_arg     =  arg;
                }

                LL_Result generate  (LL_Manytomany_t *m2m_info){}

                ///
                /// \brief Post a many-to-many receive
                /// \param buf  : start of the buffer to scatter
                /// \param cb_done : Callback when the many-to-many is received
                /// \param conn_id : The connection id for this message
                /// \param size : size of the many-to-many buffer
                /// \param pwidth : Pipeline width for the mcast buffer.
                ///                 After pwidth data is received the
                ///                 cb_done handler will be called.
                ///

                unsigned generateRecv (LL_Request_t         * request,
				       const LL_Callback_t  * cb_done,
				       unsigned                 conn_id,
				       char                   * buf,
				       unsigned               * sizes,
				       unsigned               * offsets,
				       unsigned               * counters,
				       unsigned                 nranks,
				       unsigned                 rankIndex);

        };  //-- ManyToManyFactory
    };  //-- Multisend namespace
};  //-- LL

#endif
