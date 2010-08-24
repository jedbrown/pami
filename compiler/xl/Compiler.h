/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file compiler/xl/Compiler.h
 * \brief ???
 */

#ifndef __compiler_xl_Compiler_h__
#define __compiler_xl_Compiler_h__

// Here go things specific to this compiler family
/**
 * \todo I am not convinced this is the right way to handle gcc in xl - too much questionable casting.
 * plus, these XL instrinsics are specific to type and GCC are not. We need to
 * use (e.g.) __fetch_and_addlp() for long and __fetch_and_add() for int.
 * Also, __compare_and_swap() modifies param 2 while GCC does not.
 */

#include "builtins.h"

#define __sync_synchronize() __sync()

#ifdef __64BIT__
#define __sync_fetch_and_or(x,y)	__fetch_and_orlp((volatile unsigned long *)x,y)
#define __sync_fetch_and_and(x,y)	__fetch_and_andlp((volatile unsigned long *)x,y)
#define __sync_fetch_and_add(x,y)	__fetch_and_addlp((volatile long *)x,y)
#define __sync_fetch_and_sub(x,y)	__fetch_and_addlp((volatile long *)x,-(y))
#define __sync_fetch_and_swap(x,y)	__fetch_and_swaplp((volatile long *)x,y)

#define __sync_bool_compare_and_swap(x,y,z) ({		\
	bool _b;					\
	long _y = (long)y;				\
	_b = __compare_and_swaplp((volatile long *)x,&_y,(long)z);\
	_b;						\
})
#define __sync_val_compare_and_swap(x,y,z) (__typeof__(*x))({\
	long _y = (long)y;				\
	__compare_and_swaplp((volatile long *)x,&_y,(long)z);\
	_y;						\
})
#else	// 32-bit
#define __sync_fetch_and_or(x,y)	__fetch_and_or((volatile unsigned int *)x,y)
#define __sync_fetch_and_and(x,y)	__fetch_and_and((volatile unsigned int *)x,y)
#define __sync_fetch_and_add(x,y)	__fetch_and_add((volatile int *)x,y)
#define __sync_fetch_and_sub(x,y)	__fetch_and_add((volatile int *)x,-(y))
#define __sync_fetch_and_swap(x,y)	__fetch_and_swap((volatile int *)x,y)
#define __sync_bool_compare_and_swap(x,y,z) ({		\
	bool _b;					\
	int _y = (int)y;				\
	_b = __compare_and_swap((volatile int *)x,&_y,(int)z);\
	_b;						\
})
#define __sync_val_compare_and_swap(x,y,z) (__typeof__(*x))({\
	int _y = (int)y;				\
	__compare_and_swap((volatile int *)x,&_y,(int)z);\
	_y;						\
})
#endif	// 32/64-bit

#endif // __pami_compiler_xl_h__
