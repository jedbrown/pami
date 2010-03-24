/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/protocols/allreduce/TreeReduceFunctions.cc
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
        int checkOp(PAMI_Dt dt, PAMI_Op op)
        {
          TRACE_ADAPTOR ((stderr, "<          >CCMI::Adaptor::Tree:checkOp((op %#X, type %#X)\n",
                          op, dt));
          switch(op)
          {
          case PAMI_SUM:
            switch(dt)
            {
            case PAMI_SIGNED_INT:
            case PAMI_UNSIGNED_INT:
              break;
            case PAMI_SIGNED_LONG_LONG:
              break;
            case PAMI_UNSIGNED_LONG_LONG:
              break;
            case PAMI_SIGNED_SHORT:
            case PAMI_UNSIGNED_SHORT:
              break;
            case PAMI_DOUBLE:
              break;
            case PAMI_LOGICAL:
              return -1;
            case PAMI_FLOAT:
              return -1;
            case PAMI_LONG_DOUBLE:
              return -1;
            case PAMI_DOUBLE_COMPLEX:
              return -1;
            case PAMI_SIGNED_CHAR:
            case PAMI_UNSIGNED_CHAR:
            default:
              return -1;
            }
            break;
          case PAMI_MAX:
            switch(dt)
            {
            case PAMI_SIGNED_INT:
              break;
            case PAMI_SIGNED_LONG_LONG:
              break;
            case PAMI_UNSIGNED_LONG_LONG:
              break;
            case PAMI_SIGNED_SHORT:
              break;
            case PAMI_UNSIGNED_INT:
              break;
            case PAMI_UNSIGNED_SHORT:
              break;
            case PAMI_FLOAT:
              break;
            case PAMI_DOUBLE:
              break;
            case PAMI_LOGICAL:
              return -1;
            case PAMI_LONG_DOUBLE:
              return -1;
            case PAMI_DOUBLE_COMPLEX:
              return -1;
            case PAMI_UNSIGNED_CHAR:
            case PAMI_SIGNED_CHAR:
            default:
              return -1;
            }
            break;
          case PAMI_MIN:
            switch(dt)
            {
            case PAMI_SIGNED_INT:
              break;
            case PAMI_SIGNED_LONG_LONG:
            case PAMI_UNSIGNED_LONG_LONG:
              break;
            case PAMI_SIGNED_SHORT:
              break;
            case PAMI_UNSIGNED_INT:
              break;
            case PAMI_UNSIGNED_SHORT:
              break;
            case PAMI_FLOAT:
              break;
            case PAMI_DOUBLE:
              break;
            case PAMI_LOGICAL:
              return -1;
            case PAMI_LONG_DOUBLE:
              return -1;
            case PAMI_DOUBLE_COMPLEX:
              return -1;
            case PAMI_UNSIGNED_CHAR:
            case PAMI_SIGNED_CHAR:
            default:
              return -1;
            }
            break;
          case PAMI_BAND:
            switch(dt)
            {
            case PAMI_LOGICAL:
            case PAMI_FLOAT:
            case PAMI_UNSIGNED_INT:
            case PAMI_SIGNED_INT:
              break;
            case PAMI_DOUBLE:
            case PAMI_SIGNED_LONG_LONG:
            case PAMI_UNSIGNED_LONG_LONG:
              break;
            case PAMI_UNSIGNED_SHORT:
            case PAMI_SIGNED_SHORT:
              break;
            case PAMI_UNSIGNED_CHAR:
            case PAMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case PAMI_BOR:
            switch(dt)
            {
            case PAMI_LOGICAL:
            case PAMI_FLOAT:
            case PAMI_UNSIGNED_INT:
            case PAMI_SIGNED_INT:
              break;
            case PAMI_DOUBLE:
            case PAMI_SIGNED_LONG_LONG:
            case PAMI_UNSIGNED_LONG_LONG:
              break;
            case PAMI_UNSIGNED_SHORT:
            case PAMI_SIGNED_SHORT:
              break;
            case PAMI_UNSIGNED_CHAR:
            case PAMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case PAMI_BXOR:
            switch(dt)
            {
            case PAMI_LOGICAL:
            case PAMI_FLOAT:
            case PAMI_UNSIGNED_INT:
            case PAMI_SIGNED_INT:
              break;
            case PAMI_DOUBLE:
            case PAMI_SIGNED_LONG_LONG:
            case PAMI_UNSIGNED_LONG_LONG:
              break;
            case PAMI_UNSIGNED_SHORT:
            case PAMI_SIGNED_SHORT:
              break;
            case PAMI_UNSIGNED_CHAR:
            case PAMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case PAMI_LAND:
            switch(dt)
            {
            case PAMI_LOGICAL:
            case PAMI_FLOAT:
            case PAMI_UNSIGNED_INT:
            case PAMI_SIGNED_INT:
              break;
            case PAMI_DOUBLE:
            case PAMI_UNSIGNED_LONG_LONG:
            case PAMI_SIGNED_LONG_LONG:
              break;
            case PAMI_UNSIGNED_SHORT:
            case PAMI_SIGNED_SHORT:
              break;
            case PAMI_UNSIGNED_CHAR:
            case PAMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case PAMI_LOR:
            switch(dt)
            {
            case PAMI_LOGICAL:
            case PAMI_FLOAT:
            case PAMI_UNSIGNED_INT:
            case PAMI_SIGNED_INT:
              break;
            case PAMI_DOUBLE:
            case PAMI_UNSIGNED_LONG_LONG:
            case PAMI_SIGNED_LONG_LONG:
              break;
            case PAMI_UNSIGNED_SHORT:
            case PAMI_SIGNED_SHORT:
              break;
            case PAMI_UNSIGNED_CHAR:
            case PAMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case PAMI_LXOR:
            switch(dt)
            {
            case PAMI_LOGICAL:
            case PAMI_FLOAT:
            case PAMI_UNSIGNED_INT:
            case PAMI_SIGNED_INT:
              break;
            case PAMI_DOUBLE:
            case PAMI_UNSIGNED_LONG_LONG:
            case PAMI_SIGNED_LONG_LONG:
              break;
            case PAMI_UNSIGNED_SHORT:
            case PAMI_SIGNED_SHORT:
              break;
            case PAMI_UNSIGNED_CHAR:
            case PAMI_SIGNED_CHAR:
              break;
            default:
              return -1;
            }
            break;
          case PAMI_MAXLOC:
            switch(dt)
            {
            case PAMI_LOC_2INT:
              break;
            case PAMI_LOC_SHORT_INT:
              break;
            case PAMI_LOC_FLOAT_INT:
              break;
            case PAMI_LOC_DOUBLE_INT:
              break;
            case PAMI_LOC_2FLOAT:
              break;
            case PAMI_LOC_2DOUBLE:
              break;
            default:
              return -1;
            }
            break;
          case PAMI_MINLOC:
            switch(dt)
            {
            case PAMI_LOC_2INT:
              break;
            case PAMI_LOC_SHORT_INT:
              break;
            case PAMI_LOC_FLOAT_INT:
              break;
            case PAMI_LOC_DOUBLE_INT:
              break;
            case PAMI_LOC_2FLOAT:
              break;
            case PAMI_LOC_2DOUBLE:
              break;
            default:
              return -1;
            }
            break;
          case PAMI_PROD:
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
