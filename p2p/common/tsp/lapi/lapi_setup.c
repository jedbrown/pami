/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */


#include "../../include/pgasrt.h"
#include "pgasrt_lapi.h"
#if defined(__cplusplus)
extern "C" {
#endif


/* ************************************************************************ */
/*            global variables                                              */
/* ************************************************************************ */

lapi_handle_t __pgasrt_lapi_handle;
int           __pgasrt_lapi_mynode;
int           __pgasrt_lapi_nodes;

/* ************************************************************************ */
/*                          access functions                                */
/* ************************************************************************ */

#pragma weak __pgasrt_tsp_myID    = ___pgasrt_tsp_myID
#pragma weak __pgasrt_tsp_numnodes = ___pgasrt_tsp_numnodes
int ___pgasrt_tsp_myID     (void) { return __pgasrt_lapi_mynode; }
int ___pgasrt_tsp_numnodes (void) { return __pgasrt_lapi_nodes; }

typedef void (*SIGFUNC)(int);
void __pgasrt_sighandler (int);

/* ************************************************************************ */
/*                             setup LAPI                                   */
/* ************************************************************************ */

#pragma weak __pgasrt_tsp_setup=___pgasrt_tsp_setup
void *___pgasrt_tsp_setup (unsigned local_threads,
			   int                   * pargc,
			   char                *** pargv)
{
#ifndef NDEBUG
  signal(SIGSEGV, (SIGFUNC)__pgasrt_sighandler);
  signal(SIGBUS,  (SIGFUNC)__pgasrt_sighandler);
  signal(SIGTRAP, (SIGFUNC)__pgasrt_sighandler);
#endif

  lapi_info_t   * lapi_info;     /* used as argument to LAPI_Init */
  lapi_extend_t * extend_info;   /* holds IP addresses and ports */
  lapi_udp_t    * udp_info;      /* List of ip, port info to pass to LAPI */
  int             num_tasks;     /* number of tasks (from LAPI_LIST_NAME) */
  char          * list_name;     /* name of UDP host/port list file */
  FILE          * fp;
  int             i;

  /* ------------------------------------------------------------ */
  /*        initialize request allocation pool                    */
  /* ------------------------------------------------------------ */
  __pgasrt_lapi_pool_init();

  /* ------------------------------------------------------------ */
  /*       allocate and initialize lapi_info                      */
  /* ------------------------------------------------------------ */

  CHECK_NULL(lapi_info,(lapi_info_t *)malloc(sizeof(lapi_info_t)));
  memset(lapi_info, 0, sizeof(lapi_info_t));

  CHECK_NULL(extend_info,(lapi_extend_t *)malloc(sizeof(lapi_extend_t)));
  memset(extend_info, 0, sizeof(lapi_extend_t));

  /* ------------------------------------------------------------ */
  /* collect UDP hostnames and ports into udp_info data structure */
  /* ------------------------------------------------------------ */

  udp_info = NULL;
  list_name=getenv("LAPI_LIST_NAME");
  if (list_name)
    {
      if ((fp = fopen (list_name, "r")) == NULL) { 
	printf ("Cannot find LAPI_LIST_NAME\n");
	abort();
      }
      fscanf(fp, "%u", &num_tasks);
      CHECK_NULL(udp_info,(lapi_udp_t *) malloc(num_tasks*sizeof(lapi_udp_t)));
      for (i = 0; i < num_tasks; i++) 
	{
	  char ip[256];
	  unsigned port;
	  fscanf(fp, "%s %u", ip, &port);
	  udp_info[i].ip_addr = inet_addr(ip);
	  udp_info[i].port_no = port;
	}
    }

  /* ------------------------------------------------------------ */
  /*        link up udp_info, extend_info and lapi_info           */
  /* ------------------------------------------------------------ */

  extend_info->add_udp_addrs = udp_info;
  extend_info->num_udp_addr  = num_tasks;
  extend_info->udp_hndlr     = 0;
  lapi_info->add_info        = extend_info;

  /* ------------------------------------------------------------ */
  /*                call LAPI_Init                                */
  /* ------------------------------------------------------------ */
  int intval = getenv("PGASRT_INTERRUPT")?atoi(getenv("PGASRT_INTERRUPT")):0;
  
  CALL_AND_CHECK_RC((LAPI_Init(&__pgasrt_lapi_handle, lapi_info)));
  CALL_AND_CHECK_RC((LAPI_Senv(__pgasrt_lapi_handle,INTERRUPT_SET, intval)));
  CALL_AND_CHECK_RC((LAPI_Qenv(__pgasrt_lapi_handle,TASK_ID,
			       (int *)&__pgasrt_lapi_mynode)));
  CALL_AND_CHECK_RC((LAPI_Qenv(__pgasrt_lapi_handle,NUM_TASKS,
			       (int *)&__pgasrt_lapi_nodes)));
  
  /* ------------------------------------------------------------ */
  /*              create handles for callbacks                    */
  /* ------------------------------------------------------------ */
  
  CALL_AND_CHECK_RC((LAPI_Addr_set (__pgasrt_lapi_handle,
				    (void *)__pgasrt_lapi_amSendRequestHandler,
				    PGASRT_LAPI_AMSENDREQUESTHANDLER)));
#if 0
  CALL_AND_CHECK_RC((LAPI_Addr_set (__pgasrt_lapi_handle, 
				    (void *)__pgasrt_lapi_getRequestHandler,
				    PGASRT_LAPI_GETREQUESTHANDLER)));
  CALL_AND_CHECK_RC((LAPI_Addr_set (__pgasrt_lapi_handle,
				    (void *)__pgasrt_lapi_getReplyHandler,
				    PGASRT_LAPI_GETREPLYHANDLER)));
  CALL_AND_CHECK_RC((LAPI_Addr_set (__pgasrt_lapi_handle,
				    (void *)__pgasrt_lapi_getvRequestHandler,
				    PGASRT_LAPI_GETVREQUESTHANDLER)));
  CALL_AND_CHECK_RC((LAPI_Addr_set (__pgasrt_lapi_handle,
				    (void *)__pgasrt_lapi_getvReplyHandler,
				    PGASRT_LAPI_GETVREPLYHANDLER)));
  CALL_AND_CHECK_RC((LAPI_Addr_set (__pgasrt_lapi_handle,
				    (void *)__pgasrt_lapi_putRequestHandler,
				    PGASRT_LAPI_PUTREQUESTHANDLER)));
  CALL_AND_CHECK_RC((LAPI_Addr_set (__pgasrt_lapi_handle,
				    (void *)__pgasrt_lapi_putReplyHandler,
				    PGASRT_LAPI_PUTREPLYHANDLER)));
  CALL_AND_CHECK_RC((LAPI_Addr_set (__pgasrt_lapi_handle,
				    (void *)__pgasrt_lapi_putvRequestHandler,
				    PGASRT_LAPI_PUTVREQUESTHANDLER)));
  CALL_AND_CHECK_RC((LAPI_Addr_set (__pgasrt_lapi_handle,
				    (void *)__pgasrt_lapi_putvReplyHandler,
				    PGASRT_LAPI_PUTVREPLYHANDLER)));
  CALL_AND_CHECK_RC((LAPI_Addr_set (__pgasrt_lapi_handle,
				    (void *)__pgasrt_lapi_accRequestHandler,
				    PGASRT_LAPI_ACCREQUESTHANDLER)));
#endif
  /* ------------------------------------------------------------ */
  /*               barrier                                        */
  /* ------------------------------------------------------------ */

  LAPI_Gfence (__pgasrt_lapi_handle);
  return NULL;
}

/* ************************************************************************ */
/*                   terminate LAPI                                         */
/* ************************************************************************ */

#pragma weak __pgasrt_tsp_finish=___pgasrt_tsp_finish
void * ___pgasrt_tsp_finish ()
{
  LAPI_Gfence (__pgasrt_lapi_handle);
  // CALL_AND_CHECK_RC((LAPI_Term(&__pgasrt_lapi_handle)));
  return NULL;
}

/* ************************************************************************ */
/*                   terminate LAPI *immediately*                           */
/* ************************************************************************ */

#pragma weak __pgasrt_tsp_abort=___pgasrt_tsp_abort
void ___pgasrt_tsp_abort (int exitcode)
{
  fprintf(stderr, "%d: Aborting w/ exit code=%d\n", PGASRT_MYNODE, exitcode);
  if (exitcode < 128)
    fprintf(stderr, "WARNING: POE jobs may hang instead of exiting"
	    " when exit code < 128\n");
  exit (exitcode);
}

/* ************************************************************************ */
/*                  wait for a LAPI counter                                 */
/* ************************************************************************ */
#pragma weak __pgasrt_tsp_wait=___pgasrt_tsp_wait
void * ___pgasrt_tsp_wait(void * r)
{
  int val;
  lapi_msg_info_t info;
  __pgasrt_lapi_request_t * req = (__pgasrt_lapi_request_t *)r;
  if (__pgasrt_lapi_nodes<=1)
    {
      return NULL;
    }
  else if (!req) 
    { 
      LAPI_Msgpoll (__pgasrt_lapi_handle, 5, &info);
      return NULL;
    }
  else
    {
      while (!req->cmplt)
	LAPI_Msgpoll (__pgasrt_lapi_handle, 10, &info);
      __pgasrt_lapi_pool_free(req);
      return NULL;
    }
}

void * __pgasrt_tsp_delete (void *r)
{
  __pgasrt_lapi_request_t * req = (__pgasrt_lapi_request_t *)r;
  if (!req) return NULL;
  assert (req->cmplt != 0);
  __pgasrt_lapi_pool_free (req);
}

#if defined(__xlC__)
int bubucika() { abort (); return 0; }
#pragma weak __IBMCPlusPlusExceptionV2 = bubucika
#pragma weak __dl__FPv = bubucika
#endif

#if defined(__cplusplus)
}
#endif
