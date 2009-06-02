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

#include "pgasrt.h"

#include <stdio.h>
#include <string.h>

#define DEBUG_BIND 1
#ifdef DEBUG_BIND
#define TRACE(x)  fprintf x
#else
#define TRACE(x)
#endif


/* ************************************************************************ */
/*            bind my ID to corresponding entry in map file                 */
/* ************************************************************************ */

static int cpubind (int);
#pragma weak __pgasrt_cpubind=___pgasrt_cpubind
void ___pgasrt_cpubind (int myID)
{
  char buffer[80];
  char * mapfile = getenv ("PGASRT_CPUMAP");  /* getting map file           */
  if (!mapfile) return;                       /* no map file:    ignore     */
  FILE * fp = fopen (mapfile, "r");           /* opening map file           */
  if (!fp)
    __pgasrt_fatalerror(-1, "%d: Cannot open mapfile %s", myID, mapfile);
  fgets (buffer, sizeof(buffer)-1, fp);
  int i; for (i=0; i<myID; i++) 
    {
      fgets (buffer, sizeof(buffer)-1, fp);
      if (strlen(buffer) == 0) return;        /* not enough entries: ignore */
    }
  fclose (fp);
  int cpuID=atoi(buffer);
  TRACE((stderr, "Binding ID=%d to local CPU=%d on node=%d\n", 
	 myID, cpuID, PGASRT_MYNODE));
  cpubind (cpuID);
}

/* ********************************************************************* */
/*                 processor binding on AIX                              */
/* ********************************************************************* */

#if defined(_AIX)

#include <sys/processor.h>
#include <sys/thread.h>

int cpubind (int cpuID)
{
  int noProcessorConf   = sysconf(_SC_NPROCESSORS_CONF);
  int noProcessorOnline = sysconf(_SC_NPROCESSORS_ONLN);
  
  if (noProcessorConf == -1 || noProcessorOnline == -1)
    __pgasrt_fatalerror (-1, "Cpubind: sysconf: no processors configured");
  
  if (cpuID > noProcessorConf)
    __pgasrt_fatalerror (-1, "Cpubind: requested cpu=%d out of range=[0:%d]",
			cpuID, noProcessorConf);
  
  if (bindprocessor(BINDTHREAD, thread_self(), cpuID) < 0)
    __pgasrt_fatalerror (-1, "Cpubind: error in bindprocessor: %s",
			 strerror(errno));
  return 0;
}

#else /* !__linux && !_AIX */

/* ********************************************************************* */
/*          processor binding on an unsupported architecture             */
/* ********************************************************************* */

int cpubind (int cpuId)
{
  return -1; /* not binding anything */
}

#endif












