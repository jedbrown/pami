/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/* ---------------------------------------------------------------- */
/* IBM Confidential                                                 */
/*                                                                  */
/* OCO Source Materials                                             */
/*                                                                  */
/* Product(s):                                                      */
/*     5733-BG1                                                     */
/*                                                                  */
/* (C)Copyright IBM Corp.  2006, 2006                               */
/*                                                                  */
/* The Source code for this program is not published  or otherwise  */
/* divested of its trade secrets,  irrespective of what has been    */
/* deposited with the U.S. Copyright Office.                        */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */

#ifndef OMPI_SKIP_MPICXX
#define OMPI_SKIP_MPICXX
#endif

#include "mpi.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "interface/ccmi_internal.h"
#include "interface/mpi/multisend/multisend_impl.h"
#include "interface/mpi/multisend/multisend.h"

extern "C" int Multisend_multicast_register( CCMI_Register_t    * registration, 
					     CCMI_RecvMultiSend   cb_recv, 
					     void               * arg )
{
  assert( sizeof ( CCMI_Register_t ) >= sizeof( CCMI::Adaptor::Generic::OldMulticastImpl ));
  
  TRACE_ADAPTOR((stderr, "<%#.8X>Multisend_multicast_register\n", (int)registration));
  CCMI::Adaptor::Generic::OldMulticastImpl * msend =
    new ( registration ) CCMI::Adaptor::Generic::OldMulticastImpl();
  msend->setCallback( cb_recv, arg );
  return 0;
}

extern "C" int MultiSend_multicast(CCMI_Register_t   * registration,
				   XMI_Request_t    * request,
				   XMI_Callback_t     cb_done,
				   CCMI_Consistency    consistency,
				   unsigned            connection_id,
				   unsigned            bytes,
				   const char        * src,
				   unsigned            nranks,
				   unsigned          * rank,
				   unsigned          * hints,
				   const CMQuad      * msginfo )
{
  assert ( registration );

  TRACE_ADAPTOR((stderr, "<%#.8X>Multisend_multicast\n", (int)registration));
  CCMI::Adaptor::Generic::OldMulticastImpl * msend =
    ( CCMI::Adaptor::Generic::OldMulticastImpl * )registration;

  return msend->send(request,
		     &cb_done,
		     consistency,
		     msginfo,
		     1,
		     connection_id,
		     src,
		     bytes,
		     hints,
		     rank,
		     nranks);
}


extern "C" int Multisend_advance(CCMI_Register_t * registration)
{
  assert ( registration );
  
  TRACE_ADAPTOR((stderr, "<%#.8X>Multisend_advance\n", (int)registration));
  CCMI::Adaptor::Generic::OldMulticastImpl * msend =
    ( CCMI::Adaptor::Generic::OldMulticastImpl * )registration;

  msend->advance();
}
