/*  (C)Copyright IBM Corp.  2007, 2008  */
/**
 * \file sys/compact_attributes.h
 * \brief ???
 */

#ifndef _COMPACT_ATTRIBUTES_H_
#define _COMPACT_ATTRIBUTES_H_

/* denotes the end of arguments in variadic functions */
#define CA_END_ARGS   -1


/* should this be size_t????? */
typedef int CA_Mask;

#define CA_BYTE_SIZE       (sizeof(char))
#define CA_MASK_NUM_BYTES  (sizeof(CA_Mask))
#define CA_TOTAL_BITS      (CA_BYTE_SIZE * CA_MASK_NUM_BYTES)
#define CA_NUM_ELEMENTS    1

#define CA_BIT(b)       ((b) / CA_TOTAL_BITS)
#define CA_MASK(b)      ((CA_Mask) 1 << ((b) % CA_TOTAL_BITS))


/* compact list of informative attributes in the form of bits*/
typedef struct
{
  /* initially 1 list element of 32 bits */
  CA_Mask ca_list[CA_NUM_ELEMENTS];
} ca_list;

#define CA_ELEMENT(list) ((list)->ca_list)

/* sets a bit in the attribute list */
#define CA_SET(l, a)   (CA_ELEMENT (l)[CA_BIT(a)] |= CA_MASK(a))

/* unsets a bit in the attribute list */
#define CA_UNSET(l, a) (CA_ELEMENT (l)[CA_BIT(a)] &= ~CA_MASK(a))

/* checks an attribute bit in the attribute list */
#define CA_ISSET(l, a) ((CA_ELEMENT (l)[CA_BIT(a)] & CA_MASK(a)) != 0)


/* unset all attributes bits */
#define CA_UNSET_ALL(l)                                                       \
  do                                                                          \
  {                                                                           \
    unsigned int __i, __j;                                                    \
    ca_list *__list = (l);                                                    \
    for (__i = 0; __i < CA_NUM_ELEMENTS; __i++)                               \
      CA_ELEMENT (__list)[__i] = 0;                                           \
  } while (0)

/* set all attributes bits up to a range 'r' (i.e. from 0 .. r) */
#define CA_SET_ALL(l, r)                                                      \
  do                                                                          \
  {                                                                           \
    unsigned int __i;                                                         \
    for (__i = 0; __i < r; ++__i)  CA_SET(l, __i);                            \
  } while (0)

/* copy attribute list s into list d */
#define CA_COPY(s, d)                                                         \
  do                                                                          \
  {                                                                           \
    unsigned int __i, __j;                                                    \
    ca_list  *__src = (s);                                                    \
    ca_list  *__dst = (d);                                                    \
    for (__i = 0; __i < CA_NUM_ELEMENTS; ++__i)                               \
      CA_ELEMENT (__dst)[__i] |= CA_ELEMENT (__src)[__i];                     \
  } while (0)


/* utility macro to print '0' or '1' to indicate attributes are set/unset */
#define CA_PRINT(l)                                                           \
  do                                                                          \
  {                                                                           \
    unsigned int __i, __bit, __element;                                       \
    for (__element = CA_NUM_ELEMENTS - 1; __element >=0; __element--)         \
    {                                                                         \
      for (__i = CA_TOTAL_BITS - 1; i >= 0; i--)                              \
      {                                                                       \
        if (((__i+1) % 4 == 0) && (__i+1) != CA_TOTAL_BITS) printf("-");      \
        __bit = (((CA_ELEMENT (l)[__element]) >> __i) & 1);                   \
        printf("%d", __bit);                                                  \
      }                                                                       \
      printf("-");                                                            \
    }                                                                         \
    printf("\n");                                                             \
  } while (0)                                                                 \

/* is 'l1' attribute list a subset of 'l2' list, if so, 'result' is true */
#define CA_ISSUBSET(l1, l2, result)                                           \
  do                                                                          \
  {                                                                           \
    result = 1;                                                               \
    unsigned int __i;                                                         \
    ca_list *__l1 = (l1);                                                     \
    ca_list *__l2 = (l2);                                                     \
    for (__i = 0; __i < CA_NUM_ELEMENTS; __i++)                               \
      if ((CA_ELEMENT (__l1)[__i] & CA_ELEMENT (__l2)[__i]) !=                \
          CA_ELEMENT (__l2)[__i])                                             \
      {                                                                       \
        result = 0;                                                           \
        break;                                                                \
      }                                                                       \
  } while (0)                                                                 \

/* this sets multiple attributes in a list in a single call */
/* extern void CA_MSET(ca_list *list, ...);*/

#endif
