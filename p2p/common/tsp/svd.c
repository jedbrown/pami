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


/* $Id: svd.c,v 1.32 2009-04-08 18:00:43 almasig Exp $ */
/**
 * \addtogroup uSVD
 * Implementation of SVD functions.
 * The Shared Variable Directory (SVD) is the mechanism for accessing shared
 * variables. It associates a {\em shared address} with a handle of type
 * __xlupc_shared_var_handle_t.
 * \sa __xlupc_shared_var_handle_t
 * @{
 */
#include <assert.h>
#include <stdio.h>
#include "pgasrt.h"
#include "xlupc_types.h"

//#define DEBUG_SVD 1
#ifdef DEBUG_SVD
#define TRACE(x)  fprintf x
//#define PGASRT_MYTHREAD 0
#else
#define TRACE(x)
#endif

/** the global table. Each thread has its own copy in private space */
__pgasrt_svd_table_t __pgasrt_svd_table = NULL;
unsigned             __pgasrt_svd_numparts = 0;
DECL_RW_MUTEX(__pgasrt_SVD_mutex);

/**
 * initializes the SVD. 
 * It allocates space for the global table and its partitions. Initializes the
 * partition information for all the threads. It gets called only once per
 * node, before threads get created.
 * Return PGASRT_OK if OK, error code otherwise.
 * \sa PGASRT_OK
 */

int __pgasrt_svd_init(unsigned numparts)
{
  unsigned i;
  const int PGASRT_SVD_DIR_SIZE = 2; /* initial size of directory */
  __pgasrt_svd_numparts = numparts;

  TRACE((stderr, "%d: %s\n", PGASRT_MYTHREAD, __FUNCTION__));
  /* allocate partitions in the SVD for all threads + ALL */
  __pgasrt_svd_table = 
    (__pgasrt_svd_table_t)malloc(sizeof(__pgasrt_svd_partition) * numparts);
  if(!__pgasrt_svd_table) return PGASRT_ERR_NO_PRIVATE_MEM;

  /* allocate each partition */
  for(i = 0; i < numparts; i++) {
    __pgasrt_svd_table[i]._directory = (__pgasrt_shared_var_ctrl_block *)
      malloc(sizeof(__pgasrt_shared_var_ctrl_block) * PGASRT_SVD_DIR_SIZE);
    if(!__pgasrt_svd_table[i]._directory) return PGASRT_ERR_NO_PRIVATE_MEM;
    __pgasrt_svd_table[i]._dirSize = PGASRT_SVD_DIR_SIZE;
    __pgasrt_svd_table[i]._nextFreeIndex = 1;
    TRACE((stderr, "%d: %s Allocate each partition %d dirSize %d nextFreeIndex %d\n", PGASRT_MYTHREAD, __FUNCTION__, i, __pgasrt_svd_table[i]._dirSize,__pgasrt_svd_table[i]._nextFreeIndex));
  }

  return PGASRT_OK;
}

/* ************************************************************************ */
/***                    free the table                                      */
/* ************************************************************************ */

void __pgasrt_svd_free()
{
  unsigned i;
  for(i = 0; i < __pgasrt_svd_numparts; i++) {
    unsigned j;
    /* free the "shared" portion of the entry */
    for(j = 0; j < __pgasrt_svd_table[i]._nextFreeIndex; j++) {
      //__pgasrt_shared_var_handle_t entry = { i, j };
      __pgasrt_svd_handle_t entry;
      entry._partition =i;
      entry._index = j;
      __pgasrt_svd_free_entry(entry);
    }
    /* free the "local" portion of the entry */
    free(__pgasrt_svd_table[i]._directory);
  }

  /* free the svd */
  free(__pgasrt_svd_table);
}

/* ************************************************************************ */
/** return the next free index in the specified partition                   */
/* \internal \todo need to return an error code somehow !!!                 */
/* \internal \todo how do we lock access to the ALL_PARTITION               */
/* ************************************************************************ */

unsigned short __pgasrt_svd_get_next_index (unsigned int partition)
{
  unsigned short index;
  __pgasrt_shared_var_ctrl_block *dir; /* temp pointer in case realloc fails*/
  assert(partition < __pgasrt_svd_numparts);
//   assert(((void)"Threads may change their own partition only!",
//           (partition == THREADS || PGASRT_MYTHREAD == partition)));

 TRACE((stderr,
	"%d: %s(part = %d, nextFreeIndex = %d)\n", PGASRT_MYTHREAD, __FUNCTION__, 
	partition, __pgasrt_svd_table[partition]._nextFreeIndex));
  
  if (partition == __pgasrt_svd_numparts-1) WRLOCK(&__pgasrt_SVD_mutex);

  __pgasrt_svd_handle_t max = {0, -1};
  if (__pgasrt_svd_table[partition]._nextFreeIndex == max._index)
    __pgasrt_fatalerror (-1, 
			"Too many (>= %d) shared variables "
			"in partition %d. Try allocating fewer "
			"shared variables.\n", max._index, partition);

  if(__pgasrt_svd_table[partition]._nextFreeIndex + 1 < 
     __pgasrt_svd_table[partition]._dirSize) {
    index = __pgasrt_svd_table[partition]._nextFreeIndex++;
    if (partition == __pgasrt_svd_numparts-1) RWUNLOCK(&__pgasrt_SVD_mutex);
    return index;
  }

  /* not enough space in current directory. resize the partition */
  dir = (__pgasrt_shared_var_ctrl_block *)
    realloc(__pgasrt_svd_table[partition]._directory,
	    sizeof(__pgasrt_shared_var_ctrl_block) * 
	    2*__pgasrt_svd_table[partition]._dirSize);
  assert(dir != NULL); /* TODO: ... how do we return an error code? */ 

  /* success */
  __pgasrt_svd_table[partition]._directory = dir;
  __pgasrt_svd_table[partition]._dirSize *= 2;
  index = __pgasrt_svd_table[partition]._nextFreeIndex++;
  if (partition == __pgasrt_svd_numparts-1) RWUNLOCK(&__pgasrt_SVD_mutex);

  return index; 
}

/* ************************************************************************ */
/**
 * initialize the directory entry specified by the handle. It assumes that the
 * entry was allocated.
 * \param entry    the SVD handle that is to be intialized
 * \param var_type the type of the shared variable 
 * \param shr_ptr  the actual address in shared memory of the variable
 */
/* ************************************************************************ */

void __pgasrt_svd_set_entry (__pgasrt_svd_handle_t        entry,
			    __pgasrt_shared_var_types    var_type,
			    void                       * shr_ptr)
{
  assert(entry._partition < __pgasrt_svd_numparts);
  assert(entry._index > 0 &&
	 entry._index < __pgasrt_svd_table[entry._partition]._nextFreeIndex);
  TRACE((stderr,"%d: %s([%d:%d], var_type = %d)\n",
         PGASRT_MYTHREAD, __FUNCTION__,
	 entry._partition, entry._index, var_type));
  
  if (entry._partition==__pgasrt_svd_numparts-1) WRLOCK(&__pgasrt_SVD_mutex);
  __pgasrt_svd_table[entry._partition]._directory[entry._index]._type = var_type;
  __pgasrt_svd_table[entry._partition]._directory[entry._index].u._scalar = 
    (__pgasrt_local_addr_t)shr_ptr;
  if (entry._partition==__pgasrt_svd_numparts-1) RWUNLOCK(&__pgasrt_SVD_mutex);
}

/* ************************************************************************ */
/**
 * returns the directory entry specified by a handle
 * \param entry  the entry's handle
 */
/* ************************************************************************ */

__pgasrt_shared_var_ctrl_block *
__pgasrt_svd_get_entry(__pgasrt_svd_handle_t entry)
{
  __pgasrt_shared_var_ctrl_block *var;

  TRACE((stderr,"xlc_upc_rts_get_svd_entry\n"));
  assert(entry._partition < __pgasrt_svd_numparts);
  TRACE((stderr,"entry._index %d entry._partition %d and dirSize %d and nextFreeIndex %d\n",entry._index, entry._partition, __pgasrt_svd_table[entry._partition]._dirSize, entry._index < __pgasrt_svd_table[entry._partition]._nextFreeIndex));
  //assert(entry._index < __pgasrt_svd_table[entry._partition]._nextFreeIndex);
  assert (entry._index > 0);
  assert (entry._index < __pgasrt_svd_table[entry._partition]._dirSize);

  /* use retry loop to synchronize using and extending a partition */
  /* generated code is better if retry loop is used for all partitions */
  {
    volatile __pgasrt_svd_partition * partition_ptr;
    __pgasrt_shared_var_ctrl_block *volatile base, *volatile base_prev;

    partition_ptr = __pgasrt_svd_table + entry._partition;
    base = partition_ptr->_directory;
    do {
#pragma execution_frequency (very_low)
      var = base + entry._index;
      base_prev = base;
      base = partition_ptr->_directory;
    } while (base != base_prev);
  }

  return var;
}

/* ************************************************************************ */
/**
 * free a directory entry. This routine deallocate the space allocated
 * by upc_init_handle.
 * \param entry the entry's handle
 * \sa upc_init_handle
 */
/* ************************************************************************ */

void __pgasrt_svd_free_entry(__pgasrt_svd_handle_t entry)
{
  __pgasrt_shared_var_ctrl_block *theEntry;

  if (entry._partition==__pgasrt_svd_numparts-1) WRLOCK(&__pgasrt_SVD_mutex);
#ifdef DEBUG_SVD
  fprintf(stderr, "Thread %d freeing svd entry ", PGASRT_MYTHREAD);
  __pgasrt_svd_print_entry(entry._partition, entry._index);
#endif

  /* invalid entry? We don't assert because this may be an uninintialized
     pointer with shared target. Just return. */
  if(entry._partition >= __pgasrt_svd_numparts || 
     entry._index == 0 || 
     entry._index > __pgasrt_svd_table[entry._partition]._nextFreeIndex) {
    if (entry._partition==__pgasrt_svd_numparts-1)
      RWUNLOCK(&__pgasrt_SVD_mutex);
    return;
  }

  theEntry = &__pgasrt_svd_table[entry._partition]._directory[entry._index];
  switch(theEntry->_type) {

  case UPC_SHR_SCALAR: 
    if(theEntry->u._scalar) free (theEntry->u._scalar);
    break;

  case UPC_SHR_STRUCT:
    if(theEntry->u._struct) free (theEntry->u._struct);
    break;

  case UPC_SHR_ARRAY:
    if(theEntry->u._array) free (theEntry->u._array);
    break;
    
  case UPC_SHR_BLOCKED_ARRAY:
    if(theEntry->u._array) free (theEntry->u._mbarray);
    break;
    
  case UPC_SHR_TARGET:
    if(theEntry->u._shr_tgt) free(theEntry->u._shr_tgt);
    theEntry->u._shr_tgt = NULL;
    break;

  case UPC_PVT_TARGET:
    if(theEntry->u._pvt_tgt) free (theEntry->u._pvt_tgt);
    break;

  case UPC_SHR_LOCK:
    if(theEntry->u._shr_lock) free (theEntry->u._shr_lock);
    theEntry->u._shr_lock = NULL;
    break;

  case UPC_SHR_UNINITIALIZED:
    TRACE((stderr, "%d: freeing [%d,%d] twice\n", PGASRT_MYTHREAD, entry._partition, entry._index));
    break;

  default: break;
  }
  theEntry->_type = UPC_SHR_UNINITIALIZED;
  if (entry._partition==__pgasrt_svd_numparts-1) RWUNLOCK(&__pgasrt_SVD_mutex);
}

/* ********************************************************************* */
/*        figure out the local address of an SVD entry                   */
/* ********************************************************************* */
__pgasrt_local_addr_t
__pgasrt_svd_addrof (__pgasrt_svd_handle_t handle)
{
  TRACE((stderr, "%d: getting address for [%d,%d]\n", 
	 PGASRT_MYTHREAD, 
	 handle._partition,
	 handle._index));

  __pgasrt_local_addr_t local_addr = NULL;
  __pgasrt_shared_var_ctrl_block *e = __pgasrt_svd_get_entry(handle);

  switch(e->_type)
    {
    case UPC_SHR_SCALAR:        local_addr = e->u._scalar;               break;
    case UPC_SHR_STRUCT:        local_addr = e->u._struct;               break;
    case UPC_SHR_ARRAY:         local_addr = e->u._array->_local_addr;   break;
    case UPC_SHR_BLOCKED_ARRAY: local_addr = e->u._mbarray->_local_addr; break;
    case UPC_PVT_TARGET:        local_addr = e->u._pvt_tgt;              break;
    case UPC_SHR_TARGET:        local_addr = (__pgasrt_local_addr_t)
				  e->u._shr_tgt;                         break;
    default:
      __pgasrt_fatalerror (-1, 
			   "%d: svd_addrof: uninitialized SVD handle [%d,%d]",
			   PGASRT_MYNODE,
			   handle._partition,
			   handle._index);
      assert(((void)"Uninitialized shared variable", 0));
      break;
    }
  // assert (local_addr); // ? not obvious that this is needed ?
  return(local_addr);
}

/** 
 * print the SVD entry defined by [part:index]
 * \param part the SVD partition
 * \param index the index in the partition
 */
void __pgasrt_svd_print_entry(unsigned int part, unsigned int index)
{
  char *var_type = NULL;

  assert(part < __pgasrt_svd_numparts);
  if (index > 0 && index < __pgasrt_svd_table[part]._nextFreeIndex)
    {
      switch(__pgasrt_svd_table[part]._directory[index]._type)
	{
	case UPC_SHR_SCALAR: var_type = "SHR_SCALAR"; break;
	case UPC_SHR_STRUCT: var_type = "SHR_STRUCT"; break;
	case UPC_SHR_ARRAY:  var_type = "SHR_ARRAY"; break;
	case UPC_SHR_TARGET: var_type = "SS_PTR"; break;
	case UPC_PVT_TARGET: var_type = "SP_PTR"; break;
	case UPC_SHR_BLOCKED_ARRAY: var_type = "SHR_BLK_ARRY"; break;
	default: break;
	}
    }
  else
    var_type = "UNKNOWN";

  fprintf(stderr, "Node %d: [%d:%d]{ %s }\n", PGASRT_MYNODE, part, index, var_type);
}

/** print the whole svd table */
void __pgasrt_svd_print()
{
  unsigned i, j;

  for(i = 0; i < __pgasrt_svd_numparts; i++) {
    fprintf(stderr, "{ part %d: entries=%d, size=%d\n",
	    i, __pgasrt_svd_table[i]._nextFreeIndex, __pgasrt_svd_table[i]._dirSize);
    for(j = 0; j < __pgasrt_svd_table[i]._nextFreeIndex; j++) {
      fprintf(stderr, "\t"); 
      __pgasrt_svd_print_entry(i, j); 
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "}\n");
  }
}

/** get the whole svd table */
__pgasrt_svd_table_t __pgasrt_get_g_svd_table() {
  return __pgasrt_svd_table;
}

/** @} *//* end group uSVD */

