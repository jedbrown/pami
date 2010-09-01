///
/// \file api/c/pami_type.cc
/// \brief ???
///
#include "sys/pami.h"
#include "common/type/TypeMachine.h"
using namespace PAMI::Type;

#ifndef RETURN_ERR_PAMI
#define RETURN_ERR_PAMI(code, ...) return (code)
#endif

pami_type_t        PAMI_BYTE = new TypeContig;
pami_data_function PAMI_DATA_COPY = NULL;

pami_result_t PAMI_Type_create (pami_type_t * type)
{
    TypeCode * type_obj;
    try {
        type_obj = new TypeCode();
    } catch (std::bad_alloc) {
        RETURN_ERR_PAMI(PAMI_ENOMEM, "Out of memory.\n");
    }
    type_obj->AcquireReference();
    *type = (pami_type_t)type_obj;
    return PAMI_SUCCESS;
}

pami_result_t PAMI_Type_add_simple (pami_type_t type,
                                    size_t     bytes,
                                    size_t     offset,
                                    size_t     count,
                                    size_t     stride)
{
    TypeCode * type_obj = (TypeCode *)type;
    if (type_obj->IsCompleted()) {
        RETURN_ERR_PAMI(PAMI_INVAL, "Modifying a completed type.\n");
    }

    try {
        type_obj->AddShift(offset);
        type_obj->AddSimple(bytes, stride, count);
    } catch (std::bad_alloc) {
        RETURN_ERR_PAMI(PAMI_ENOMEM, "Out of memory.\n");
    }
    return PAMI_SUCCESS;
}

pami_result_t PAMI_Type_add_typed (pami_type_t type,
                                   pami_type_t subtype,
                                   size_t     offset,
                                   size_t     count,
                                   size_t     stride)
{
    TypeCode * type_obj = (TypeCode *)type;
    if (type_obj->IsCompleted()) {
        RETURN_ERR_PAMI(PAMI_INVAL, "Modifying a completed type.\n");
    }

    TypeCode * subtype_obj = (TypeCode *)subtype;
    if (!subtype_obj->IsCompleted()) {
        RETURN_ERR_PAMI(PAMI_INVAL, "Adding an incomplete subtype.\n");
    }

    try {
        type_obj->AddShift(offset);
        type_obj->AddTyped(subtype_obj, stride, count);
    } catch (std::bad_alloc) {
        RETURN_ERR_PAMI(PAMI_ENOMEM, "Out of memory.\n");
    }
    return PAMI_SUCCESS;
}

pami_result_t PAMI_Type_complete (pami_type_t type, size_t atom_size)
{
    TypeCode * type_obj = (TypeCode *)type;
    if (type_obj->IsCompleted()) {
        RETURN_ERR_PAMI(PAMI_INVAL, "Completing a completed type.\n");
    }

    // TODO: make MAX_ATOM_SIZE global and queriable
    const unsigned MIN_ATOM_SIZE = 1, MAX_ATOM_SIZE = 128;
    if (atom_size < MIN_ATOM_SIZE || atom_size > MAX_ATOM_SIZE) {
        RETURN_ERR_PAMI(PAMI_INVAL,
                "Atom size %lu is out of range [%lu, %lu].\n",
                atom_size, MIN_ATOM_SIZE, MAX_ATOM_SIZE);
    }

    size_t unit = type_obj->GetUnit();
    if (unit % atom_size != 0) {
        RETURN_ERR_PAMI(PAMI_INVAL,
                "Atom size %lu does not divide type unit %lu.\n",
                atom_size, unit);
    }

    try {
        type_obj->Complete();
        type_obj->SetAtomSize(atom_size);
    } catch (std::bad_alloc) {
        RETURN_ERR_PAMI(PAMI_ENOMEM, "Out of memory.\n");
    }
    return PAMI_SUCCESS;
}

pami_result_t PAMI_Type_destroy (pami_type_t * type)
{
    TypeCode * type_obj = *(TypeCode **)type;
    *type = NULL;

    type_obj->ReleaseReference();
    return PAMI_SUCCESS;
}

pami_result_t PAMI_Type_serialize (pami_type_t   type,
                                   void       ** address,
                                   size_t      * size)
{
    TypeCode * type_obj = (TypeCode *)type;
    if (type_obj->IsCompleted()) {
        RETURN_ERR_PAMI(PAMI_INVAL, "Serializing an incompleted type.\n");
    }

    *address = type_obj->GetCodeAddr();
    *size    = type_obj->GetCodeSize();
    return PAMI_SUCCESS;
}

pami_result_t PAMI_Type_deserialize (pami_type_t * type,
                                     void        * address,
                                     size_t        size)
{
    try {
        TypeCode * type_obj = new TypeCode(address, size);
        assert(size == type_obj->GetCodeSize());
        *type = (pami_type_t)type_obj;
    } catch (std::bad_alloc) {
        *type = NULL;
        RETURN_ERR_PAMI(PAMI_INVAL, "Out of memory creating type.\n");
    }
    return PAMI_SUCCESS;
}

pami_result_t PAMI_Type_query (pami_type_t           type,
                               pami_configuration_t  config[],
                               size_t                num_configs)
{
    TypeCode * type_obj = (TypeCode *)type;
    if (type_obj->IsCompleted()) {
        RETURN_ERR_PAMI(PAMI_INVAL, "Querying an incompleted type.\n");
    }

    for (unsigned i = 0; i < num_configs; i++) {
        switch (config[i].name) {
            case PAMI_TYPE_DATA_SIZE:
                config[i].value.intval = type_obj->GetDataSize();
                break;
            case PAMI_TYPE_DATA_EXTENT:
                config[i].value.intval = type_obj->GetExtent();
                break;
            case PAMI_TYPE_ATOM_SIZE:
                config[i].value.intval = type_obj->GetAtomSize();
                break;
            default:
                RETURN_ERR_PAMI(PAMI_INVAL,
                        "Invalid attribute '%s' to query.\n", config[i].name);
        }
    }
    return PAMI_SUCCESS;
}

extern "C" {
// TODO: Remove pack and unpack which are no longer in pami.h

pami_result_t PAMI_Type_pack_data (pami_type_t         src_type,
                                   size_t              src_offset,
                                   void              * src_addr,
                                   void              * dst_addr,
                                   size_t              dst_size,
                                   pami_data_function  data_fn,
                                   void              * cookie)
{
    TypeCode * type_obj = (TypeCode *)src_type;
    if (!type_obj->IsCompleted()) {
        RETURN_ERR_PAMI(PAMI_INVAL, "Using incomplete type.\n");
    }

    TypeMachine packer(type_obj);
    packer.SetCopyFunc(data_fn, cookie);
    packer.MoveCursor(src_offset);
    packer.Pack(dst_addr, src_addr, dst_size);
    return PAMI_SUCCESS;
}

pami_result_t PAMI_Type_unpack_data (pami_type_t         dst_type,
                                     size_t              dst_offset,
                                     void              * dst_addr,
                                     void              * src_addr,
                                     size_t              src_size,
                                     pami_data_function  data_fn,
                                     void              * cookie)
{
    TypeCode * type_obj = (TypeCode *)dst_type;
    if (!type_obj->IsCompleted()) {
        RETURN_ERR_PAMI(PAMI_INVAL, "Using incomplete type.\n");
    }

    TypeMachine unpacker(type_obj);
    unpacker.SetCopyFunc(data_fn, cookie);
    unpacker.MoveCursor(dst_offset);
    unpacker.Unpack(dst_addr, src_addr, src_size);
    return PAMI_SUCCESS;
}

}
