/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file adaptor/Mapping.h
 * \brief Define a mapping which must be implemented in the 
 * adaptor. 
 */

#ifndef   __collective_engine_mapping__
#define   __collective_engine_mapping__

#include "./ccmi_util.h"

namespace CCMI
{
  class Mapping
  {
  protected:
    void           *  _personality;
    unsigned          _rank;
    unsigned          _size;

  public:

    Mapping ()
    {
      _rank        = 0;
      _size        = 0;
      _personality = 0;
    }

    //////////////////////////////////////////////////////////////////
    /// \brief Construct an (adaptor) mapping from a personality.
    /// Implement this in the adaptor
    /// \return new instance
    //////////////////////////////////////////////////////////////////
    Mapping (void * personality);

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
  };  //- Mapping
};  //- CCMI

#endif
