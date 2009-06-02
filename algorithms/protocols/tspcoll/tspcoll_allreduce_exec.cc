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

#include "collectives/interface/lapiunix/common/include/pgasrt.h"
#include "Allreduce.h"

/* ************************************************************************* */
/*                allreduce callbacks with inline operators                  */
/* ************************************************************************* */

#define CB_ALLREDUCE(typen,opn) cb_allreduce_##opn##_##typen
#define DEFN_CB_ALLREDUCE_INLINE(typename,type,opname,op) \
static void \
CB_ALLREDUCE (typename,opname)(const void * dst, void * src, unsigned nelems) \
{ \
  type * dbuf   = (type *) dst; \
  type * pbuf   = (type *) src; \
  for (unsigned i=0; i<nelems; i++) (dbuf[i]) op (pbuf[i]); \
}

/* ------------ */
/* ADD operator */
/* ------------ */

DEFN_CB_ALLREDUCE_INLINE (byte, unsigned char,      add, +=);
DEFN_CB_ALLREDUCE_INLINE (chr,  char,               add, +=);
DEFN_CB_ALLREDUCE_INLINE (srt,  short,              add, +=);
DEFN_CB_ALLREDUCE_INLINE (hwrd, unsigned short,     add, +=);
DEFN_CB_ALLREDUCE_INLINE (int,  int,                add, +=);
DEFN_CB_ALLREDUCE_INLINE (word, unsigned,           add, +=);
DEFN_CB_ALLREDUCE_INLINE (llg,  long long,          add, +=);
DEFN_CB_ALLREDUCE_INLINE (dwrd, unsigned long long, add, +=);
DEFN_CB_ALLREDUCE_INLINE (dbl,  double,             add, +=);
DEFN_CB_ALLREDUCE_INLINE (flt,  float,              add, +=);

/* ------------- */
/* MULT operator */
/* ------------- */

DEFN_CB_ALLREDUCE_INLINE (byte, unsigned char,      mul, *=);
DEFN_CB_ALLREDUCE_INLINE (chr,  char,               mul, *=);
DEFN_CB_ALLREDUCE_INLINE (srt,  short,              mul, *=);
DEFN_CB_ALLREDUCE_INLINE (hwrd, unsigned short,     mul, *=);
DEFN_CB_ALLREDUCE_INLINE (int,  int,                mul, *=);
DEFN_CB_ALLREDUCE_INLINE (word, unsigned,           mul, *=);
DEFN_CB_ALLREDUCE_INLINE (llg,  long long,          mul, *=);
DEFN_CB_ALLREDUCE_INLINE (dwrd, unsigned long long, mul, *=);
DEFN_CB_ALLREDUCE_INLINE (dbl,  double,             mul, *=);
DEFN_CB_ALLREDUCE_INLINE (flt,  float,              mul, *=);

/* ------------- */
/* DIV operator  */
/* ------------- */

DEFN_CB_ALLREDUCE_INLINE (byte, unsigned char,      div, /=);
DEFN_CB_ALLREDUCE_INLINE (chr,  char,               div, /=);
DEFN_CB_ALLREDUCE_INLINE (srt,  short,              div, /=);
DEFN_CB_ALLREDUCE_INLINE (hwrd, unsigned short,     div, /=);
DEFN_CB_ALLREDUCE_INLINE (int,  int,                div, /=);
DEFN_CB_ALLREDUCE_INLINE (word, unsigned,           div, /=);
DEFN_CB_ALLREDUCE_INLINE (llg,  long long,          div, /=);
DEFN_CB_ALLREDUCE_INLINE (dwrd, unsigned long long, div, /=);
DEFN_CB_ALLREDUCE_INLINE (dbl,  double,             div, /=);
DEFN_CB_ALLREDUCE_INLINE (flt,  float,              div, /=);

/* ---------------------- */
/* (bitwise) AND operator */
/* ---------------------- */

DEFN_CB_ALLREDUCE_INLINE (byte, unsigned char,      and, &=);
DEFN_CB_ALLREDUCE_INLINE (chr,  char,               and, &=);
DEFN_CB_ALLREDUCE_INLINE (srt,  short,              and, &=);
DEFN_CB_ALLREDUCE_INLINE (hwrd, unsigned short,     and, &=);
DEFN_CB_ALLREDUCE_INLINE (int,  int,                and, &=);
DEFN_CB_ALLREDUCE_INLINE (word, unsigned,           and, &=);
DEFN_CB_ALLREDUCE_INLINE (llg,  long long,          and, &=);
DEFN_CB_ALLREDUCE_INLINE (dwrd, unsigned long long, and, &=);

/* ---------------------- */
/* (bitwise) OR operator  */
/* ---------------------- */

DEFN_CB_ALLREDUCE_INLINE (byte, unsigned char,      or,  |=);
DEFN_CB_ALLREDUCE_INLINE (chr,  char,               or,  |=);
DEFN_CB_ALLREDUCE_INLINE (srt,  short,              or,  |=);
DEFN_CB_ALLREDUCE_INLINE (hwrd, unsigned short,     or,  |=);
DEFN_CB_ALLREDUCE_INLINE (int,  int,                or,  |=);
DEFN_CB_ALLREDUCE_INLINE (word, unsigned,           or,  |=);
DEFN_CB_ALLREDUCE_INLINE (llg,  long long,          or,  |=);
DEFN_CB_ALLREDUCE_INLINE (dwrd, unsigned long long, or,  |=);

/* ------------- */
/* XOR operator  */
/* ------------- */

DEFN_CB_ALLREDUCE_INLINE (byte, unsigned char,      xor, ^=);
DEFN_CB_ALLREDUCE_INLINE (chr,  char,               xor, ^=);
DEFN_CB_ALLREDUCE_INLINE (srt,  short,              xor, ^=);
DEFN_CB_ALLREDUCE_INLINE (hwrd, unsigned short,     xor, ^=);
DEFN_CB_ALLREDUCE_INLINE (int,  int,                xor, ^=);
DEFN_CB_ALLREDUCE_INLINE (word, unsigned,           xor, ^=);
DEFN_CB_ALLREDUCE_INLINE (llg,  long long,          xor, ^=);
DEFN_CB_ALLREDUCE_INLINE (dwrd, unsigned long long, xor, ^=);

/* ------------------- */
/* MIN, MAX operators  */
/* ------------------- */

#define DEFN_CB_ALLREDUCE_MINMAX(typename,type,opname,op)		\
static void \
CB_ALLREDUCE(typename,opname) (const void * dst, void * src, unsigned nelems) \
{ \
  type * dbuf   = (type *) dst; \
  type * pbuf   = (type *) src; \
  for (unsigned i=0; i<nelems; i++) \
    if (dbuf[i] op pbuf[i]) dbuf[i] = pbuf[i]; \
}

DEFN_CB_ALLREDUCE_MINMAX (byte, unsigned char,      max, <);
DEFN_CB_ALLREDUCE_MINMAX (chr,  char,               max, <);
DEFN_CB_ALLREDUCE_MINMAX (srt,  short,              max, <);
DEFN_CB_ALLREDUCE_MINMAX (hwrd, unsigned short,     max, <);
DEFN_CB_ALLREDUCE_MINMAX (int,  int,                max, <);
DEFN_CB_ALLREDUCE_MINMAX (word, unsigned,           max, <);
DEFN_CB_ALLREDUCE_MINMAX (llg,  long long,          max, <);
DEFN_CB_ALLREDUCE_MINMAX (dwrd, unsigned long long, max, <);
DEFN_CB_ALLREDUCE_MINMAX (dbl,  double,             max, <);
DEFN_CB_ALLREDUCE_MINMAX (flt,  float,              max, <);

DEFN_CB_ALLREDUCE_MINMAX (byte, unsigned char,      min, >);
DEFN_CB_ALLREDUCE_MINMAX (chr,  char,               min, >);
DEFN_CB_ALLREDUCE_MINMAX (srt,  short,              min, >);
DEFN_CB_ALLREDUCE_MINMAX (hwrd, unsigned short,     min, >);
DEFN_CB_ALLREDUCE_MINMAX (int,  int,                min, >);
DEFN_CB_ALLREDUCE_MINMAX (word, unsigned,           min, >);
DEFN_CB_ALLREDUCE_MINMAX (llg,  long long,          min, >);
DEFN_CB_ALLREDUCE_MINMAX (dwrd, unsigned long long, min, >);
DEFN_CB_ALLREDUCE_MINMAX (dbl,  double,             min, >);
DEFN_CB_ALLREDUCE_MINMAX (flt,  float,              min, >);

static void CB_ALLREDUCE(dblint,max) (const void * dst, void * src, unsigned nelems) \
{ \
  typedef struct { double val; int loc; }  dblint_t;
  dblint_t * dbuf   = (dblint_t *)dst;
  dblint_t * pbuf   = (dblint_t *)src;
  for (unsigned i=0; i<nelems; i++)
    if (dbuf[i].val < pbuf[i].val) { dbuf[i].val = pbuf[i].val; dbuf[i].loc = pbuf[i].loc; }
    else if (dbuf[i].val == pbuf[i].val && dbuf[i].loc < pbuf[i].loc) dbuf[i].loc = pbuf[i].loc;
}

/* allreduce callback discriminator  */

TSPColl::Allreduce::cb_Allreduce_t  TSPColl::Allreduce::
getcallback (__pgasrt_ops_t optype, __pgasrt_dtypes_t dtype)
{
  switch (optype)
    {

    case PGASRT_OP_ADD: 
      {
	switch (dtype)
	  {
	  case PGASRT_DT_byte:   return CB_ALLREDUCE(byte,  add);
	  case PGASRT_DT_chr:    return CB_ALLREDUCE(chr,   add);
	  case PGASRT_DT_srt:    return CB_ALLREDUCE(srt,   add);
	  case PGASRT_DT_hwrd:   return CB_ALLREDUCE(hwrd,  add);
	  case PGASRT_DT_int:    return CB_ALLREDUCE(int,   add);
	  case PGASRT_DT_word:   return CB_ALLREDUCE(word,  add);
	  case PGASRT_DT_llg:    return CB_ALLREDUCE(llg,   add);
	  case PGASRT_DT_dwrd:   return CB_ALLREDUCE(dwrd,  add);
	  case PGASRT_DT_dbl:    return CB_ALLREDUCE(dbl,   add);
	  case PGASRT_DT_flt:    return CB_ALLREDUCE(flt,   add);
	  default:               break;
	  }
      }

    case PGASRT_OP_MUL: 
      {
	switch (dtype) 
	  {
	  case PGASRT_DT_byte:   return CB_ALLREDUCE(byte,  mul);
	  case PGASRT_DT_chr:    return CB_ALLREDUCE(chr,   mul);
	  case PGASRT_DT_srt:    return CB_ALLREDUCE(srt,   mul);
	  case PGASRT_DT_hwrd:   return CB_ALLREDUCE(hwrd,  mul);
	  case PGASRT_DT_int:    return CB_ALLREDUCE(int,   mul);
	  case PGASRT_DT_word:   return CB_ALLREDUCE(word,  mul);
	  case PGASRT_DT_llg:    return CB_ALLREDUCE(llg,   mul);
	  case PGASRT_DT_dwrd:   return CB_ALLREDUCE(dwrd,  mul);
	  case PGASRT_DT_dbl:    return CB_ALLREDUCE(dbl,   mul);
	  case PGASRT_DT_flt:    return CB_ALLREDUCE(flt,   mul);
          default:               break;
	  }
      }

    case PGASRT_OP_DIV: 
      {
	switch (dtype) 
	  {
	  case PGASRT_DT_byte:   return CB_ALLREDUCE(byte,  div);
	  case PGASRT_DT_chr:    return CB_ALLREDUCE(chr,   div);
	  case PGASRT_DT_srt:    return CB_ALLREDUCE(srt,   div);
	  case PGASRT_DT_hwrd:   return CB_ALLREDUCE(hwrd,  div);
	  case PGASRT_DT_int:    return CB_ALLREDUCE(int,   div);
	  case PGASRT_DT_word:   return CB_ALLREDUCE(word,  div);
	  case PGASRT_DT_llg:    return CB_ALLREDUCE(llg,   div);
	  case PGASRT_DT_dwrd:   return CB_ALLREDUCE(dwrd,  div);
	  case PGASRT_DT_dbl:    return CB_ALLREDUCE(dbl,   div);
	  case PGASRT_DT_flt:    return CB_ALLREDUCE(flt,   div);
          default:               break;
	  }
      }

    case PGASRT_OP_AND: 
      {
	switch (dtype) 
	  {
	  case PGASRT_DT_byte:   return CB_ALLREDUCE(byte,  and);
	  case PGASRT_DT_chr:    return CB_ALLREDUCE(chr,   and);
	  case PGASRT_DT_srt:    return CB_ALLREDUCE(srt,   and);
	  case PGASRT_DT_hwrd:   return CB_ALLREDUCE(hwrd,  and);
	  case PGASRT_DT_int:    return CB_ALLREDUCE(int,   and);
	  case PGASRT_DT_word:   return CB_ALLREDUCE(word,  and);
	  case PGASRT_DT_llg:    return CB_ALLREDUCE(llg,   and);
	  case PGASRT_DT_dwrd:   return CB_ALLREDUCE(dwrd,  and);
          default:               break;
	  }
      }

    case PGASRT_OP_OR: 
      {
	switch (dtype) 
	  {
	  case PGASRT_DT_byte:   return CB_ALLREDUCE(byte,  or);
	  case PGASRT_DT_chr:    return CB_ALLREDUCE(chr,   or);
	  case PGASRT_DT_srt:    return CB_ALLREDUCE(srt,   or);
	  case PGASRT_DT_hwrd:   return CB_ALLREDUCE(hwrd,  or);
	  case PGASRT_DT_int:    return CB_ALLREDUCE(int,   or);
	  case PGASRT_DT_word:   return CB_ALLREDUCE(word,  or);
	  case PGASRT_DT_llg:    return CB_ALLREDUCE(llg,   or);
	  case PGASRT_DT_dwrd:   return CB_ALLREDUCE(dwrd,  or);
          default:               break;
	  }
      }

    case PGASRT_OP_XOR: 
      {
	switch (dtype) 
	  {
	  case PGASRT_DT_byte:   return CB_ALLREDUCE(byte,  xor);
	  case PGASRT_DT_chr:    return CB_ALLREDUCE(chr,   xor);
	  case PGASRT_DT_srt:    return CB_ALLREDUCE(srt,   xor);
	  case PGASRT_DT_hwrd:   return CB_ALLREDUCE(hwrd,  xor);
	  case PGASRT_DT_int:    return CB_ALLREDUCE(int,   xor);
	  case PGASRT_DT_word:   return CB_ALLREDUCE(word,  xor);
	  case PGASRT_DT_llg:    return CB_ALLREDUCE(llg,   xor);
	  case PGASRT_DT_dwrd:   return CB_ALLREDUCE(dwrd,  xor);
          default:               break;
	  }
      }

    case PGASRT_OP_MAX: 
      {
	switch (dtype) 
	  {
	  case PGASRT_DT_byte:   return CB_ALLREDUCE(byte,  max);
	  case PGASRT_DT_chr:    return CB_ALLREDUCE(chr,   max);
	  case PGASRT_DT_srt:    return CB_ALLREDUCE(srt,   max);
	  case PGASRT_DT_hwrd:   return CB_ALLREDUCE(hwrd,  max);
	  case PGASRT_DT_int:    return CB_ALLREDUCE(int,   max);
	  case PGASRT_DT_word:   return CB_ALLREDUCE(word,  max);
	  case PGASRT_DT_llg:    return CB_ALLREDUCE(llg,   max);
	  case PGASRT_DT_dwrd:   return CB_ALLREDUCE(dwrd,  max);
	  case PGASRT_DT_dbl:    return CB_ALLREDUCE(dbl,   max);
	  case PGASRT_DT_flt:    return CB_ALLREDUCE(flt,   max);
	  case PGASRT_DT_dblint: return CB_ALLREDUCE(dblint,max);
          default:               break;
	  }
      }

    case PGASRT_OP_MIN: 
      {
	switch (dtype) 
	  {
	  case PGASRT_DT_byte:   return CB_ALLREDUCE(byte,  min);
	  case PGASRT_DT_chr:    return CB_ALLREDUCE(chr,   min);
	  case PGASRT_DT_srt:    return CB_ALLREDUCE(srt,   min);
	  case PGASRT_DT_hwrd:   return CB_ALLREDUCE(hwrd,  min);
	  case PGASRT_DT_int:    return CB_ALLREDUCE(int,   min);
	  case PGASRT_DT_word:   return CB_ALLREDUCE(word,  min);
	  case PGASRT_DT_llg:    return CB_ALLREDUCE(llg,   min);
	  case PGASRT_DT_dwrd:   return CB_ALLREDUCE(dwrd,  min);
	  case PGASRT_DT_dbl:    return CB_ALLREDUCE(dbl,   min);
	  case PGASRT_DT_flt:    return CB_ALLREDUCE(flt,   min);
#if 0
	  case PGASRT_DT_dblint: return CB_ALLREDUCE(dblint,min);
#endif
          default:               break;
	  }
      }
    default: break;
    } /* switch (op) */
  __pgasrt_fatalerror(-1, "Invalid op/type [%d/%d]", optype, dtype);
  return NULL;
}

size_t TSPColl::Allreduce::datawidthof (__pgasrt_dtypes_t dtype)
{
  switch(dtype)
    {
    case PGASRT_DT_byte:   return sizeof(unsigned char);
    case PGASRT_DT_chr:    return sizeof(char);
    case PGASRT_DT_srt:    return sizeof(short);
    case PGASRT_DT_hwrd:   return sizeof(unsigned short);
    case PGASRT_DT_int:    return sizeof(int);
    case PGASRT_DT_word:   return sizeof(unsigned);
    case PGASRT_DT_llg:    return sizeof(long long);
    case PGASRT_DT_dwrd:   return sizeof(unsigned long long);
    case PGASRT_DT_dbl:    return sizeof(double);
    case PGASRT_DT_flt:    return sizeof(float);
    case PGASRT_DT_dblint: return sizeof(double) + sizeof(int);
    }
  return 0;
}




