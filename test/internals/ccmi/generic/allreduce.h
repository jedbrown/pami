/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/ccmi/generic/allreduce.h
 * \brief ccmi allreduce common routines
 */

#ifndef __test_internals_ccmi_generic_allreduce_h__
#define __test_internals_ccmi_generic_allreduce_h__

#include "common.h"

PAMI_CollectiveProtocol_t              allreduce_reg;
CCMI_Allreduce_Configuration_t         allreduce_conf;
PAMI_CollectiveRequest_t               allreduce_request;

inline void allreduce_advance (unsigned * srcbuf, unsigned * dstbuf, unsigned src_count)
{
  TRACE_TEST_VERBOSE((stderr, "%s:%s   src %p, dst %p, count %d\n", argv0,__PRETTY_FUNCTION__, srcbuf, dstbuf, src_count));
  done = 0;
  CCMI_Allreduce(&allreduce_reg,
                 &allreduce_request,
                 common_done,
                 consistency,
                 &geometry,
                 (char *)srcbuf,
                 (char *)dstbuf,
                 src_count,
                 PAMI_UNSIGNED_INT,
                 PAMI_SUM);
  while(!done)
  {
    CCMI_Generic_adaptor_advance();
  }
  return;
}


void initialize(CCMI_Barrier_Protocol barrier_protocol,
                CCMI_Barrier_Protocol lbarrier_protocol,
                CCMI_Allreduce_Protocol allreduce_protocol)
{

  initialize_common(barrier_protocol, lbarrier_protocol);

  common_done.function  = done_callback;
  common_done.clientdata= NULL;

  allreduce_conf.protocol = allreduce_protocol;
  allreduce_conf.cb_geometry = getGeometry;

  allreduce_conf.reuse_storage = 1;
  char* envopts = getenv("CCMI_ALLREDUCE_REUSE_STORAGE");
  if(envopts != NULL)
  {
    if(strncasecmp(envopts, "N", 1) == 0) /* Do not reuse the malloc'd storage */
    {
      allreduce_conf.reuse_storage = 0;
    }
    else
      if(strncasecmp(envopts, "Y", 1) == 0); /* defaults to Y */
    else
      if(rank == 0) fprintf(stderr,"Invalid CCMI_ALLREDUCE_REUSE_STORAGE option\n");
  }

  CCMI_Result ccmiResult;

  if((ccmiResult = (CCMI_Result) CCMI_Allreduce_register (&allreduce_reg, &allreduce_conf)) != PAMI_SUCCESS)
    if(rank == 0) fprintf(stderr,"CCMI_Allreduce_register failed %d\n",ccmiResult);

  if(!CCMI_Geometry_analyze(&geometry, &allreduce_reg))
  {
    no_op("Not a supported geometry - NO OP\n");
    exit(0);
  }


  return;
}

#endif
