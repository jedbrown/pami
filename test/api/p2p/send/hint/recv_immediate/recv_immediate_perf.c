/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/p2p/send/hint/recv_immediate/recv_immediate_perf.c
 * \brief ???
 */

#include "../send_perf.h"

int main (int argc, char ** argv)
{
  size_t header_count = argc+1;
  size_t header_size[header_count+2];
  header_size[0] = 0;
  header_size[1] = 16; // mpich2 msginfo size

  int arg;

  for (arg = 1; arg < argc; arg++)
    {
      header_size[arg] = (size_t) strtol (argv[arg], NULL, 10);
    }

  dispatch_info_t dispatch[3];
  
  dispatch[0].id = 10;
  dispatch[0].name = "  default ";
  dispatch[0].options = (pami_dispatch_hint_t) {0};

  dispatch[1].id = 11;
  dispatch[1].name = " +rcvimmed";
  dispatch[1].options = (pami_dispatch_hint_t) {0};
  dispatch[1].options.recv_immediate = PAMI_HINT_ENABLE;

  dispatch[2].id = 12;
  dispatch[2].name = " -rcvimmed";
  dispatch[2].options = (pami_dispatch_hint_t) {0};
  dispatch[2].options.recv_immediate = PAMI_HINT_DISABLE;

  initialize (dispatch, 3);
  
  unsigned i;
  pami_configuration_t configuration;
  pami_result_t result;  
  for (i=0; i<3; i++)
  {
    configuration.name = PAMI_DISPATCH_RECV_IMMEDIATE_MAX;
    result = PAMI_Dispatch_query(dispatch[i].context, dispatch[i].id, &configuration, 1);
    if (result == PAMI_SUCCESS)
      {
        dispatch[i].sndlen_max = configuration.value.intval;
      }
    else
      {
        fprintf (stderr, "Warning. Unable to query PAMI_DISPATCH_RECV_IMMEDIATE_MAX. result = %d\n", result);
        exit (1);
      }    
  }
  
  test (dispatch, 3, header_size, header_count);

  return 0;
}
