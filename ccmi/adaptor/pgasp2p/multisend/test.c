/* begin_generated_IBM_copyright_prolog                             */
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


#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "multisend.h"



#define MSG_SIZE   8000
#define MAX_ITER 32


// client done callback 
int done = 0;
void bcast_done (void *arg) {
  done++;
}

char sbuf[MSG_SIZE];

// async callback for receiver's side of multisend  
CCMI_Request_t * msend_recv(const CCMIQuad  * info,
			    unsigned          count,
			    unsigned          peer,
			    unsigned          sndlen,
			    unsigned          conn_id,
			    void            * arg,
			    unsigned        * rcvlen,
			    char           ** rcvbuf,
			    unsigned        * pipewidth,
			    CCMI_Callback_t * cb_done)
{
  assert ( sndlen == MSG_SIZE );
  * rcvlen          = sndlen;
  * rcvbuf          = sbuf;
  * pipewidth       = PIPEWIDTH;
  cb_done->function = bcast_done;

  return NULL;
}
 
 


int main (int argc, char **argv) 
{
  int ret = 0;

  MPI_Init (&argc, &argv);

  int rank, size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // initialize multisend
  CCMI_Register_t msend_registration;
  Multisend_multicast_register ( &msend_registration, msend_recv, NULL );
  
  unsigned *ranks = (unsigned *)malloc( size * sizeof(unsigned) );
  unsigned *hints = (unsigned *)malloc( size * sizeof(unsigned) );

  MPI_Barrier(MPI_COMM_WORLD);

  // request object for sending
  CCMI_Request_t req;

  unsigned i;
  for (i = 0; i < MAX_ITER; i ++) {

    done = 0;

    // select sender 
    unsigned root = i % size; 

    // put some meaningful data into the buffer
    unsigned j;
    for ( j = 0; j < MSG_SIZE; j++ )
      sbuf[ j ] = root == rank ? j % 256 : (unsigned)-1;

    if ( rank == root )
      {
	unsigned n = 0;
	for ( j = 0; j < size; j++ )
	  if ( j != rank ) 
	    { 
	      ranks[ n   ] = j; 
	      hints[ n++ ] = CCMI_PT_TO_PT_SUBTASK; 
	    }

	CCMI_Callback_t cb_done = { bcast_done, NULL };
	unsigned connId = i;
	// post the message
	MultiSend_multicast ( &msend_registration,
			      &req,
			      cb_done,
			      CCMI_MATCH_CONSISTENCY,
			      connId,
			      MSG_SIZE,
			      sbuf,
			      n,
			      ranks,
			      hints,
			      NULL );
      }
    
    // call advance untile message is arrived or sent
    while ( ! done  ) 
      Multisend_advance( &msend_registration );

    printf("iter:%d done:%d\n",i,done);

    for ( j = 0; j < MSG_SIZE; j++ )
      if ( sbuf[j] != j % 256 )
      {
	fprintf(stderr,"ERROR iter:%d sbuf[%d]:%d should be %d\n",i,j,sbuf[j],j%256);
	ret++;
	if ( ret > 5 )
	  {
	    fprintf(stderr,"TOO MANY ERRORS...\n");
	    break;
	  }
      }
  }
   
  MPI_Barrier (MPI_COMM_WORLD);
  MPI_Finalize ();

  if ( ! ret ) 
    printf("SUCCESS\n");
  else
    printf("FAILURE\n");
  
  return ret;
}

