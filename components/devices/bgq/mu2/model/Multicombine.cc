/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/Multicombine.cc
 * \brief ???
 */
#include "components/devices/bgq/mu/Multicombine.h"

#undef TRACE
#define TRACE(x) fprintf x

#if 1  // extra debug - trace the op/dt strings
#define TRACE_STRINGS 1
  static const char* mu_op_string(uint8_t index);
  static const char* dt_string(pami_dt index);
  static const char* op_string(pami_op index);
  #undef TRACE
  #define TRACE(x) fprintf x
#else
  #define mu_op_string(x) "untraced"
  #define op_string(x) "untraced"
  #define dt_string(x) "untraced"
#endif

const bool   PAMI::Device::MU::Multicombine::multicombine_model_op_support(pami_dt dt, pami_op op)
{
  const bool support[PAMI_DT_COUNT][PAMI_OP_COUNT] =
  {
//  PAMI_UNDEFINED_OP, PAMI_NOOP, PAMI_MAX, PAMI_MIN, PAMI_SUM, PAMI_PROD, PAMI_LAND, PAMI_LOR, PAMI_LXOR, PAMI_BAND, PAMI_BOR, PAMI_BXOR, PAMI_MAXLOC, PAMI_MINLOC, PAMI_USERDEFINED_OP,
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_UNDEFINED_DT
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_SIGNED_CHAR
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_UNSIGNED_CHAR
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_SIGNED_SHORT
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_UNSIGNED_SHORT
    {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_SIGNED_INT
    {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_UNSIGNED_INT
    {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_SIGNED_LONG_LONG
    {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_UNSIGNED_LONG_LONG
    {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_FLOAT
    {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_DOUBLE
    {false,            false,     true,     true,     true,     false,     true,      true,     true,      true,      true,     true,      false,       false,       false},//PAMI_LONG_DOUBLE
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOGICAL
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_SINGLE_COMPLEX
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_DOUBLE_COMPLEX
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_2INT
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_SHORT_INT
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_FLOAT_INT
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_DOUBLE_INT
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_2FLOAT
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false},//PAMI_LOC_2DOUBLE
    {false,            false,     false,    false,    false,    false,     false,     false,    false,     false,     false,    false,     false,       false,       false} //PAMI_USERDEFINED_DT
  };
  TRACE((stderr, "Multicombine::multicombine_model_op_support(%d, %d) = %s\n", dt, op, support[dt][op]?"true":"false"));
  return(support[dt][op]);
}

const uint8_t   PAMI::Device::MU::mu_op(pami_dt dt, pami_op op)
{
  const pami_op op_check[PAMI_OP_COUNT] =
  {
    PAMI_UNDEFINED_OP,    // PAMI_UNDEFINED_OP
    PAMI_NOOP,            // PAMI_NOOP
    PAMI_MAX,             // PAMI_MAX
    PAMI_MIN,             // PAMI_MIN
    PAMI_SUM,             // PAMI_SUM
    PAMI_PROD,            // PAMI_PROD
    PAMI_LAND,            // PAMI_LAND
    PAMI_LOR,             // PAMI_LOR
    PAMI_LXOR,            // PAMI_LXOR
    PAMI_BAND,            // PAMI_BAND
    PAMI_BOR,             // PAMI_BOR
    PAMI_BXOR,            // PAMI_BXOR
    PAMI_MAXLOC,          // PAMI_MAXLOC
    PAMI_MINLOC,          // PAMI_MINLOC
    PAMI_USERDEFINED_OP   // PAMI_USERDEFINED_OP
  };


  // The MU opcode - 0xF0 is invalid
  const uint8_t mu_op_table[PAMI_DT_COUNT][PAMI_OP_COUNT] =
  {
//  PAMI_UNDEFINED_OP PAMI_NOOP, PAMI_MAX,                                        PAMI_MIN,                                        PAMI_SUM,                                        PAMI_PROD, PAMI_LAND,                         PAMI_LOR,                        PAMI_LXOR,                         PAMI_BAND,                         PAMI_BOR,                        PAMI_BXOR,                         PAMI_MAXLOC, PAMI_MINLOC, PAMI_USERDEFINED_OP,
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_UNDEFINED_DT
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_SIGNED_CHAR
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_UNSIGNED_CHAR
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_SIGNED_SHORT
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_UNSIGNED_SHORT
    {0xF0,            0xF0,      MUHWI_COLLECTIVE_OP_CODE_SIGNED_MAX        ,     MUHWI_COLLECTIVE_OP_CODE_SIGNED_MIN        ,     MUHWI_COLLECTIVE_OP_CODE_SIGNED_ADD        ,     0xF0,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      0xF0,        0xF0,        0xF0},//PAMI_SIGNED_INT
    {0xF0,            0xF0,      MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_MAX      ,     MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_MIN      ,     MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_ADD      ,     0xF0,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      0xF0,        0xF0,        0xF0},//PAMI_UNSIGNED_INT
    {0xF0,            0xF0,      MUHWI_COLLECTIVE_OP_CODE_SIGNED_MAX        ,     MUHWI_COLLECTIVE_OP_CODE_SIGNED_MIN        ,     MUHWI_COLLECTIVE_OP_CODE_SIGNED_ADD        ,     0xF0,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      0xF0,        0xF0,        0xF0},//PAMI_SIGNED_LONG_LONG
    {0xF0,            0xF0,      MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_MAX      ,     MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_MIN      ,     MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_ADD      ,     0xF0,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      0xF0,        0xF0,        0xF0},//PAMI_UNSIGNED_LONG_LONG
    {0xF0,            0xF0,      MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MAX,     MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MIN,     MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_ADD,     0xF0,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      0xF0,        0xF0,        0xF0},//PAMI_FLOAT
    {0xF0,            0xF0,      MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MAX,     MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MIN,     MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_ADD,     0xF0,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      0xF0,        0xF0,        0xF0},//PAMI_DOUBLE
    {0xF0,            0xF0,      MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MAX,     MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MIN,     MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_ADD,     0xF0,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      MUHWI_COLLECTIVE_OP_CODE_AND,      MUHWI_COLLECTIVE_OP_CODE_OR,     MUHWI_COLLECTIVE_OP_CODE_XOR,      0xF0,        0xF0,        0xF0},//PAMI_LONG_DOUBLE
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_LOGICAL
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_SINGLE_COMPLEX
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_DOUBLE_COMPLEX
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_LOC_2INT
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_LOC_SHORT_INT
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_LOC_FLOAT_INT
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_LOC_DOUBLE_INT
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_LOC_2FLOAT
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0},//PAMI_LOC_2DOUBLE
    {0xF0,            0xF0,      0xF0                                       ,     0xF0                                       ,     0xF0                                       ,     0xF0,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0                        ,      0xF0                       ,     0xF0                        ,      0xF0,        0xF0,        0xF0} //PAMI_USERDEFINED_DT
  };
  TRACE((stderr, "Multicombine::mu_op(%d, %d) = %#X, %s->%s\n", dt, op, mu_op_table[dt][op],op_string(op),mu_op_string((mu_op_table[dt][op])>>4)));
  PAMI_assert_debugf(op_check[op] == op,"op_check[op] %u == op %u\n",op_check[op],op);
  return(mu_op_table[dt][op]);
}
const size_t   PAMI::Device::MU::mu_size(pami_dt dt)
{

  const pami_dt dt_check[PAMI_DT_COUNT] =
  {
    PAMI_UNDEFINED_DT,       //PAMI_UNDEFINED_DT
    PAMI_SIGNED_CHAR,        //PAMI_SIGNED_CHAR
    PAMI_UNSIGNED_CHAR,      //PAMI_UNSIGNED_CHAR
    PAMI_SIGNED_SHORT,       //PAMI_SIGNED_SHORT
    PAMI_UNSIGNED_SHORT,     //PAMI_UNSIGNED_SHORT
    PAMI_SIGNED_INT,         //PAMI_SIGNED_INT
    PAMI_UNSIGNED_INT,       //PAMI_UNSIGNED_INT
    PAMI_SIGNED_LONG_LONG,   //PAMI_SIGNED_LONG_LONG
    PAMI_UNSIGNED_LONG_LONG, //PAMI_UNSIGNED_LONG_LONG
    PAMI_FLOAT,              //PAMI_FLOAT
    PAMI_DOUBLE,             //PAMI_DOUBLE
    PAMI_LONG_DOUBLE,        //PAMI_LONG_DOUBLE
    PAMI_LOGICAL,            //PAMI_LOGICAL
    PAMI_SINGLE_COMPLEX,     //PAMI_SINGLE_COMPLEX
    PAMI_DOUBLE_COMPLEX,     //PAMI_DOUBLE_COMPLEX
    PAMI_LOC_2INT,           //PAMI_LOC_2INT
    PAMI_LOC_SHORT_INT,      //PAMI_LOC_SHORT_INT
    PAMI_LOC_FLOAT_INT,      //PAMI_LOC_FLOAT_INT
    PAMI_LOC_DOUBLE_INT,     //PAMI_LOC_DOUBLE_INT
    PAMI_LOC_2FLOAT,         //PAMI_LOC_2FLOAT
    PAMI_LOC_2DOUBLE,        //PAMI_LOC_2DOUBLE
    PAMI_USERDEFINED_DT      //PAMI_USERDEFINED_DT
  };
  const size_t mu_size_table[PAMI_DT_COUNT] =
  {
    -1,                         //PAMI_UNDEFINED_DT
    sizeof(signed char),        //PAMI_SIGNED_CHAR
    sizeof(unsigned char),      //PAMI_UNSIGNED_CHAR
    sizeof(signed short),       //PAMI_SIGNED_SHORT
    sizeof(unsigned short),     //PAMI_UNSIGNED_SHORT
    sizeof(signed int),         //PAMI_SIGNED_INT
    sizeof(unsigned int),       //PAMI_UNSIGNED_INT
    sizeof(signed long long),   //PAMI_SIGNED_LONG_LONG
    sizeof(unsigned long long), //PAMI_UNSIGNED_LONG_LONG
    sizeof(float),              //PAMI_FLOAT
    sizeof(double),             //PAMI_DOUBLE
    sizeof(long double),        //PAMI_LONG_DOUBLE
    sizeof(bool),               //PAMI_LOGICAL
    -1,                         //PAMI_SINGLE_COMPLEX
    -1,                         //PAMI_DOUBLE_COMPLEX
    -1,                         //PAMI_LOC_2INT
    -1,                         //PAMI_LOC_SHORT_INT
    -1,                         //PAMI_LOC_FLOAT_INT
    -1,                         //PAMI_LOC_DOUBLE_INT
    -1,                         //PAMI_LOC_2FLOAT
    -1,                         //PAMI_LOC_2DOUBLE
    -1                          //PAMI_USERDEFINED_DT
  };
  TRACE((stderr, "Multicombine::mu_size(%d) = %zu %s\n", dt, mu_size_table[dt], dt_string(dt)));
  PAMI_assert_debugf(dt_check[dt] == dt,"dt_check[dt] %u == dt %u\n",dt_check[dt],dt);
  return(mu_size_table[dt]);
}


#ifdef TRACE_STRINGS
// Extra debug trace the strings
static const char* mu_op_string(uint8_t index)
{
  const char* string[] =
  {
    "MUHWI_COLLECTIVE_OP_CODE_AND                ", //MUHWI_COLLECTIVE_OP_CODE_AND
    "MUHWI_COLLECTIVE_OP_CODE_OR                 ", //MUHWI_COLLECTIVE_OP_CODE_OR
    "MUHWI_COLLECTIVE_OP_CODE_XOR                ", //MUHWI_COLLECTIVE_OP_CODE_XOR
    "INVALID                                     ", // invalid
    "MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_ADD       ", //MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_ADD
    "MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_MIN       ", //MUHWI_COLLECTIVE_OP_CODE_UNSIGNED_MIN
    "MUHWI_COLLECTIVE_OP_CODE_UNSIGNED MAX       ", //MUHWI_COLLECTIVE_OP_CODE_UNSIGNED MAX
    "INVALID                                     ", // invalid
    "MUHWI_COLLECTIVE_OP_CODE_SIGNED_ADD         ", //MUHWI_COLLECTIVE_OP_CODE_SIGNED_ADD
    "MUHWI_COLLECTIVE_OP_CODE_SIGNED_MIN         ", //MUHWI_COLLECTIVE_OP_CODE_SIGNED_MIN
    "MUHWI_COLLECTIVE_OP_CODE_SIGNED_MAX         ", //MUHWI_COLLECTIVE_OP_CODE_SIGNED_MAX
    "INVALID                                     ", // invalid
    "MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_ADD ", //MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_ADD
    "MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MIN ", //MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MIN
    "MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MAX ", //MUHWI_COLLECTIVE_OP_CODE_FLOATING_POINT_MAX
    "INVALID                                     "  // invalid
  };
  return string[index];
}
static const char* dt_string(pami_dt index)
{
  const char* string[PAMI_DT_COUNT] =
  {
    "PAMI_UNDEFINED_DT",       //PAMI_UNDEFINED_DT
    "PAMI_SIGNED_CHAR",        //PAMI_SIGNED_CHAR
    "PAMI_UNSIGNED_CHAR",      //PAMI_UNSIGNED_CHAR
    "PAMI_SIGNED_SHORT",       //PAMI_SIGNED_SHORT
    "PAMI_UNSIGNED_SHORT",     //PAMI_UNSIGNED_SHORT
    "PAMI_SIGNED_INT",         //PAMI_SIGNED_INT
    "PAMI_UNSIGNED_INT",       //PAMI_UNSIGNED_INT
    "PAMI_SIGNED_LONG_LONG",   //PAMI_SIGNED_LONG_LONG
    "PAMI_UNSIGNED_LONG_LONG", //PAMI_UNSIGNED_LONG_LONG
    "PAMI_FLOAT",              //PAMI_FLOAT
    "PAMI_DOUBLE",             //PAMI_DOUBLE
    "PAMI_LONG_DOUBLE",        //PAMI_LONG_DOUBLE
    "PAMI_LOGICAL",            //PAMI_LOGICAL
    "PAMI_SINGLE_COMPLEX",     //PAMI_SINGLE_COMPLEX
    "PAMI_DOUBLE_COMPLEX",     //PAMI_DOUBLE_COMPLEX
    "PAMI_LOC_2INT",           //PAMI_LOC_2INT
    "PAMI_LOC_SHORT_INT",      //PAMI_LOC_SHORT_INT
    "PAMI_LOC_FLOAT_INT",      //PAMI_LOC_FLOAT_INT
    "PAMI_LOC_DOUBLE_INT",     //PAMI_LOC_DOUBLE_INT
    "PAMI_LOC_2FLOAT",         //PAMI_LOC_2FLOAT
    "PAMI_LOC_2DOUBLE",        //PAMI_LOC_2DOUBLE
    "PAMI_USERDEFINED_DT"      //PAMI_USERDEFINED_DT
  };
  return string[index];
}
static const char* op_string(pami_op index)
{
  const char* string[PAMI_OP_COUNT] =
  {
    "PAMI_UNDEFINED_OP",    // PAMI_UNDEFINED_OP
    "PAMI_NOOP",            // PAMI_NOOP
    "PAMI_MAX",             // PAMI_MAX
    "PAMI_MIN",             // PAMI_MIN
    "PAMI_SUM",             // PAMI_SUM
    "PAMI_PROD",            // PAMI_PROD
    "PAMI_LAND",            // PAMI_LAND
    "PAMI_LOR",             // PAMI_LOR
    "PAMI_LXOR",            // PAMI_LXOR
    "PAMI_BAND",            // PAMI_BAND
    "PAMI_BOR",             // PAMI_BOR
    "PAMI_BXOR",            // PAMI_BXOR
    "PAMI_MAXLOC",          // PAMI_MAXLOC
    "PAMI_MINLOC",          // PAMI_MINLOC
    "PAMI_USERDEFINED_OP"   // PAMI_USERDEFINED_OP
  };
  return string[index];
}
#endif

#undef TRACE
//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
