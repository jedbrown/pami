/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2007, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file tools/fakeTorusMapping.h
 * \brief ???
 */

/*
 * Fake TorusMapping so we don't need a real blue-gene system.
 * This must be the first thing included (before any other CCMI
 * or DCMF includes, at least).
 */

// Prevent the real TorusMapping.h from being included
#define __collective_engine_torus_mapping__

// Prevent the real Mapping.h, too.
#define __collective_engine_mapping__

/*
 * The constructor is called with complete "rectangle" information,
 * including the node coordinates. This is also used for Binomial
 * by only having X-dimensions (Y,Z,T size = 1). The "ranks[]" then
 * becomes a simple list of the coordinates along X.
 */

#include <stdlib.h>

namespace CCMI
{
  class Mapping
  {
  protected:
    unsigned _rank;
  public:
    Mapping(void)
    {
      _rank = 0;
    }
    void setRank(unsigned rank)
    {
      _rank = rank;
    }
    unsigned rank()
    {
      return _rank;
    }
  };
  class TorusMapping : public Mapping
  {

  protected:
    unsigned _x, _y, _z, _t;
    unsigned _xMin, _yMin, _zMin, _tMin;
    unsigned _xMax, _yMax, _zMax, _tMax;
    unsigned _xSize, _ySize, _zSize, _tSize;
    unsigned _nRanks;
  public:
    TorusMapping(void)
    {
      _x = _y = _z = _t =
           _xMin = _yMin = _zMin = _tMin =
           _xSize = _ySize = _zSize = _tSize = 0;
    }
    TorusMapping(unsigned xs, unsigned ys, unsigned zs, unsigned ts,
                 unsigned x0, unsigned y0, unsigned z0, unsigned t0,
                 unsigned x, unsigned y, unsigned z, unsigned t)
    {

      // Always computes ranks relative to (0,0,0,0)
      _xMin = x0;
      _yMin = y0;
      _zMin = z0;
      _tMin = t0;
      _xMax = x0 + xs;
      _yMax = y0 + ys;
      _zMax = z0 + zs;
      _tMax = t0 + ts;
      _xSize = xs;
      _ySize = ys;
      _zSize = zs;
      _tSize = ts;
      _nRanks = _xSize * _ySize * _zSize * _tSize;
      reset(x, y, z, t);
    }
    inline void * operator new(unsigned size)
    {
      return malloc(size);
    }
    inline void * operator new(unsigned size, void *addr)
    {
      return addr;
    }
    void reset(unsigned x, unsigned y, unsigned z, unsigned t)
    {
      unsigned r;
      _x = x;
      _y = y;
      _z = z;
      _t = t;
      (void) torus2rank(_x, _y, _z, _t, &r);
      setRank(r);
    }
    void reset(unsigned r)
    {
      setRank(r);
      rank2torus(r, _x, _y, _z, _t);
    }

    unsigned x()
    {
      return _x;
    }
    unsigned y()
    {
      return _y;
    }
    unsigned z()
    {
      return _z;
    }
    unsigned t()
    {
      return _t;
    }
    unsigned xSize()
    {
      return _xSize;
    }
    unsigned ySize()
    {
      return _ySize;
    }
    unsigned zSize()
    {
      return _zSize;
    }
    unsigned tSize()
    {
      return _tSize;
    }
    unsigned nRanks()
    {
      return _nRanks;
    }
    unsigned size()
    {
      return _nRanks;
    }

    CCMI_Result torus2rank(unsigned x, unsigned y, unsigned z, unsigned t,
                           unsigned *rank)
    {
      *rank = ((z * _yMax + y) * _xMax + x) * _tMax + t;
      return CCMI_SUCCESS;
    }

    const void rank2torus(unsigned rank, unsigned &x, unsigned &y, unsigned &z, unsigned &t)
    {
      t = rank % _tMax; rank = rank / _tMax;
      x = rank % _xMax; rank = rank / _xMax;
      y = rank % _yMax; rank = rank / _yMax;
      z = rank % _zMax; rank = rank / _zMax;
    }
  };
};
