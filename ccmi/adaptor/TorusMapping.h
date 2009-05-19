/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file adaptor/TorusMapping.h
 * \brief Define a torus mapping which must be implemented in the 
 * adaptor. 
 */

#ifndef   __collective_engine_torus_mapping__
#define   __collective_engine_torus_mapping__

#include "./ccmi_internal.h"
#include "./Mapping.h"

#ifndef   CCMI_TORUS_NDIMS
#define   CCMI_TORUS_NDIMS 4
#endif

#define CCMI_X_DIM 0
#define CCMI_Y_DIM 1
#define CCMI_Z_DIM 2
#define CCMI_T_DIM 3

#define CCMI_COPY_COORDS(a,b)               \
{                                           \
  int _i;                                   \
  for (_i = 0; _i < CCMI_TORUS_NDIMS; _i++) \
    a[_i] = b[_i];                          \
}

namespace CCMI
{
  class TorusMapping : public Mapping
  {
  protected:
    unsigned  _num_dims;
    unsigned _dims_lengths[CCMI_TORUS_NDIMS];
    unsigned _coords[CCMI_TORUS_NDIMS];

  public:

    TorusMapping () : Mapping ()
    {
      _num_dims = CCMI_TORUS_NDIMS;
    }

    //////////////////////////////////////////////////////////////////
    /// \brief Construct an (adaptor) mapping from a personality.
    /// Implement this in the adaptor
    /// \return new instance
    //////////////////////////////////////////////////////////////////
    TorusMapping (void * personality);
    
    //////////////////////////////////////////////////////////////////
    /// \brief Retreives the coordinate of the node in the "dim"
    ///        dimension
    /// \return physical coordinate (not mapped) in the "dim" dimension
    //////////////////////////////////////////////////////////////////
    unsigned GetCoord(unsigned dim)
    {
      return _coords[dim];
    }

    //////////////////////////////////////////////////////////////////
    /// \brief Retreives the "dim" size of the partition.  Does not
    ///        consider the mapping.
    /// \return length of "dim" 
    //////////////////////////////////////////////////////////////////
    unsigned GetDimLength(unsigned dim)
    {
      return _dims_lengths[dim];
    }

    unsigned* Coords()
    {
      return &(_coords[0]);
    }
    
    unsigned* DimsLengths()
    {
      return &(_dims_lengths[0]);
    }
    
    //////////////////////////////////////////////////////////////////
    /// \brief Returns the rank given hardware coordinates in coords
    /// \param[out] rank: rank, valid only if rc=CCMI_SUCCESS
    /// Implement this in the adaptor
    /// \return: int
    //////////////////////////////////////////////////////////////////
    CCMI_Result Torus2Rank(unsigned* coords, unsigned* rank);

    //////////////////////////////////////////////////////////////////
    /// \brief Returns the hardware coords of a logical rank
    /// Implement this in the adaptor
    /// \return Returns the mapped coords of a given rank
    //////////////////////////////////////////////////////////////////
    CCMI_Result Rank2Torus(unsigned* coords, unsigned rank) const;
    
  };  //- TorusMapping
};  //- CCMI
#endif
