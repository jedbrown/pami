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
#include "interface/ccmi_internal.h"
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
        int checkOp(CM_Dt dt, CM_Op op)
        {
          TRACE_ADAPTOR ((stderr, "<          >CCMI::Adaptor::Tree:checkOp((op %#X, type %#X)\n",
                          op, dt));
          switch(op)
          {
          case CM_SUM:
            switch(dt)
            {
            case CM_SIGNED_INT:
            case CM_UNSIGNED_INT:
              break;
            case CM_SIGNED_LONG_LONG:
              break;
            case CM_UNSIGNED_LONG_LONG:
              break;
            case CM_SIGNED_SHORT:
            case CM_UNSIGNED_SHORT:
              break;
            case CM_DOUBLE:
              break;
            case CM_LOGICAL:
              return -1;
            case CM_FLOAT:
              return -1;
            case CM_LONG_DOUBLE:
              return -1;
            case CM_DOUBLE_COMPLEX:
              return -1;
            case CM_SIGNED_CHAR:
            case CM_UNSIGNED_CHAR:
            default:
              return -1;
            }
            break;
          case CM_MAX:
            switch(dt)
            {
            case CM_SIGNED_INT:
              break;
            case CM_SIGNED_LONG_LONG:
              break;
            case CM_UNSIGNED_LONG_LONG:
              break;
            case CM_SIGNED_SHORT:
              break;
            case CM_UNSIGNED_INT:
              break;
            case CM_UNSIGNED_SHORT:
              break;
            case CM_FLOAT:
              break;
            case CM_DOUBLE:
              break;
            case CM_LOGICAL:
              return -1;
            case CM_LONG_DOUBLE:
              return -1;
            case CM_DOUBLE_COMPLEX:
              return -1;
            case CM_UNSIGNED_CHAR:
            case CM_SIGNED_CHAR:
            default:
              return -1;
            }
            break;
          case CM_MIN:
            switch(dt)
            {
            case CM_SIGNED_INT:
              break;
            case CM_SIGNED_LONG_LONG:
            case CM_UNSIGNED_LONG_LONG:
              break;
            case CM_SIGNED_SHORT:
              break;
            case CM_UNSIGNED_INT:
              break;
            case CM_UNSIGNED_SHORT:
              break;
            case CM_FLOAT:
              break;
            case CM_DOUBLE:
              break;
            case CM_LOGICAL:
              return -1;
            case CM_LONG_DOUBLE:
              return -1;
            case CM_DOUBLE_COMPLEX:
              return -1;
            case CM_UNSIGNED_CHAR:
            case CM_SIGNED_CHAR:
            default:
              return -1;
            }
            break;
          case CM_BAND:
            switch(dt)
            {
            case CM_LOGICAL:
            case CM_FLOAT:
            case CM_UNSIGNED_INT:
            case CM_SIGNED_INT:
              break;
            case CM_DOUBLE:
            case CM_SIGNED_LONG_LONG:
            case CM_UNSIGNED_LONG_LONG:
              break;
            case CM_UNSIGNED_SHORT:
            case CM_SIGNED_SHORT:
              break;
            case CM_UNSIGNED_CHAR:
            case CM_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case CM_BOR:
            switch(dt)
            {
            case CM_LOGICAL:
            case CM_FLOAT:
            case CM_UNSIGNED_INT:
            case CM_SIGNED_INT:
              break;
            case CM_DOUBLE:
            case CM_SIGNED_LONG_LONG:
            case CM_UNSIGNED_LONG_LONG:
              break;
            case CM_UNSIGNED_SHORT:
            case CM_SIGNED_SHORT:
              break;
            case CM_UNSIGNED_CHAR:
            case CM_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case CM_BXOR:
            switch(dt)
            {
            case CM_LOGICAL:
            case CM_FLOAT:
            case CM_UNSIGNED_INT:
            case CM_SIGNED_INT:
              break;
            case CM_DOUBLE:
            case CM_SIGNED_LONG_LONG:
            case CM_UNSIGNED_LONG_LONG:
              break;
            case CM_UNSIGNED_SHORT:
            case CM_SIGNED_SHORT:
              break;
            case CM_UNSIGNED_CHAR:
            case CM_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case CM_LAND:
            switch(dt)
            {
            case CM_LOGICAL:
            case CM_FLOAT:
            case CM_UNSIGNED_INT:
            case CM_SIGNED_INT:
              break;
            case CM_DOUBLE:
            case CM_UNSIGNED_LONG_LONG:
            case CM_SIGNED_LONG_LONG:
              break;
            case CM_UNSIGNED_SHORT:
            case CM_SIGNED_SHORT:
              break;
            case CM_UNSIGNED_CHAR:
            case CM_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case CM_LOR:
            switch(dt)
            {
            case CM_LOGICAL:
            case CM_FLOAT:
            case CM_UNSIGNED_INT:
            case CM_SIGNED_INT:
              break;
            case CM_DOUBLE:
            case CM_UNSIGNED_LONG_LONG:
            case CM_SIGNED_LONG_LONG:
              break;
            case CM_UNSIGNED_SHORT:
            case CM_SIGNED_SHORT:
              break;
            case CM_UNSIGNED_CHAR:
            case CM_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case CM_LXOR:
            switch(dt)
            {
            case CM_LOGICAL:
            case CM_FLOAT:
            case CM_UNSIGNED_INT:
            case CM_SIGNED_INT:
              break;
            case CM_DOUBLE:
            case CM_UNSIGNED_LONG_LONG:
            case CM_SIGNED_LONG_LONG:
              break;
            case CM_UNSIGNED_SHORT:
            case CM_SIGNED_SHORT:
              break;
            case CM_UNSIGNED_CHAR:
            case CM_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case CM_MAXLOC:
            switch(dt)
            {
            case CM_LOC_2INT:
              break;
            case CM_LOC_SHORT_INT:
              break;
            case CM_LOC_FLOAT_INT:
              break;
            case CM_LOC_DOUBLE_INT:
              break;
            case CM_LOC_2FLOAT:
              break;
            case CM_LOC_2DOUBLE:
              break;
            default:
              return -1;
            }
            break;
          case CM_MINLOC:
            switch(dt)
            {
            case CM_LOC_2INT:
              break;
            case CM_LOC_SHORT_INT:
              break;
            case CM_LOC_FLOAT_INT:
              break;
            case CM_LOC_DOUBLE_INT:
              break;
            case CM_LOC_2FLOAT:
              break;
            case CM_LOC_2DOUBLE:
              break;
            default:
              return -1;
            }
            break;
          case CM_PROD:
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
