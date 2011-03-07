/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/p2p/send/hint/use_shmem_perf.c
 * \brief ???
 */

#include "../send_perf.h"

int main (int argc, char ** argv)
{
  size_t header_count = argc;
  size_t header_size[header_count+1];
  header_size[0] = 0;

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
  dispatch[1].name = "only shmem";
  dispatch[1].options = (pami_dispatch_hint_t) {0};
  dispatch[1].options.use_shmem = PAMI_HINT_ENABLE;

  dispatch[2].id = 12;
  dispatch[2].name = " no shmem ";
  dispatch[2].options = (pami_dispatch_hint_t) {0};
  dispatch[2].options.use_shmem = PAMI_HINT_DISABLE;

  initialize (dispatch, 3);
  
  test (dispatch, 3, header_size, header_count);

  return 0;
}
