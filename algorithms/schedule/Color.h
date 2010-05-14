/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/Color.h
 * Common color definitions
 *
 */

#ifndef __algorithms_schedule_Color_h__
#define __algorithms_schedule_Color_h__

/// \todo #warning hardcoded X,Y,Z colors? Not good.

namespace CCMI
{
  namespace Schedule
  {

    typedef enum
    {
      OneColor = 1, TwoColor = 2, ThreeColor = 3,
      FourColor = 4, FiveColor = 5, SixColor = 6
    } NumColors;


/**
 * \brief Create a color value from an axis and direction
 *
 * In order to keep valus contiguous and simple to decompose,
 * the colors for the positive direction are offset by 1 (!dir)
 * so that they are 1, 2, 3.  The colors of the negative direction
 * will then be 4, 5, 6.
 *
 * Originally, colors were being used as array indices and/or in
 * numeric computations and so the ordinal properties of the values
 * was important.  If this is no longer the case, conversion between
 * axis/dir and color can be simplified even more.
 *
 * \param[in] axis
 * \param[in] dir
 * \return	integer constant value for axis+dir suitable for enum Color.
 */
#define _MK_COLOR(axis,dir)		(((axis)+!(dir))|((dir)<<2))
/**
 * \brief Extract the direction part of a color
 * \param[in] color
 * \return	Direction related to color.
 */
#define GET_DIR(color)			((color)>>2)
/**
 * \brief Extract the axis part of a color
 * \param[in] color
 * \return	Axis related to color.
 */
#define GET_AXIS(color)			(((color)&0x03)-!GET_DIR(color))

/// \todo These are also defined in Rectangle.h :(
#ifndef P_DIR
  #define P_DIR		0	/**< positive direction */
  #define N_DIR		1	/**< negative direction */
#endif
/**
 * \brief Colors - one for each standard axis and direction combination
 */
    typedef enum  /* Torus colors */
    {
      NO_COLOR = 0,       ///< "null" color value
      XP_Y_Z = _MK_COLOR(PAMI_X_DIM,P_DIR), ///< X+ vector
      YP_Z_X = _MK_COLOR(PAMI_Y_DIM,P_DIR), ///< Y+ vector
      ZP_X_Y = _MK_COLOR(PAMI_Z_DIM,P_DIR), ///< Z+ vector
      XN_Y_Z = _MK_COLOR(PAMI_X_DIM,N_DIR), ///< X- vector
      YN_Z_X = _MK_COLOR(PAMI_Y_DIM,N_DIR), ///< Y- vector
      ZN_X_Y = _MK_COLOR(PAMI_Z_DIM,N_DIR), ///< Z- vector
      MAX_COLOR,        ///< Number of colors, incl. null
    } Color;

/**
 * \brief Create a enum Color value from an axis and direction
 *
 * \param[in] axis
 * \param[in] dir
 * \return	enum Color value for axis+dir.
 */
#define MK_COLOR(axis,dir)		((Color)_MK_COLOR(axis,dir))

/**
 * \brief Compute the next axis from base color
 *
 * Computes the next axis in the sequence X => Y => Z => X ...
 *
 * Increment "n" is assume to be small.
 *
 * \param[in] color	Current color
 * \param[in] n		Increment to add to axis
 * \return	axis of color+n
 */
#define NEXT_COLOR_AXIS(color,n)	NEXT_STD_AXIS(GET_AXIS(color),n)


  }; /* Schedule */
} /* CCMI */

#endif /* !__rectangle_schedule__ */
