/*  (C)Copyright IBM Corp.  2009, 2010  */
/**
 * \file sys/xmi_attributes.h
 * \brief ???
 */

#ifndef __xmi_attributes_h__
#define __xmi_attributes_h__

#define XMI_ISPOF2(x)   (!(((x) - 1) & (x)) ? 1 : 0)
#define XMI_ISEVEN(x)   (((x) % 2) ? 0 : 1)

/* Each '#define' below represents an informative attribute                   */
/*
 * initially, each variable can define up to 32 attributes. One can extend that
 */

/******************************************************************************/
/* Geometry attributes                                                        */
/******************************************************************************/
#define XMI_GEOMETRY_POF2                                                      0
#define XMI_GEOMETRY_ODD                                                       1
#define XMI_GEOMETRY_TREE                                                      2
#define XMI_GEOMETRY_RECT                                                      3
#define XMI_GEOMETRY_TORUS                                                     4
#define XMI_GEOMETRY_STAR                                                      5
#define XMI_GEOMETRY_CROSSBAR                                                  6
#define XMI_GEOMETRY_IRREG                                                     7
#define XMI_GEOMETRY_LOCAL                                                     8
#define XMI_GEOMETRY_GLOBAL                                                    9

/******************************************************************************/
/* Buffer/Operation/Datatype/msg_size ranges attributes                       */
/******************************************************************************/
#define XMI_CONTIG_SBUFF                                                       0
#define XMI_CONTIG_RBUFF                                                       1
#define XMI_CONTIN_RBUFF                                                       2
#define XMI_ALIGNED_BUFF                                                       3
#define XMI_4XBYTE_BUFF                                                        4
#define XMI_OP_TYPE_TREE                                                       5
#define XMI_OP_TYPE_TORUS                                                      6
#define XMI_OP_TYPE_ANY                                                        7
#define XMI_MSIZE_RANGE_1                                                      8
#define XMI_MSIZE_RANGE_2                                                      9
#define XMI_MSIZE_RANGE_3                                                     10
#define XMI_MSIZE_RANGE_4                                                     11
#define XMI_MSIZE_RANGE_5                                                     12
#define XMI_MSIZE_RANGE_6                                                     13
#define XMI_MSIZE_RANGE_7                                                     14
#define XMI_MSIZE_RANGE_8                                                     15

/******************************************************************************/
/* platform mode of operation, thread levels, misc                            */
/******************************************************************************/
#define XMI_MODE_SMP                                                           0
#define XMI_MODE_DUAL                                                          1
#define XMI_MODE_VN                                                            2
#define XMI_MODE_THREADED                                                      3
#define XMI_MODE_UNTHREADED                                                    4

#endif
