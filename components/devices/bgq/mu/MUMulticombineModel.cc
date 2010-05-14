/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file components/devices/bgq/mu/MUMulticombineModel.cc
 * \brief ???
 */
#include "components/devices/bgq/mu/MUMulticombineModel.h"

#undef TRACE
#define TRACE(x) //fprintf x

#if 0  // extra debug - trace the op/dt strings
#define TRACE_STRINGS 1
  static const char* mu_op_string(uint8_t index);
  static const char* dt_string(pami_dt index);
  static const char* op_string(pami_op index);
#else
  #define mu_op_string(x) "untraced"
  #define op_string(x) "untraced"
  #define dt_string(x) "untraced"
#endif

const bool   PAMI::Device::MU::MUMulticombineModel::multicombine_model_op_support(pami_dt dt, pami_op op)
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
  TRACE((stderr, "MUMulticombineModel::multicombine_model_op_support(%d, %d) = %s\n", dt, op, support[dt][op]?"true":"false"));
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
  TRACE((stderr, "MUMulticombineModel::mu_op(%d, %d) = %#X, %s->%s\n", dt, op, mu_op_table[dt][op],op_string(op),mu_op_string((mu_op_table[dt][op])>>4)));
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
  TRACE((stderr, "MUMulticombineModel::mu_size(%d) = %zu %s\n", dt, mu_size_table[dt], dt_string(dt)));
  PAMI_assert_debugf(dt_check[dt] == dt,"dt_check[dt] %u == dt %u\n",dt_check[dt],dt);
  return(mu_size_table[dt]);
}

/// \see MUMulticombineModel
PAMI::Device::MU::MUMulticombineModel::MUMulticombineModel (MUCollDevice & device, pami_result_t &status):
Interface::MulticombineModel < MUMulticombineModel,MUCollDevice, sizeof(mu_multicombine_statedata_t) > (device, status),
_device (device),
_wrapper_model (&_desc_model)
{
  COMPILE_TIME_ASSERT(MUCollDevice::message_metadata_size >= sizeof(MUMulticombineModel::metadata_t));
  TRACE((stderr, "<%p>:MUMulticombineModel::ctor\n", this));
/// \see MUSPI_BaseDescriptorInfoFields_t
  MUSPI_BaseDescriptorInfoFields_t base =
  {

    Pre_Fetch_Only   :
    MUHWI_DESCRIPTOR_PRE_FETCH_ONLY_NO,

    Payload_Address  :
    0,

    Message_Length   :
    0,

    Torus_FIFO_Map   :
    MUHWI_DESCRIPTOR_TORUS_FIFO_MAP_CUSER,

    Dest :
    {

      Destination :

      { Destination :
        0}
    }
  };

/// \see MUSPI_CollectiveDescriptorInfoFields_t
  MUSPI_CollectiveDescriptorInfoFields_t coll =
  {

    Op_Code :
    MUHWI_COLLECTIVE_OP_CODE_OR,/// \todo not true? ignored for MUHWI_COLLECTIVE_TYPE_BROADCAST

    Word_Length:
    4,    /// \todo not true? ignored for MUHWI_COLLECTIVE_TYPE_BROADCAST

    Class_Route:
    0,    /// \todo global class route always 2 (arbitrary see MUCollDevice)?

    Misc:
    MUHWI_PACKET_VIRTUAL_CHANNEL_USER_COMM_WORLD |
    MUHWI_COLLECTIVE_TYPE_ALLREDUCE,

    Skip       :
    0
  };

/// \see MUSPI_MemoryFIFODescriptorInfoFields_t
  MUSPI_MemoryFIFODescriptorInfoFields_t memfifo =
  {

    Rec_FIFO_Id    :
    0,

    Rec_Put_Offset :
    0,

    Interrupt      :
    MUHWI_DESCRIPTOR_DO_NOT_INTERRUPT_ON_PACKET_ARRIVAL,

    SoftwareBit    :
    0,

    SoftwareBytes  :
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  };

  _desc_model.setBaseFields (&base);
  _desc_model.setCollectiveFields(&coll);
  _desc_model.setMemoryFIFOFields (&memfifo);

  // Use our rank/addr to set our master reception fifo
  pami_task_t target_rank = __global.mapping.task();
  size_t addr[BGQ_TDIMS + BGQ_LDIMS];
  TRACE((stderr, "<%p>:MUMulticombineModel::ctor .. %p->getRecFifoIdForDescriptor() target_rank %zu\n", this, &_device, (size_t)target_rank));
  __global.mapping.task2global ((pami_task_t)target_rank, addr);
  TRACE((stderr, "<%p>:MUMulticombineModel::ctor .. %p->getRecFifoIdForDescriptor(%zu) target_rank %zu\n", this, &_device, addr[5], (size_t)target_rank));
  /// \todo Assuming p is the recv grp id?
  uint32_t recFifoId = _device.getCollRecFifoIdForDescriptor(addr[5]);;

  TRACE((stderr, "<%p>:MUMulticombineModel::ctor .. recFifoId %d\n", this, recFifoId));
  _desc_model.setRecFIFOId (recFifoId);

  MemoryFifoPacketHeader * hdr = (MemoryFifoPacketHeader *) & _desc_model.PacketHeader;

  // Register the dispatch function.
  bool success =
  _device.registerPacketHandler (0x00AA,
                                 dispatch,
                                 this,
                                 hdr->dev.dispatch_id);
  TRACE((stderr, "<%p>:MUMulticastModel::registerMcastRecvFunction_impl dispatch_id = %#X, success = %d\n", this, hdr->dev.dispatch_id, (unsigned)success));
  DUMP_DESCRIPTOR("MUMultisyncModel::ctor", &_desc_model);

  PAMI_assert(success);

};

PAMI::Device::MU::MUMulticombineModel::~MUMulticombineModel ()
{
};

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
