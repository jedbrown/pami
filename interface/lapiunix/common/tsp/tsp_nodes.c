/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2009                             */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */

#include <stdio.h>
#include "../include/pgasrt.h"

int __pgasrt_tsp_expected_nodes = 0;

void __pgasrt_tsp_set_expected_nodes(int expected_nodes)
{
    if (__pgasrt_tsp_expected_nodes != 0 && __pgasrt_tsp_expected_nodes != expected_nodes)
        __pgasrt_fatalerror(-1, "%s: attempt to set nodes twice", __FUNCTION__);

    __pgasrt_tsp_expected_nodes = expected_nodes;
}
