/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file interface/CollectiveMapping.h
 * \brief Define a mapping which must be implemented in the 
 * adaptor. 
 */


#ifndef   __collective_engine_mapping__
#define   __collective_engine_mapping__

#include "util/ccmi_util.h"

namespace CCMI
{
  class CollectiveMapping
  {
  public:
  protected:

    void           *  _personality;
    unsigned          _rank;
    unsigned          _size;

  public:

    CollectiveMapping ():
      _personality(NULL),
      _rank(0),
      _size(0)
    {
    }

    //////////////////////////////////////////////////////////////////
    /// \brief Construct an (adaptor) mapping from a personality.
    /// Implement this in the adaptor
    /// \return new instance
    //////////////////////////////////////////////////////////////////
    CollectiveMapping (void * personality);

    //////////////////////////////////////////////////////////////////
    /// \brief Returns the mapped rank
    /// \return Logical Rank
    //////////////////////////////////////////////////////////////////
    unsigned rank()
    {
      return _rank;
    }

    //////////////////////////////////////////////////////////////////
    /// \brief Returns the size of the partition
    /// \return Returns the size of the partition (mapped)
    //////////////////////////////////////////////////////////////////
    unsigned size()
    {
      return _size;
    }
  };  //- CollectiveMapping
};  //- CCMI

#endif
