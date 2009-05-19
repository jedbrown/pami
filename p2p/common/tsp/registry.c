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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/pgasrt.h"

#if defined(__cplusplus)
extern "C" {
#endif


//#define DEBUG_REGISTRY
#ifdef DEBUG_REGISTRY
#define TRACE(x) fprintf x 
#else
#define TRACE(x)
#endif


/** pointers to module initializers */
static size_t *__init_array      = NULL;
/** count of pointers to module initers */
static size_t   __init_array_size = 0;
/** the allocated size of __init_array */
static size_t   __init_array_max  = 1;
/** pointers to module finalizers */
static size_t *__fini_array      = NULL;
/** count of pointers to modules finalizers */
static size_t   __fini_array_size = 0;
/** the allocated size of __fini_array */
static size_t   __fini_array_max  = 1;


/*
 * for XCOFF we use two global variables initialized by "constructors".
 * As in the ELF case, the function pointers are invoked blindly
 */
static inline void pgasrt_walkFunctionSection(void (*func_array)(void), 
					     unsigned int nFcns)
{
  void (*funcPtr) (void);
  unsigned int seenFcns = 0;

  TRACE((stderr,"%d: Entering walkFunctionSection\n", PGASRT_MYTHREAD));

  if(!func_array) return;

  for(funcPtr = (void (*)(void))((unsigned long *)func_array)[0];
      funcPtr && seenFcns < nFcns;
      funcPtr = (void(*)(void))((unsigned long *)func_array)[++seenFcns]
      )
    (*funcPtr)();

  if(seenFcns != nFcns)
    TRACE((stderr, "Thread %d: seen %d functions out of %d\n",
	   PGASRT_MYTHREAD, seenFcns, nFcns));

  TRACE((stderr,"%d: Finished walking function section\n", PGASRT_MYTHREAD));
}


/**
 * call all the init modules stored in init_array
 */
void __pgasrt_call_init_modules()
{
    pgasrt_walkFunctionSection((void (*)(void))__init_array, (unsigned int)__init_array_size);
}

/**
 * call all the cleanup functions stored in fini_array
 */
void __pgasrt_call_fini_modules()
{
  pgasrt_walkFunctionSection((void (*)(void))__fini_array, __fini_array_size);
}

/**
 * \brief Register compiler-generated functions to initialize global
 * shared variables
 *
 * The xlupc compiler generates a specific function
 * (__xlc_upc_this_module_init) that contains code to allocate global
 * shared variables. This function is registered with the RTS by
 * calling __pgasrt_register in the C++ constructor
 * initialization routines (.sinit), which run before main.
 *
 * \param init_module a pointer to the function to run before
 *                    executing __xlc_upc_main 
 * \param fini_module a pointer to the function to run after 
 *                    __xlc_upc_main has finished
 */

#pragma weak __pgasrt_register=___pgasrt_register
void ___pgasrt_register(void *init_module, void *fini_module)
{
  TRACE((stderr,"%d: Entering __pgasrt_register\n", PGASRT_MYTHREAD));
  if(!__init_array) {
    __init_array = (unsigned long *)malloc(sizeof(void (*)(void)));
    __init_array_max = 1;
    __init_array_size = 0;
    __init_array[0] = 0;
  }

  if(!__fini_array) {
    __fini_array = (unsigned long *)malloc(sizeof(void (*)(void)));
    __fini_array_max = 1;
    __fini_array_size = 0;
    __fini_array[0] = 0;
  }

  if(init_module != NULL) {
    if(__init_array_size+1 >= __init_array_max) {
      __init_array = (unsigned long *)realloc((void *)__init_array,
              2*__init_array_max*sizeof(void *));
      __init_array_max *= 2;
    }
    __init_array[__init_array_size++] = (unsigned long)init_module;
  }

  if(fini_module != NULL) {
    if(__fini_array_size+1 >= __fini_array_max) {
      __fini_array = (unsigned long *)realloc((void *)__fini_array,
              2*__fini_array_max*sizeof(void *));
      __fini_array_max *= 2;
    }
    __fini_array[__fini_array_size++] = (unsigned long)fini_module;
  }
}

#if defined(__cplusplus)
}
#endif
