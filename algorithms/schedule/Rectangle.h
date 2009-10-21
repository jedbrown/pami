/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file algorithms/schedule/Rectangle.h
 * The Rectangle Schedule
 *
 * \todo This needs to be modified to use SpraySchedule instead of
 * just hard-coding a spray schedule in the local cores phase.
 * I.e. make the local cores phase plugable with any schedule.
 */

#ifndef __rectangle_schedule__
#define __rectangle_schedule__
#define HAVE_NEW_RECT_SCHED	// until branch is merged into main

/**
 * @defgroup BGTORUS_AXIS Axii used on BG Torus/Mesh network.
 *@{
 */

/*@}*/
/** \brief Number of axii on BG Torus/Mesh network. */

// TODO:  make these queryable by the mapping
#define NUM_STD_AXIS	2
/** \brief Total number of axii on BG (includes local cores). */
#define NUM_AXIS	2

typedef size_t axis_array_t[NUM_STD_AXIS];

/** \brief Type which holds a set of coordinates (axii - X,Y,Z,T) */
typedef size_t axii_t[NUM_AXIS];

#include "Schedule.h"
#include "algorithms/schedule/BinomialTree.h"
#include "algorithms/schedule/RingSchedule.h"

namespace CCMI
{
  namespace Schedule
  {

// reset indentation
    typedef enum
    {
      OneColor = 1, TwoColor = 2, ThreeColor = 3,
      FourColor = 4, FiveColor = 5, SixColor = 6
    } NumColors;

/**
 * @defgroup BGRECT_PHASES Phases (starting) used by rectangle schedule.
 *@{
 */
#define PHASE_NONE	((unsigned)-1)	/**< not a valid phase */
#define PHASE_ONE	0	/**< root bcast to pri axis */
#define PHASE_TWO	1	/**< face node bcast to sec axis */
#define PHASE_THREE	2	/**< sec axis bcast to all of face (ter axis) */
#define PHASE_FOUR	3	/**< face nodes bcast back on pri axis */
#define PHASE_FIVE	4	/**< (begin) local core-to-core. NCores-1 phases
				 * begin here */
/*@}*/
/** \brief Number of (starting) phases. */
#define NUM_PHASES	5

/**
 * \brief Increment an axis by some small value
 *
 * Uses the circular sequence X -> Y -> Z -> X ... (depends on
 * values assigned for axii - CCMI_X_DIM, CCMI_Y_DIM, CCMI_Z_DIM).
 *
 * Assumes "n" is small, <= NUM_STD_AXIS. Also assumes 'axis'
 * is a valid "standard" axis, i.e. axis < NUM_STD_AXIS.
 *
 * \param[in] axis	Current axis
 * \param[in] n		Increment to add to axis
 * \return	axis+n => NUM_STD_AXIS
 */
    inline int NEXT_STD_AXIS(int axis, int n)
    {
      int _x = axis + n;
      if(_x >= NUM_STD_AXIS) _x -= NUM_STD_AXIS;
      return _x;
    }

/**
 * @defgroup BGTORUS_DIR Directions along an axis on BG Torus/Mesh network.
 *@{
 */
#define P_DIR		0	/**< positive direction */
#define N_DIR		1	/**< negative direction */
/*@}*/
/** \brief Number of directions defined */
#define NUM_DIR		2

/**
 * \brief Convert direction to opposite direction
 *
 * \param[in] dir	Direction to convert (P_DIR or N_DIR)
 * \return	Opposite direction (N_DIR or P_DIR)
 */
#define OPP_DIR(dir)	((dir) ^ 1)

/**
 * \brief Translation table for axis+direction into DMA bcast command.
 *
 * Since C++ does not support designated initializers, we have to assume
 * a particular order of memory placement w.r.t. multi-dimension array indices.
 * We also have to assume the numerical values for axii and directions.
 * This code really should be (in C would be):
 *
 * static unsigned short line_bcast[NUM_STD_AXIS][NUM_DIR] = {
 *	[CCMI_X_DIM][P_DIR] = CCMI_LINEBCAST_XP,
 *	[CCMI_X_DIM][N_DIR] = CCMI_LINEBCAST_XM,
 *	[CCMI_Y_DIM][P_DIR] = CCMI_LINEBCAST_YP,
 *	[CCMI_Y_DIM][N_DIR] = CCMI_LINEBCAST_YM,
 *	[CCMI_Z_DIM][P_DIR] = CCMI_LINEBCAST_ZP,
 *	[CCMI_Z_DIM][N_DIR] = CCMI_LINEBCAST_ZM,
 * };
 */
// \todo:  make this work from a query of axis size and num directions!
    static unsigned short line_bcast[NUM_STD_AXIS][NUM_DIR] = {
      { XMI_LINE_BCAST_XP, XMI_LINE_BCAST_XM},
//      { XMI_LINE_BCAST_YP, XMI_LINE_BCAST_YM},
//      { XMI_LINE_BCAST_ZP, XMI_LINE_BCAST_ZM},
    };


/** \brief The maximum number of phases*steps needed by any node.
 * Any given node will do line broadcasts on at most 3 axii, with
 * each line broadcast involving at most 2 directions.
 * No step is consumed by local pt2pt sends, as they are
 * always the same and can be built when getDstPeList() is called.
 * Existence of a _startphase implies existence of these local steps.
 */
#define MAX_NUM_STEPS	(NUM_DIR * NUM_STD_AXIS)

/**
 * \brief Structure to store rectangle
 * \note This must be the same memory layout as struct axis_rect!
 * i.e. must be valid to cast Rectangle to (struct axis_rect *).
 */
    struct Rectangle
    {
      unsigned x0;  /**< lower left corner, x coordinate */
      unsigned y0;  /**< lower left corner, y coordinate */
      unsigned z0;  /**< lower left corner, z coordinate */
      unsigned t0;  /**< lower left corner, t coordinate */
      unsigned xs;  /**< x size */
      unsigned ys;  /**< y size */
      unsigned zs;  /**< z size */
      unsigned ts;  /**< t size */
      unsigned isTorusX;    /**< torus wrap around link in X dir*/
      unsigned isTorusY;    /**< torus wrap around link in Y dir*/
      unsigned isTorusZ;    /**< torus wrap around link in Z dir*/
      unsigned isTorusT;    /**< torus wrap around link in T dir*/
    };

/**
 * \brief Structure to store rectangle by axii_t
 * \note This must be the same memory layout as struct Rectangle!
 * i.e. must be valid to cast Rectangle to (struct axis_rect *).
 */
    struct axis_rect
    {
      axii_t x0;  /**< lower left front corner */
      axii_t xs;  /**< size (upper right back corner + (1,1,1,1) */
      axii_t xt;
    };

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

/**
 * \brief Colors - one for each standard axis and direction combination
 */
    typedef enum  /* Torus colors */
    {
      NO_COLOR = 0,       ///< "null" color value
      XP_Y_Z = _MK_COLOR(XMI_X_DIM,P_DIR), ///< X+ vector
      YP_Z_X = _MK_COLOR(XMI_Y_DIM,P_DIR), ///< Y+ vector
      ZP_X_Y = _MK_COLOR(XMI_Z_DIM,P_DIR), ///< Z+ vector
      XN_Y_Z = _MK_COLOR(XMI_X_DIM,N_DIR), ///< X- vector
      YN_Z_X = _MK_COLOR(XMI_Y_DIM,N_DIR), ///< Y- vector
      ZN_X_Y = _MK_COLOR(XMI_Z_DIM,N_DIR), ///< Z- vector
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

/**
 * \brief Get rank of given coords (axii_t)
 *
 * \param[in] map	CollectiveMapping to use
 * \param[in] x		Coordinates to convert into rank
 * \return	nothing
 */
    static inline unsigned coord2rank(XMI_MAPPING_CLASS *map, axii_t x)
    {
      size_t rank = XMI_UNDEFINED_RANK;
      xmi_result_t rc = map->task2global(rank, (size_t (&)[NUM_STD_AXIS])x);
      return(rc == XMI_SUCCESS ? rank : XMI_UNDEFINED_RANK);
    }

/**
 * \brief Get coords (axii_t) of given rank
 *
 * \param[in] map	CollectiveMapping to use
 * \param[in] rank	Rank to convert
 * \param[out] x	Coordinates to return
 * \return	nothing
 */
    static inline void rank2coord(XMI_MAPPING_CLASS *map, size_t rank, axii_t x)
    {
      map->global2task((axis_array_t &)x, rank);
    }

/**
 * \brief Get coords (axii_t) of my rank
 *
 * This is an optimization of rank2coord(map, map->rank(), x);
 *
 * \param[in] map	CollectiveMapping to use
 * \param[out] x	Coordinates to return
 * \return	nothing
 */
    static inline void get_my_coord(XMI_MAPPING_CLASS *map, axii_t x)
    {
      rank2coord(map, map->task(), x);
//      x[XMI_X_DIM] = map->GetCoord(XMI_X_DIM);
//      x[XMI_Y_DIM] = map->GetCoord(XMI_Y_DIM);
//      x[XMI_Z_DIM] = map->GetCoord(XMI_Z_DIM);
//      x[XMI_T_DIM] = map->GetCoord(XMI_T_DIM);
    }

/**
 * \brief Node Set ID
 * Where we fall within the Rectangular Algorithm.
 */
    typedef enum
    {
      NODE_SET_A, /**< Root Node */
      NODE_SET_B, /**< Face Node of root, !A */
      NODE_SET_C, /**< Pri Axis with root, excl B, A */
      NODE_SET_D, /**< Sec axis of root face, excl C,B,A */
      NODE_SET_E, /**< Ter axis of D, excl D, C, B, A */
      NODE_SET_F, /**< Rest of nodes, excl E,D,C,B,A */
      NODE_SET_AB,  /**< Root Node _is_ Face Node (A&B) */
      NODE_SET_T, /**< local nodes, not otherwise involved */
      NUM_NODE_SETS,
    } node_set;

    class OneColorRectangle : public Schedule
    {

/*
 * \page ocrmb_sched One-Color Rectangular Mesh Broadcast Schedule Design
 *
 * Torus/Mesh Rectangular communication schedule.
 * Supports broadcast and (broadcast portion of) reduce.
 *
 * Since individual phases are not resolved by the broadcast executor (only
 * the start phase is used, after which all steps of all phases are executed
 * out of sync with subsequent phases on other nodes), this schedule does not
 * separate phases internally. This allows for a more compressed data structure
 * and, thus, and smaller object size.
 *
 * Terms:
 * Rect Color = The starting axis for operations on the Rectangle; X, Y, or Z.
 * Face Node = The designated node at the surface of the rectangle that shares the
 * same Rect Color axis. The root node will never be the Face Node (i.e. the opposite
 * face is chosen in case the root node is on a face) - although, to support > 3 colors
 * this rule must be broken and handled.
 * Next Axis = current axis plus one in the sequence ... X -> Y -> Z -> X -> Y ...
 *
 *<TABLE BORDER>
 *<TR><TH>Destination Set (direction)</TH><TH>Description</TH></TR>
 *<TR><TD>X</TD><TD>All nodes along the Primary axis, a.k.a. Rectangle "color"</TD></TR>
 *<TR><TD>Y</TD><TD>All nodes along the Secondary axis, i.e. {X} + 1 in the sequence x, y, z, x, y, ...</TD></TR>
 *<TR><TD>Z</TD><TD>All nodes along the Tertiary axis, i.e. {X} + 2 in the sequence x, y, z, x, y, ...</TD></TR>
 *</TABLE>
 *<P>
 *<TABLE BORDER>
 *<TR><TH>Node Set</TH><TH>Description</TH></TR>
 *<TR><TD>A</TD><TD>The Root Node</TD></TR>
 *<TR><TD>B</TD><TD>The Root's Face Node</TD></TR>
 *<TR><TD>C</TD><TD>All Nodes on {X} with Root Node, excluding {A} and {B}</TD></TR>
 *<TR><TD>D</TD><TD>All Nodes on {Y} with Root's Face Node, excluding {B}</TD></TR>
 *<TR><TD>E</TD><TD>All Nodes on the Root Face, excluding {A}, {B}, {C}, {D}</TD></TR>
 *<TR><TD>F</TD><TD>All (the rest of the) Nodes in rectangle, excluding {A}, {B}, {C}, {D}, {E}</TD></TR>
 *<TR><TD>loc1</TD><TD>adjacent local core node #1</TD></TR>
 *<TR><TD>loc2</TD><TD>adjacent local core node #2</TD></TR>
 *<TR><TD>loc3</TD><TD>adjacent local core node #3</TD></TR>
 *
 *</TABLE>
 *
 *Broadcast:
 *<TABLE BORDER>
 *<TR><TH ROWSPAN=2>Phase</TH>
 *        <TH COLSPAN=6>Node Set</TH></TR>
 *<TR><TH>A</TH><TH>B</TH><TH>C</TH><TH>D</TH><TH>E</TH><TH>F</TH></TR>
 *<TR><TH>0</TH><TD>{X}</TD><TD>(recv A)</TD><TD>(recv A)</TD><TD></TD><TD></TD><TD></TD></TR>        <TR><TH>1</TH><TD>loc1</TD><TD>{Y}</TD><TD>loc1</TD><TD>(recv B)</TD><TD></TD><TD></TD></TR>
 *<TR><TH>2</TH><TD>loc2</TD><TD>{Z}</TD><TD>loc2</TD><TD>{Z}</TD><TD>(recv B|D)</TD><TD></TD></TR>
 *<TR><TH>3</TH><TD>loc3</TD><TD>loc1</TD><TD>loc3</TD><TD>{X}</TD><TD>{X}</TD><TD>(recv D|E)</TD></TR>
 *<TR><TH>4</TH><TD></TD><TD>loc2</TD><TD></TD><TD>loc1</TD><TD>loc1</TD><TD>loc1</TD></TR>
 *<TR><TH>5</TH><TD></TD><TD>loc3</TD><TD></TD><TD>loc2</TD><TD>loc2</TD><TD>loc2</TD></TR>
 *<TR><TH>6</TH><TD></TD><TD></TD><TD></TD><TD>loc3</TD><TD>loc3</TD><TD>loc3</TD></TR>
 *</TABLE>
 *
 *<P>
 *<HR>
 *<P>
 *Reduce:
 *<TABLE BORDER>
 *<TR><TH ROWSPAN=2>Phase</TH>
 *        <TH COLSPAN=6>Node Set</TH></TR>
 *<TR><TH>A</TH><TH>B</TH><TH>C</TH><TH>D</TH><TH>E</TH><TH>F</TH></TR>
 *<TR><TH>0</TH><TD>loc</TD><TD>loc</TD><TD>loc</TD><TD>loc</TD><TD>loc</TD><TD>loc</TD></TR>
 *<TR><TH>1</TH><TD></TD><TD></TD><TD></TD><TD>{X}root</TD><TD>{X}root</TD><TD>{X}</TD></TR>
 *<TR><TH>2</TH><TD></TD><TD>{Z}root</TD><TD></TD><TD>{Z}root</TD><TD>{Z}</TD><TD></TD></TR>
 *<TR><TH>3</TH><TD></TD><TD>{Y}root</TD><TD></TD><TD>{Y}</TD><TD></TD><TD></TD></TR>
 *<TR><TH>4</TH><TD>{X}root</TD><TD>{X}</TD><TD>{X}</TD><TD></TD><TD></TD><TD></TD></TR>
 *</TABLE>
 *
 * For rectangles "colored" XN_Y_Z, YN_Z_X, or ZN_X_Y, when the root node is on a face
 * plane, the columns "A" and "B" are effectively merged (as A and B are the same node).
 */

    protected:
      /**
       * \brief Structure for coordinates needed while creating the schedule.
       */
      struct _me_t
      {
        axii_t _my;   /**< My coords */
        axii_t _mx;   /**< Max geom rect coords (not size!) */
        axii_t _mn;   /**< Min geom rect coords */
      };

      /**
       * \brief Method to set the _startphase value
       * The first time it is called is when the value is set.
       * All subsequent calls do nothing.
       * \param[in] phase	The phase
       */
      inline void setPhase(unsigned phase)
      {
        XMI_assert_debug(phase != PHASE_NONE && phase < NUM_PHASES);
        if(_startsend == PHASE_NONE) _startsend = phase;
      }
      inline void setRecv(unsigned phase)
      {
        XMI_assert_debug(phase != PHASE_NONE && phase < NUM_PHASES);
        if(_startrecv == PHASE_NONE) _startrecv = phase;
      }

      /**
       * \brief Get the starting phase when I will first get data
       * \return	starting phase for this node
       */
      inline unsigned getStartPhase()
      {
        return _startsend;
      }

    public:

      /**
       * \brief Constructor for OneColorRectangle
       * Initialize everything to "empty".
       * \return	nothing
       */
      OneColorRectangle() : Schedule()
      {
        _color = NO_COLOR;
        _op = 0;
        _mapping = NULL;
        _rect = NULL;
        /* invalidate schedule */
        _startrecv = PHASE_NONE;
        _startsend = PHASE_NONE;
      }

      /**
       * \brief Initialize the one-color rectangular mesh schedule
       * Sets up an empty, no-op, schedule for 'mapping', 'color',
       * and 'rect'. Simply stores these values, as no other
       * initialization makes sense until root node is known.
       *
       * \param[in] mapping	CollectiveMapping is use on the partition
       * \param[in] color	Primary axis (color).
       * \param[in] rect	Rectangle descriptor
       * \return
       */
      inline OneColorRectangle(XMI_MAPPING_CLASS *mapping, Color color,
                               const Rectangle &rect) : Schedule()
      {
        _color = color;
        _rect = (axis_rect *)&rect;
        _op = 0;
        _mapping = mapping;
        /* invalidate any previous schedule */
        _startrecv = PHASE_NONE;
        _startsend = PHASE_NONE;
      }

      /**
       * \brief \e new operator for in-place construction of object
       *
       * Asserts that memory is cabable of containing the object.
       *
       * \param[in] size	Size of memory (\e addr) to contain new object
       * \param[in] addr	Memory to contain new object
       * \return	Pointer to new object
       */
//      inline void * operator new(size_t size, void *addr)
//      {
//        XMI_assert(size >= sizeof(OneColorRectangle));
//        return addr;
//      }

      /**
       * \brief \e delete operator for destruction of object
       *
       * \param[in] p	Pointer to object to destroy
       *
       * \note This is required to make "C" programs link successfully
       * with virtual destructors
       */
      void operator delete(void *p)
      {
        XMI_abort();
      }

      /*
       * Interface Functions
       */

      /**
       * \brief Get Source node phase list
       *
       * \param[in] phase	Phase for which to extract information
       * \param[out] srcpes	Array to hold source node(s)
       * \param[out] nsrc	Number of source nodes
       * \param[out] subtasks	Operation for receive
       * \return	nothing (else)
       */
      virtual void getSrcPeList(unsigned phase, unsigned *srcpes,
                                unsigned &nsrc, unsigned *subtasks=NULL) = 0;
      /**
       * \brief Get Destination node phase list
       *
       * \param[in] phase	Phase for which to extract information
       * \param[out] dstpes	Array to hold destination node(s)
       * \param[out] ndst	Number of destination nodes (and subtasks)
       * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
       * \return	nothing (else).
       */
      virtual void getDstPeList(unsigned phase, unsigned *dstpes,
                                unsigned &ndst, unsigned *subtasks) = 0;

      /*
       * Query Functions
       */

      /**
       * \brief	Get node-in-rectangle's role (node_set)
       * \return	Current node-in-rectangle's node set
       */
      inline node_set nodeSet()
      {
        return _ns;
      }

      /**
       * \brief	Get current rectangle's color
       * \return	Current rectangle's color (primary axis)
       */
      inline unsigned color()
      {
        return _color;
      }

      /**
       * \brief Get colors that make sense for this rectangle.
       *
       * \param[in] rect	The rectange in question
       * \param[out] ideal	The ideal number of colors
       * \param[out] max	The maximum number of colors (returned in colors)
       * \param[out] colors	(optional) Array of enum Color's usable on rect
       */

      static void getColors(const Rectangle &rect,
                            int &ideal, int &max, Color *colors = NULL) {
        int n = 0, x,y;
        unsigned dims[NUM_STD_AXIS];
        axis_rect *r = (axis_rect *)&rect;

        for (x = 0; x < NUM_STD_AXIS; ++x) {
          dims[x] = x;
        }

        /* List the dimensions in sorted order, via bubble sort */
        for (x = 0; x < NUM_STD_AXIS; ++x) {
          for (y = x+1; y < NUM_STD_AXIS; ++y)
            if (r->xs[dims[x]] > r->xs[dims[y]]) {
              unsigned tmp = dims[x];
              dims[x] = dims[y];
              dims[y] = tmp;
            }
        }

        /* Positive direction, "normal" colors. */
        for (x = 0; x < NUM_STD_AXIS; ++x) {
          if (r->xs[dims[x]] > 1) colors[n++] = MK_COLOR(dims[x],P_DIR);
        }
        ideal = n;
        /*
         * Negative direction, secondary colors.
         * These should be used cautiously, as they
         * result in overlapping traffic on the primary
         * axis (root sends twice over a given axis).
         */
        for (x = 0; x < NUM_STD_AXIS; ++x) {
          if (r->xs[dims[x]] > 1) colors[n++] = MK_COLOR(dims[x],N_DIR);
        }
        max = n;
        if (!n) {
          /*
           * The rectangle has no X, Y, or Z dimensions,
           * so just allow one, dummy, color.
           */
          colors[n++] = XP_Y_Z;
          ideal = max = n;
        }
      }


    protected:
      Color _color;   /** \brief color to use (pri axis) */
      unsigned _op;   /** \brief operation, BROADCAST_OP... */
      const axis_rect *_rect; /** \brief pointer to rectangle structure */
      XMI_MAPPING_CLASS *_mapping; /** \brief saved pointer to mapping
               * for this partition */
      unsigned _startrecv;  /** \brief Starting recv phase for this node */
      unsigned _startsend;  /** \brief Starting send phase for this node */
      unsigned _root; /** \brief Root node of collective */
      _me_t _me;  /** \brief Distilled info on node-in-rect */
      node_set _ns0, _ns; /** \brief NodeSet - role of node in rect */
      unsigned _pivot[NUM_AXIS];
      int _recv_axis; /** \brief Axis on which to recv in bcast (send in reduce) */

      inline void setPivotPoint(int axis, int coord)
      {
        _pivot[axis] = coord;
      }

      /**
       * \brief Determine the "node set" this node belongs to
       *
       * The node set represents nodes which all do the same work (steps).
       * Uses class variables which must have been previously set,
       * i.e. by constructor _AND_ init(). Called from init().
       *
       * \return	enum node_set value
       */
      inline node_set getRectNodeSet(int &is_T)
      {
        unsigned f; // face coords
        axii_t r; // root coords
        int x, y, z, d;

        is_T = 0;
        // extract coords of root node.
        rank2coord(_mapping, _root, r);

        /*
         * setup conditions for this rectangle/color/root
         * combination, relative to the current node.
         */
        x = GET_AXIS(_color);
        d = GET_DIR(_color);
        y = NEXT_STD_AXIS(x,1);
        z = NEXT_STD_AXIS(x,2);
        if(d == P_DIR)
        {
          f = (r[x] < _me._mx[x] ? _me._mx[x] : _me._mn[x]);
        }
        else
        {
          /* > 3 color support */
          f = (r[x] < _me._mx[x] ? _me._mn[x] : _me._mx[x]);
        }
        setPivotPoint(x, f); // root axis changes this
        setPivotPoint(y, r[y]);
        setPivotPoint(z, r[z]);
        setPivotPoint(XMI_T_DIM, r[XMI_T_DIM]);
        if(_root == _mapping->task())
        {
          _recv_axis = -1;  // no receives
          setPivotPoint(x, r[x]);
          if(f == r[x])
          {
            return NODE_SET_AB;
          }
          else
          {
            return NODE_SET_A;
          }
        }
        else
        {
          bool on_axis, on_perp, on_face;

          if(_mapping->torusgetcoord(XMI_T_DIM) != r[XMI_T_DIM])
          {
            _recv_axis = XMI_T_DIM;
            //return NODE_SET_T;
            is_T = 1;
          }
          on_face = (_me._my[x] == f);
          on_axis = (_me._my[y] == r[y] && _me._my[z] == r[z]);
          on_perp = (on_face && _me._my[z] == r[z]);

          if(on_axis)
          {
            /*
             * same axis as root. The (non-root) non-face
             * nodes will do only local core-to-core.
             */
            if(!is_T)
            {
              _recv_axis = x;
              setPivotPoint(x, r[x]);
            }
            if(on_face)
            {
              return NODE_SET_B;
            }
            else
            {
              return NODE_SET_C;
            }
          }
          else if(on_perp)
          {
            /*
             * perpendicular face axis (non-root and non-root-axis).
             */
            if(on_face)
            {
              if(!is_T) _recv_axis = y;
              return NODE_SET_D;
            }
          }
          else if(on_face)
          {
            /*
             * on face plane.
             */
            if(!is_T) _recv_axis = z;
            return NODE_SET_E;
          }
          else
          {
            /* everything else */
            if(!is_T) _recv_axis = x;
            return NODE_SET_F;
          }
        }
        /* NOTREACHED */
        return NODE_SET_T;
      }

      /**
       * \brief Initialize the Rectangle Schedule.
       *
       * The actual creation of the schedule is done in another routine, called here.
       * Future versions might select different schedule-building routines based on
       * properties of the rectangle and/or location of root node (and which operation
       * is selected).
       *
       * \param[in] root		Root node rank
       * \param[in] op		Collective operation (e.g. Broadcast)
       * \param[out] start		Starting phase for this node
       * \param[out] nphases		Number of phases for this node
       * \param[out] nmessages	Number of steps in each phase for this node
       *				(not used by executor).
       * \return	nothing (else)
       *
       * \refer ocrmb_sched
       */
      virtual void init(int root, int op, int &start, int &nphases,
                        int &nmessages)
      {
        int flag = 0;
        _op = (CollectiveOperation)op;
        _root = (unsigned)root;

        /* invalidate and irradicate any previous schedule */
        _startrecv = PHASE_NONE;
        _startsend = PHASE_NONE;

        _me._my[XMI_X_DIM] = _mapping->torusgetcoord(XMI_X_DIM);
        _me._my[XMI_Y_DIM] = _mapping->torusgetcoord(XMI_Y_DIM);
        _me._my[XMI_Z_DIM] = _mapping->torusgetcoord(XMI_Z_DIM);
        _me._my[XMI_T_DIM] = _mapping->torusgetcoord(XMI_T_DIM);
        for(unsigned x = 0; x < NUM_AXIS; ++x)
        {
          _me._mx[x] = _rect->x0[x] + _rect->xs[x] - 1;
          _me._mn[x] = _rect->x0[x];
        }
        _ns0 = getRectNodeSet(flag);
        _ns = (flag ? NODE_SET_T : _ns0);
      }

    }; /* OneColorRectangle */

    class OneColorRectBcastSched : public OneColorRectangle
    {
    protected:

      /**
       * \brief Initialize Phase for Mesh.
       *
       * Based on active axis (color), create necessary communication steps
       * that will result in all nodes on the axis having the data.
       *
       * The step may have up to two operations, depending on the location
       * of the node relative to the end-nodes of the mesh axis.
       *
       * A torus version of this routine might take advantage of the torus links and
       * perform only one line bcast operation. However, this mesh version will
       * function correctly in both torus and mesh configurations.
       *
       * \param[in] color	Axis to be used in this phase
       * \return	nothing
       *
       * \refer ocrmb_sched
       */
      inline void lineBcastStep(axii_t x, unsigned axis, unsigned pdir,
                                unsigned *dstpes, unsigned *subtasks,
                                unsigned &ndst)
      {
        if(_rect->xt[axis])  // full torus
        {
          if(_rect->xs[axis] > 1)
          {
            if(pdir == P_DIR)
            {
              x[axis] = (_me._my[axis] == _me._mn[axis] ?
                         _me._mx[axis] : _me._my[axis] - 1);
            }
            else // N_DIR
            {
              x[axis] = (_me._my[axis] == _me._mx[axis] ?
                         _me._mn[axis] : _me._my[axis] + 1);
            }
            dstpes[ndst] = coord2rank(_mapping, x);
            subtasks[ndst] = line_bcast[axis][pdir];
            ++ndst;
          }
        }
        else
        {
          if(_me._my[axis] < _me._mx[axis])
          {
            x[axis] = _me._mx[axis];
            dstpes[ndst] = coord2rank(_mapping, x);
            subtasks[ndst] = line_bcast[axis][P_DIR];
            ++ndst;
          }
          if(_me._my[axis] > _me._mn[axis])
          {
            x[axis] = _me._mn[axis];
            dstpes[ndst] = coord2rank(_mapping, x);
            subtasks[ndst] = line_bcast[axis][N_DIR];
            ++ndst;
          }
        }
        x[axis] = _me._my[axis];
      }

      /**
       * \brief Create a one-color rectangle broadcast schedule for a mesh.
       *
       * For N-color broadcasts multiple, independent, schedules are used.
       *
       * The root node's coordinates are computed and saved, and the face node
       * coordinate is chosen for the primary axis of the rectangle color.
       *
       * If the node's T-coord is not the same as root's then this node will
       * do no work (other than receive the data). In this case, _startphase is
       * PHASE_NONE and number of phases will be reported as zero.
       *
       * If the node is on the same T-plane as root, then determine starting phase
       * and create phase operations. At a minimum, a node will send data to local
       * cores (along T axis) - provided the partition mode specifies a T dimension
       * greater than 1.
       *
       * The face nodes are explicitely chosen to never be the root node, so if the
       * root node is on a face in any given axis, the face node will be the node
       * at the opposite end of that axis (opposite face).
       *
       * Notes on "6-color" (more than 3 color) support: If the rectangle color
       * is one of XN_Y_Z, YN_Z_X, or ZN_X_Y then the opposite face node is chosen.
       * This may result in the face node and the root node being the same, in which
       * case the root node does both roles and does broadcasts along all three axii.
       *
       * \refer ocrmb_sched
       *
       */

      /**
       * \brief initialize a one-color-rectangle broadcast schedule
       *
       * Only sets _startphase, rest is done in getXXXPeList().
       *
       */
      inline void initRectBcastSched(int &nsend)
      {
        int nsnd = 0;
        switch(_ns)
        {
        case NODE_SET_AB:
          nsnd += 2;
          /* FALLTHROUGH */
        case NODE_SET_A:
          setPhase(PHASE_ONE);
          nsnd += 1;
          nsnd += (_rect->xs[XMI_T_DIM] > 1);
          break;
        case NODE_SET_B:
          setRecv(PHASE_ONE);
          setPhase(PHASE_TWO);
          nsnd += 2;
          nsnd += (_rect->xs[XMI_T_DIM] > 1);
          break;
        case NODE_SET_C:
          setRecv(PHASE_ONE);
          nsnd += (_rect->xs[XMI_T_DIM] > 1);
          if(_rect->xs[XMI_T_DIM] > 1)
            setPhase(PHASE_TWO);
          break;
        case NODE_SET_D:
          setRecv(PHASE_TWO);
          setPhase(PHASE_THREE);
          nsnd += 2;
          nsnd += (_rect->xs[XMI_T_DIM] > 1);
          break;
        case NODE_SET_E:
          setRecv(PHASE_THREE);
          setPhase(PHASE_FOUR);
          nsnd += 1;
          nsnd += (_rect->xs[XMI_T_DIM] > 1);
          break;
        case NODE_SET_F:
          /*
           * If there are nodes along T, must ensure
           * they get the data. But, we all must receive
           * anyway, so just set phase.
           */
          nsnd += (_rect->xs[XMI_T_DIM] > 1);
          if(_rect->xs[XMI_T_DIM] > 1)
            setPhase(PHASE_FIVE);
          setRecv(PHASE_FOUR);
          break;
        case NODE_SET_T:
          /* receive only */
          switch(_ns0)
          {
          case NODE_SET_AB:
            setRecv(PHASE_FOUR);
            break;
          case NODE_SET_A:
            setRecv(PHASE_TWO);
            break;
          case NODE_SET_B:
            setRecv(PHASE_FOUR);
            break;
          case NODE_SET_C:
            setRecv(PHASE_TWO);
            break;
          case NODE_SET_D:
            setRecv(PHASE_FIVE);
            break;
          case NODE_SET_E:
            setRecv(PHASE_FIVE);
            break;
          case NODE_SET_F:
            setRecv(PHASE_FIVE);
            break;
          default:
            break;
          }
        default:
          break;
        }
        nsend = nsnd;
      }
    public:
      /**
       * \brief Constructor for Rectangle Broadcast Schedule
       *
       * Simply calls the One Color Rectangle constructor.
       */
      inline OneColorRectBcastSched(void) : OneColorRectangle()
      {
      }
      /**
       * \brief Constructor for Rectangle Broadcast Schedule
       *
       * Simply calls the One Color Rectangle constructor.
       *
       * \param[in] mapping	The mapping of the geometry
       * \param[in] color	The color to use for this schedule
       * \param[in] rect	The rectangle info for this geom
       */
      inline OneColorRectBcastSched(XMI_MAPPING_CLASS *mapping, Color color,
                                    const Rectangle &rect) :
      OneColorRectangle(mapping, color, rect)
      {
      }

      /**
       * \brief In-place constructor for Rectangle Broadcast Schedule
       *
       * Checks the memory object has room for the schedule object.
       *
       * \param[in] size	Size, in bytes, of the memory region
       * \param[in] addr	The memory region
       * \return	Address of memory region to use for object
       */
//      inline void * operator new(size_t size, void *addr)
//      {
//        XMI_assert(size >= sizeof(OneColorRectBcastSched));
//        return addr;
//      }
      /**
       * \brief object delete operator.
       *
       * Aborts since delete should not be used.
       *
       * \param[in] p	The address of the object
       */
      void operator delete(void *p)
      {
        XMI_abort();
      }

      /**
       * \brief Get Source node phase list
       *
       * \param[in] phase	Phase for which to extract information
       * \param[out] srcpes	Array to hold source node(s)
       * \param[out] nsrc	Number of source nodes
       * \param[out] subtasks	Operation for receive
       * \return	nothing (else)
       */
      virtual void getSrcPeList(unsigned phase, unsigned *srcpes,
                                unsigned &nsrc, unsigned *subtasks=NULL)
      {
        if(_recv_axis != -1 && phase == _startrecv)
        {
          axii_t m;
          memcpy(m, _me._my, sizeof(m));
          m[_recv_axis] = _pivot[_recv_axis];
          unsigned r = coord2rank(_mapping, m);
          srcpes[0] = r;
          if(subtasks != NULL)
          {
            if(_rect->xt[_recv_axis]) // full torus
            {
              int dir = GET_DIR(_color);
              if(_ns != NODE_SET_F)
              {
                dir = OPP_DIR(dir);
              }
              *subtasks = line_bcast[_recv_axis][dir];
            }
            else
            {
              *subtasks = r > _mapping->task() ?
                          line_bcast[_recv_axis][N_DIR] :
                          line_bcast[_recv_axis][P_DIR];
            }
          }
          nsrc = 1;
        }
        else
        {
          nsrc = 0;
        }
      }

      /**
       * \brief Get Destination node phase list
       *
       * Since we do not separate the phase steps by phase, we simply
       * return everything when the _startphase is called for. See we
       * only report 1 phase anyway, that should be the only call.
       *
       * We also have to create the implied local core sends.
       *
       * If _startphase is PHASE_NONE, we return zero (no) steps.
       *
       * \param[in] phase	Phase for which to extract information
       * \param[out] dstpes	Array to hold destination node(s)
       * \param[out] ndst	Number of destination nodes (and subtasks)
       * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
       * \return	nothing (else). Sets 'ndst' to 0 if the given phase does not
       *		exist for this schedule (i.e. if != _startphase).
       */
      virtual void getDstPeList(unsigned phase, unsigned *dstpes,
                                unsigned &ndst, unsigned *subtasks)
      {
        unsigned np = 0;
        int last = 0;
        if(_startsend != PHASE_NONE && phase >= _startsend)
        {
          axii_t x;
          memcpy(x, _me._my, sizeof(x));
          switch(_ns)
          {
          case NODE_SET_A:
            switch(phase - _startsend)
            {
            case 0:
              lineBcastStep(x, GET_AXIS(_color),
                            GET_DIR(_color),
                            dstpes, subtasks, np);
              break;
            case 1:
              last = 1;
              break;
            default:
              break;
            }
            break;
          case NODE_SET_AB:
            switch(phase - _startsend)
            {
            case 0:
              lineBcastStep(x, GET_AXIS(_color),
                            GET_DIR(_color),
                            dstpes, subtasks, np);
              break;
            case 1:
              lineBcastStep(x, NEXT_COLOR_AXIS(_color,1),
                            GET_DIR(_color),
                            dstpes, subtasks, np);
              break;
            case 2:
              lineBcastStep(x, NEXT_COLOR_AXIS(_color,2),
                            GET_DIR(_color),
                            dstpes, subtasks, np);
              break;
            case 3:
              last = 1;
              break;
            default:
              break;
            }
            break;
          case NODE_SET_B:
            switch(phase - _startsend)
            {
            case 0:
              lineBcastStep(x, NEXT_COLOR_AXIS(_color,1),
                            GET_DIR(_color),
                            dstpes, subtasks, np);
              break;
            case 1:
              lineBcastStep(x, NEXT_COLOR_AXIS(_color,2),
                            GET_DIR(_color),
                            dstpes, subtasks, np);
              break;
            case 2:
              last = 1;
              break;
            default:
              break;
            }
            break;
          case NODE_SET_D:
            switch(phase - _startsend)
            {
            case 0:
              lineBcastStep(x, NEXT_COLOR_AXIS(_color,2),
                            GET_DIR(_color),
                            dstpes, subtasks, np);
              break;
            case 1:
              lineBcastStep(x, NEXT_COLOR_AXIS(_color,3),
                            OPP_DIR(GET_DIR(_color)),
                            dstpes, subtasks, np);
              break;
            case 2:
              last = 1;
              break;
            default:
              break;
            }
            break;
          case NODE_SET_E:
            switch(phase - _startsend)
            {
            case 0:
              lineBcastStep(x, NEXT_COLOR_AXIS(_color,3),
                            OPP_DIR(GET_DIR(_color)),
                            dstpes, subtasks, np);
              break;
            case 1:
              last = 1;
              break;
            default:
              break;
            }
            break;
          case NODE_SET_C:
          case NODE_SET_F:
            /* end nodes, no line bcasts */
            last = 1;
            break;
          case NODE_SET_T:
            /* never sends anything */
          default:
            ndst = np; // zero
            return; /* no sends at all */
            break;
          }
          /* add phases for local core-to-core transfers */
          if(last)
          {
            for(unsigned dt = _me._mn[XMI_T_DIM];
               dt <= _me._mx[XMI_T_DIM]; ++dt)
            {
              if(dt == _me._my[XMI_T_DIM])
              {
                continue;
              }
              x[XMI_T_DIM] = dt;
              dstpes[np] = coord2rank(_mapping, x);
              subtasks[np] = XMI_PT_TO_PT_SUBTASK;
              ++np;
            }
            x[XMI_T_DIM] = _me._my[XMI_T_DIM];
          }
        }
        ndst = np;
      }

      /**
       * \brief Initialize the Rectangle Schedule.
       *
       * The actual creation of the schedule is done in another routine, called here.
       * Future versions might select different schedule-building routines based on
       * properties of the rectangle and/or location of root node (and which operation
       * is selected).
       *
       * \param[in] root		Root node rank
       * \param[in] op		Collective operation (e.g. Broadcast)
       * \param[out] start		Starting phase for this node
       * \param[out] nphases		Number of phases for this node
       * \param[out] nmessages	Number of steps in each phase for this node
       *				(not used by executor).
       * \return	nothing (else)
       *
       * \refer ocrmb_sched
       */
      inline void init(int root, int op, int &start, int &nphases, int &nmessages)
      {
        int nsend = 0;
        /* invalidate and irradicate any previous schedule */
        OneColorRectangle::init(root, op,
                                /* not used: */ start, nphases, nmessages);

        initRectBcastSched(nsend);

        if(_startrecv != PHASE_NONE && _startsend != PHASE_NONE)
        {
          start = _startrecv;
          nphases = _startsend - _startrecv + nsend;
        }
        else if(_startrecv != PHASE_NONE)
        {
          start = _startrecv;
          nphases = 1;
        }
        else if(_startsend != PHASE_NONE)
        {
          start = _startsend;
          nphases = nsend;
        }
        else
        {
          start = PHASE_NONE;
          nphases = 0;
        }
        nmessages = 1;
      }
    }; /* OneColorRectBcastSched */

/**
 * \brief Table to determine whether a rectangle phase is in use
 *
 * Maximum number of "used" (true) per node set <= NUM_AXIS...
 */
    static int _node_set_redu[NUM_NODE_SETS][NUM_PHASES] = {
      /* 1      2      3      4      5 */
/* NODE_SET_A */  { true, false, false, false,  true},
/* NODE_SET_B */  { true, false,  true,  true,  true},
/* NODE_SET_C */  { true, false, false, false,  true},
/* NODE_SET_D */  { true,  true,  true,  true, false},
/* NODE_SET_E */  { true,  true,  true, false, false},
/* NODE_SET_F */  { true,  true, false, false, false},
/* NODE_SET_AB */ { true, false,  true,  true,  true},
/* NODE_SET_T */  { true, false, false, false, false},
    };
      /** Allow different sub schedules (currently Binomial or Ring) */
      typedef enum
      {
        Binomial, Ring
      } Subschedule;


    template <class T_Sysdep, class T_Schedule>
    class OneColorRectRedSched : public OneColorRectangle
    {
    public:
    protected:
      Subschedule _subScheduleType;
#define MAX_SUB_SCHEDULE_SIZE sizeof(CCMI::Schedule::BinomialTreeSchedule<T_Sysdep>) > \
                              sizeof(CCMI::Schedule::RingSchedule<T_Sysdep>)?          \
                              sizeof(CCMI::Schedule::BinomialTreeSchedule<T_Sysdep>) : \
                              sizeof(CCMI::Schedule::RingSchedule<T_Sysdep>)

      /**
       * \brief Create a sub schedule based on the desired type
       *
       * \param[in] axis	Active axis to schedule
       * \return	nothing
       */
      void createSubSchedule(unsigned axis)
      {

        if(_subScheduleType == Binomial)
        {
          COMPILE_TIME_ASSERT(MAX_SUB_SCHEDULE_SIZE >= sizeof(BinomialTreeSchedule<T_Sysdep>));
          _subschedule[axis] = new (&_subschedule_storage[axis])
            BinomialTreeSchedule<T_Sysdep>(_me._my[axis], _me._mn[axis], _me._mx[axis]);
        }
        else
        {
          COMPILE_TIME_ASSERT(MAX_SUB_SCHEDULE_SIZE >= sizeof(RingSchedule<T_Sysdep>));
          _subschedule[axis] = new (&_subschedule_storage[axis])
            RingSchedule<T_Sysdep>(_me._my[axis], _me._mn[axis], _me._mx[axis]);
        }

      }
      /**
       * \brief Ask the sub schedule class how many max phases are possible
       *
       * \param[in] mapping	geometry mapping
       * \param[in] nranks	number of ranks on the axis
       * \return	number of phases
       */
      unsigned getMaxPhases(T_Sysdep *mapping, unsigned nranks)
      {
        // These are static functions so we need to class them based on type
        if(_subScheduleType == Binomial)
        {
          return BinomialTreeSchedule<T_Sysdep>::getMaxPhases(mapping,nranks);
        }
        else
        {
          return RingSchedule<T_Sysdep>::getMaxPhases(mapping, nranks);
        }
      }

      /** \brief array of sub-schedules, one for each rectangle line */
      T_Schedule _subschedule[NUM_AXIS];
      /** \brief storage for sub-schedules, one for each rectangle line */
      char _subschedule_storage[NUM_AXIS][MAX_SUB_SCHEDULE_SIZE];

      unsigned _baxis[NUM_PHASES];  /** \brief Axis used by bino sched */
      unsigned _startph[NUM_PHASES];  /** \brief start phase of bino sched */
      unsigned _numph[NUM_PHASES];  /** \brief num phases in bino sched */
      unsigned _maxph[NUM_PHASES];  /** \brief max num phases bino sched */
      unsigned _nbino;  /** \brief number of bino scheds used */
      unsigned _hiPhase;
      int _nmsg;    /** largest number of messages by bino scheds */

      /**
       * \brief Initialize a mesh phase operation list for a given axis
       *
       * \param[in] phase	Phase to which step belongs
       * \param[in] axis	Active axis to use for operations
       * \return	nothing
       */
      inline void lineReduStep(unsigned phase, unsigned axis, unsigned &maxph)
      {
        int start = 0, nph = 0, nr = 0, max = 0;
        bool used;

        if(_rect->xs[axis] <= 1)
        {
          return;
        }
        unsigned idx = _nbino++;
        used = _node_set_redu[_ns][phase];
        setPhase(phase);  // not needed?
        max = getMaxPhases(_mapping, _me._mx[axis] - _me._mn[axis] + 1);
        _baxis[idx] = axis;
        _maxph[idx] = max;    // relative to _firstph
        maxph += max;
        if(used)
        {
          XMI_assert(axis < NUM_AXIS);
          createSubSchedule(axis);
          _subschedule[axis]->init(_pivot[axis], REDUCE_OP, start, nph, nr);
        }
        _startph[idx] = start;
        _numph[idx] = nph;
        _nmsg += nr;
      }

      /**
       * \brief Translate global phase to a bino sched and relative phase
       *
       * \param[in] phase	Global phase number
       * \param[out] which	Index into _subschedule[] for schedule, i.e. axis
       * \return	Phase number relative to _subschedule[] schedule,
       *	or UNDEFINED_PHASE if non found.
       */
      inline unsigned xlatPhase(unsigned phase, unsigned &which)
      {
        unsigned i, p = phase;

        for(i = 0; i < _nbino; ++i)
        {
          if(p < _maxph[i])
          {
            if(p >= _startph[i] &&
               p < _startph[i] + _numph[i])
            {
              which = _baxis[i];
              return p;
            }
            else
            {
              break;
            }
          }
          p -= _maxph[i];
        }
        return XMI_UNDEFINED_PHASE;
      }


      /**
       * \brief Initialize a mesh rectangular broadcast schedule
       *
       * Uses color, previous set in _color, as starting point for schedule.
       *
       */
      inline void initRectReduSched(unsigned &maxph)
      {
        unsigned m = 0;

        _nbino = 0;
        _nmsg = 0;
        /*
         * If there are nodes along T, must ensure they get reduced.
         */
        /*
         * We init every phase from 0ne to our last, even if it
         * is not used (we do not create a schedule, though).
         * The reason for this is to ensure we have proper phase
         * offsets for use when calling the sub schedule(s),
         * and also when continuing with the rectangle broadcast.
         */
        lineReduStep(PHASE_ONE, XMI_T_DIM, m);
        lineReduStep(PHASE_TWO, NEXT_COLOR_AXIS(_color,3), m);
        lineReduStep(PHASE_THREE, NEXT_COLOR_AXIS(_color,2), m);
        lineReduStep(PHASE_FOUR, NEXT_COLOR_AXIS(_color,1), m);
        lineReduStep(PHASE_FIVE, GET_AXIS(_color), m);
        maxph = m;
      }

    public:
      /**
       * \brief Constructor for Rectangle Reduce Schedule
       *
       * Calls One Color Rectangle constructor and inits variables.
       */
      inline OneColorRectRedSched(Subschedule subschedule=Binomial) : OneColorRectangle(),
      _subScheduleType(subschedule),
      _nbino(0),
      _nmsg(0)
      {
      }
      /**
       * \brief Constructor for Rectangle Reduce Schedule
       *
       * Calls One Color Rectangle constructor and inits variables.
       *
       * \param[in] mapping	Torus CollectiveMapping for geometry
       * \param[in] color	Color of schedule
       * \param[in] rect	Rectangle of geom
       */
      inline OneColorRectRedSched(XMI_MAPPING_CLASS *mapping, Color color,
                                  const Rectangle &rect, Subschedule subschedule=Binomial) :
      OneColorRectangle(mapping, color, rect),
      _subScheduleType(subschedule),
      _nbino(0),
      _nmsg(0)
      {
      }

      /**
       * \brief In-place constructor for Rectangle Reduce Schedule
       *
       * Checks the memory object has room for the schedule object.
       *
       * \param[in] size	Size, in bytes, of the memory region
       * \param[in] addr	The memory region
       * \return	Address of memory region to use for object
       */
//      inline void * operator new(size_t size, void *addr)
//      {
//        XMI_assert(size >= sizeof(OneColorRectRedSched));
//        return addr;
//      }
      /**
       * \brief object delete operator.
       *
       * Aborts since delete should not be used.
       *
       * \param[in] p	The address of the object
       */
      void operator delete(void *p)
      {
        XMI_abort();
      }

      /**
       * \brief Get Source node phase list
       *
       * \param[in] phase	Phase for which to extract information
       * \param[out] srcpes	Array to hold source node(s)
       * \param[out] nsrc	Number of source nodes
       * \param[out] subtasks	Operation for receive
       * \return	nothing (else)
       */
      virtual void getSrcPeList(unsigned phase, unsigned *srcpes,
                                unsigned &nsrc, unsigned *subtasks=NULL)
      {
        unsigned axis, p;

        p = xlatPhase(phase, axis);
        if(p == XMI_UNDEFINED_PHASE)
        {
          nsrc = 0;
          return;
        }
        _subschedule[axis]->getSrcPeList(p, srcpes, nsrc, subtasks);
        if(nsrc)
        {
          unsigned i;
          axii_t x;
          /* now convert them all to ranks */
          memcpy(x, _me._my, sizeof(x));
          for(i = 0; i < nsrc; ++i)
          {
            x[axis] = srcpes[i];
            srcpes[i] = coord2rank(_mapping, x);
          }
        }
      }

      /**
       * \brief Get Destination node phase list
       *
       * \param[in] phase	Phase for which to extract information
       * \param[out] dstpes	Array to hold destination node(s)
       * \param[out] ndst	Number of destination nodes (and subtasks)
       * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
       * \return	nothing (else).
       */
      virtual void getDstPeList(unsigned phase, unsigned *dstpes,
                                unsigned &ndst, unsigned *subtasks)
      {
        unsigned axis, p;

        p = xlatPhase(phase, axis);
        if(p == XMI_UNDEFINED_PHASE)
        {
          ndst = 0;
          return;
        }
        _subschedule[axis]->getDstPeList(p, dstpes, ndst, subtasks);
        if(ndst)
        {
          unsigned i;
          axii_t x;
          /* now convert them all to ranks */
          memcpy(x, _me._my, sizeof(x));
          for(i = 0; i < ndst; ++i)
          {
            x[axis] = dstpes[i];
            dstpes[i] = coord2rank(_mapping, x);
          }
        }
      }

      /**
       * \brief Initialize the Rectangle Schedule.
       *
       * The actual creation of the schedule is done in another routine, called here.
       * Future versions might select different schedule-building routines based on
       * properties of the rectangle and/or location of root node (and which operation
       * is selected).
       *
       * \param[in] root		Root node rank
       * \param[in] op		Collective operation (e.g. Broadcast)
       * \param[out] start		Starting phase for this node
       * \param[out] nphases		Number of phases for this node
       * \param[out] nmessages	Number of steps in each phase for this node
       *				(not used by executor).
       * \return	nothing (else)
       *
       * \refer ocrmb_sched
       */
      inline void
      init(int root, int op, int &start, int &nphases, int &nmessages)
      {
        unsigned max = 0;

        /* invalidate and irradicate any previous schedule */
        OneColorRectangle::init(root, op,
                                /* not used: */ start, nphases, nmessages);

        initRectReduSched(max);
        _hiPhase = max;

        if(_startsend != PHASE_NONE && _nbino)
        {
          int st = 0;
          for(unsigned i = 0; i < _nbino; ++i)
          {
            if(_numph[i])
            {
              st += _startph[i];
              break;
            }
            st += _maxph[i];
          }
          start = st;
          nphases = max - st;
        }
        else
        {
          start = 0;
          nphases = 0;
        }
        nmessages = _nmsg;
      }

      /**
       * \brief Get upper bound on phase numbers used
       */
      int hiPhase(void)
      {
        return _hiPhase;
      }

    }; /* OneColorRectRedSched */
    template <class T_Sysdep, class T_Schedule>
    class OneColorRectAllredSched : public OneColorRectangle
    {
    protected:
      CCMI::Schedule::OneColorRectRedSched<T_Sysdep, T_Schedule> _reduce;
      CCMI::Schedule::OneColorRectBcastSched _bcast;
      unsigned _midphase;

    public:
      /**
       * \brief Constructor for Rectangle Allreduce schedule
       *
       * Construct rectangle reduce and broadcast schedules.
       *
       * \param[in] mapping	Torus mapping for geometry
       * \param[in] color	Color of schedule
       * \param[in] rect	Rectangle of geom
       */
      inline OneColorRectAllredSched(XMI_MAPPING_CLASS *mapping,
                                     Color color, const Rectangle &rect,
                                     Subschedule subschedule=Binomial) :
      OneColorRectangle(mapping, color, rect)
      {
        COMPILE_TIME_ASSERT(sizeof(_reduce) >= sizeof(OneColorRectRedSched<T_Sysdep, T_Schedule>));
        new (&_reduce) OneColorRectRedSched<T_Sysdep, T_Schedule>(mapping, color, rect, subschedule);
        COMPILE_TIME_ASSERT(sizeof(_bcast) >= sizeof(OneColorRectBcastSched));
        new (&_bcast)  OneColorRectBcastSched(mapping, color, rect);
        _midphase = PHASE_NONE;

//        =CCMI::Schedule::OneColorRectRedSched::Binomial
      }

      /**
       * \brief Get Source node phase list
       *
       * Call the appropriate rectangle schedule to get the list.
       *
       * \param[in] phase	Phase for which to extract information
       * \param[out] srcpes	Array to hold source node(s)
       * \param[out] nsrc	Number of source nodes
       * \param[out] subtasks	Operation for receive
       * \return	nothing (else)
       */
      virtual void getSrcPeList(unsigned phase, unsigned *srcpes,
                                unsigned &nsrc, unsigned *subtasks=NULL)
      {
        if(phase < _midphase)
        {
          _reduce.getSrcPeList(phase, srcpes, nsrc, subtasks);
        }
        else
        {
          _bcast.getSrcPeList(phase - _midphase, srcpes, nsrc, subtasks);
        }
      }

      /**
       * \brief Get Destination node phase list
       *
       * Call the appropriate rectangle schedule to get the list.
       *
       * \param[in] phase	Phase for which to extract information
       * \param[out] dstpes	Array to hold destination node(s)
       * \param[out] ndst	Number of destination nodes (and subtasks)
       * \param[out] subtask	Array to hold subtasks (operation, e.g. LINE_BCAST_XM)
       * \return	nothing (else).
       */
      virtual void getDstPeList(unsigned phase, unsigned *dstpes,
                                unsigned &ndst, unsigned *subtasks)
      {
        if(phase < _midphase)
        {
          _reduce.getDstPeList(phase, dstpes, ndst, subtasks);
        }
        else
        {
          _bcast.getDstPeList(phase - _midphase, dstpes, ndst, subtasks);
        }
      }

      /**
       * \brief Initialize the Rectangle Schedule.
       *
       * The actual creation of the schedule is done in another routine, called here.
       * Future versions might select different schedule-building routines based on
       * properties of the rectangle and/or location of root node (and which operation
       * is selected).
       *
       * \param[in] root		Root node rank
       * \param[in] op		Collective operation (e.g. Broadcast)
       * \param[out] start		Starting phase for this node
       * \param[out] nphases		Number of phases for this node
       * \param[out] nmessages	Number of steps in each phase for this node
       *				(not used by executor).
       * \return	nothing (else)
       *
       * \refer ocrmb_sched
       */
      inline void
      init(int root, int op, int &start, int &nphases, int &nmessages)
      {
        unsigned r;
        int str, npr, nmr;
        int stb, npb, nmb;

        XMI_assert(op == ALLREDUCE_OP);
        // We can pick our own root node...
        //r = coord2rank(_mapping, _rect->x0);
        unsigned coords[XMI_TORUS_NDIMS];
        XMI_COPY_COORDS(coords, _rect->x0);
        coords[XMI_T_DIM] += (_color % _rect->xs[XMI_T_DIM]);
        _mapping->global2task((axis_array_t &)coords, (size_t)r);

        CCMI::Schedule::OneColorRectangle::init(r, op,
                                                /* not used: */ start, nphases, nmessages);

        _reduce.init(r, op, str, npr, nmr);
        _bcast.init(r, op, stb, npb, nmb);

        if((unsigned)str == PHASE_NONE)
        {
          _midphase = 0;
          start = stb;
          if((unsigned)stb == PHASE_NONE)
          {
            nphases = 0;
          }
          else
          {
            nphases = npb;
          }
        }
        else
        {
          _midphase = _reduce.hiPhase();
          start = str;
          if((unsigned)stb == PHASE_NONE)
          {
            nphases = npr;
          }
          else
          {
            nphases = (_midphase + stb + npb) - str;
          }
        }
        nmessages = nmr + nmb; // max recvs we might do
      }
    }; /* OneColorRectAllredSched */


  }; /* Schedule */
} /* CCMI */

#endif /* !__rectangle_schedule__ */
