/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/tree/TreeReduceFunctions.cc
 * \brief Support functions for the tree operations
 */

#include "util/ccmi_util.h"  // need this first to define throw() before stdlib.h is included (for xlC -noeh)
#include "algorithms/ccmi.h"
#include "math/math_coremath.h"
#include "algorithms/executor/AllreduceBase.h"
//#include "protocols/allreduce/Factory.h"

namespace CCMI
{
  namespace Adaptor
  {
    namespace Allreduce
    {
      namespace Tree
      {
        ///
        /// \brief check the reduce function and datatype and see if
        /// it's supported by the tree
        ///
        int checkOp(XMI_Dt dt, XMI_Op op)
        {
          TRACE_ADAPTOR ((stderr, "<          >CCMI::Adaptor::Tree:checkOp((op %#X, type %#X)\n",
                          op, dt));
          switch(op)
          {
          case XMI_SUM:
            switch(dt)
            {
            case XMI_SIGNED_INT:
            case XMI_UNSIGNED_INT:
              break;
            case XMI_SIGNED_LONG_LONG:
              break;
            case XMI_UNSIGNED_LONG_LONG:
              break;
            case XMI_SIGNED_SHORT:
            case XMI_UNSIGNED_SHORT:
              break;
            case XMI_DOUBLE:
              break;
            case XMI_LOGICAL:
              return -1;
            case XMI_FLOAT:
              return -1;
            case XMI_LONG_DOUBLE:
              return -1;
            case XMI_DOUBLE_COMPLEX:
              return -1;
            case XMI_SIGNED_CHAR:
            case XMI_UNSIGNED_CHAR:
            default:
              return -1;
            }
            break;
          case XMI_MAX:
            switch(dt)
            {
            case XMI_SIGNED_INT:
              break;
            case XMI_SIGNED_LONG_LONG:
              break;
            case XMI_UNSIGNED_LONG_LONG:
              break;
            case XMI_SIGNED_SHORT:
              break;
            case XMI_UNSIGNED_INT:
              break;
            case XMI_UNSIGNED_SHORT:
              break;
            case XMI_FLOAT:
              break;
            case XMI_DOUBLE:
              break;
            case XMI_LOGICAL:
              return -1;
            case XMI_LONG_DOUBLE:
              return -1;
            case XMI_DOUBLE_COMPLEX:
              return -1;
            case XMI_UNSIGNED_CHAR:
            case XMI_SIGNED_CHAR:
            default:
              return -1;
            }
            break;
          case XMI_MIN:
            switch(dt)
            {
            case XMI_SIGNED_INT:
              break;
            case XMI_SIGNED_LONG_LONG:
            case XMI_UNSIGNED_LONG_LONG:
              break;
            case XMI_SIGNED_SHORT:
              break;
            case XMI_UNSIGNED_INT:
              break;
            case XMI_UNSIGNED_SHORT:
              break;
            case XMI_FLOAT:
              break;
            case XMI_DOUBLE:
              break;
            case XMI_LOGICAL:
              return -1;
            case XMI_LONG_DOUBLE:
              return -1;
            case XMI_DOUBLE_COMPLEX:
              return -1;
            case XMI_UNSIGNED_CHAR:
            case XMI_SIGNED_CHAR:
            default:
              return -1;
            }
            break;
          case XMI_BAND:
            switch(dt)
            {
            case XMI_LOGICAL:
            case XMI_FLOAT:
            case XMI_UNSIGNED_INT:
            case XMI_SIGNED_INT:
              break;
            case XMI_DOUBLE:
            case XMI_SIGNED_LONG_LONG:
            case XMI_UNSIGNED_LONG_LONG:
              break;
            case XMI_UNSIGNED_SHORT:
            case XMI_SIGNED_SHORT:
              break;
            case XMI_UNSIGNED_CHAR:
            case XMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case XMI_BOR:
            switch(dt)
            {
            case XMI_LOGICAL:
            case XMI_FLOAT:
            case XMI_UNSIGNED_INT:
            case XMI_SIGNED_INT:
              break;
            case XMI_DOUBLE:
            case XMI_SIGNED_LONG_LONG:
            case XMI_UNSIGNED_LONG_LONG:
              break;
            case XMI_UNSIGNED_SHORT:
            case XMI_SIGNED_SHORT:
              break;
            case XMI_UNSIGNED_CHAR:
            case XMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case XMI_BXOR:
            switch(dt)
            {
            case XMI_LOGICAL:
            case XMI_FLOAT:
            case XMI_UNSIGNED_INT:
            case XMI_SIGNED_INT:
              break;
            case XMI_DOUBLE:
            case XMI_SIGNED_LONG_LONG:
            case XMI_UNSIGNED_LONG_LONG:
              break;
            case XMI_UNSIGNED_SHORT:
            case XMI_SIGNED_SHORT:
              break;
            case XMI_UNSIGNED_CHAR:
            case XMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case XMI_LAND:
            switch(dt)
            {
            case XMI_LOGICAL:
            case XMI_FLOAT:
            case XMI_UNSIGNED_INT:
            case XMI_SIGNED_INT:
              break;
            case XMI_DOUBLE:
            case XMI_UNSIGNED_LONG_LONG:
            case XMI_SIGNED_LONG_LONG:
              break;
            case XMI_UNSIGNED_SHORT:
            case XMI_SIGNED_SHORT:
              break;
            case XMI_UNSIGNED_CHAR:
            case XMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case XMI_LOR:
            switch(dt)
            {
            case XMI_LOGICAL:
            case XMI_FLOAT:
            case XMI_UNSIGNED_INT:
            case XMI_SIGNED_INT:
              break;
            case XMI_DOUBLE:
            case XMI_UNSIGNED_LONG_LONG:
            case XMI_SIGNED_LONG_LONG:
              break;
            case XMI_UNSIGNED_SHORT:
            case XMI_SIGNED_SHORT:
              break;
            case XMI_UNSIGNED_CHAR:
            case XMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case XMI_LXOR:
            switch(dt)
            {
            case XMI_LOGICAL:
            case XMI_FLOAT:
            case XMI_UNSIGNED_INT:
            case XMI_SIGNED_INT:
              break;
            case XMI_DOUBLE:
            case XMI_UNSIGNED_LONG_LONG:
            case XMI_SIGNED_LONG_LONG:
              break;
            case XMI_UNSIGNED_SHORT:
            case XMI_SIGNED_SHORT:
              break;
            case XMI_UNSIGNED_CHAR:
            case XMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case XMI_MAXLOC:
            switch(dt)
            {
            case XMI_LOC_2INT:
              break;
            case XMI_LOC_SHORT_INT:
              break;
            case XMI_LOC_FLOAT_INT:
              break;
            case XMI_LOC_DOUBLE_INT:
              break;
            case XMI_LOC_2FLOAT:
              break;
            case XMI_LOC_2DOUBLE:
              break;
            default:
              return -1;
            }
            break;
          case XMI_MINLOC:
            switch(dt)
            {
            case XMI_LOC_2INT:
              break;
            case XMI_LOC_SHORT_INT:
              break;
            case XMI_LOC_FLOAT_INT:
              break;
            case XMI_LOC_DOUBLE_INT:
              break;
            case XMI_LOC_2FLOAT:
              break;
            case XMI_LOC_2DOUBLE:
              break;
            default:
              return -1;
            }
            break;
          case XMI_PROD:
            return -1;
          default:
            return -1;
          }
          TRACE_ADAPTOR ((stderr, "<          >CCMI::Adaptor::Tree:checkOp() supported\n"));
          return 0;
        }
      }
    }
  }
}
