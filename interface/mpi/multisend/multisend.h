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


#ifndef __generic_multisend_h__
#define __generic_multisend_h__

#include "collectives/interface/ccmi_internal.h"


typedef CCMIQuad CCMI_Register_t[32];

#ifdef __cplusplus
extern "C"
{
#endif


  
  
  typedef CCMI_Request_t * (*CCMI_RecvMultiSend) (const CCMIQuad    * info,
						  unsigned            count,
						  unsigned            peer,
						  unsigned            sndlen,
						  unsigned            conn_id,
						  void              * arg,
						  unsigned          * rcvlen,
						  char             ** rcvbuf,
						  unsigned          * pipewidth,
						  CCMI_Callback_t   * cb_done);
  

  int Multisend_multicast_register ( CCMI_Register_t    * registration, 
				     CCMI_RecvMultiSend   cb_recv, 
				     void               * arg ); 
  
  
  int MultiSend_multicast (CCMI_Register_t   * registration,
			   CCMI_Request_t    * request,
			   CCMI_Callback_t     cb_done,
			   CCMI_Consistency    consistency,
			   unsigned            connection_id,
			   unsigned            bytes,
			   const char        * src,
			   unsigned            nranks,
			   unsigned          * rank,
			   unsigned          * hints,
			   const CCMIQuad    * msginfo );


  int Multisend_advance ( CCMI_Register_t * registration );
  
#ifdef __cplusplus
};
#endif

#endif
