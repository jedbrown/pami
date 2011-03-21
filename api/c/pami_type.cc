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

pami_type_t PAMI_TYPE_CONTIGUOUS         = new TypeContig;
pami_type_t PAMI_TYPE_SIGNED_INT         = new TypeContig(sizeof(signed int));
pami_type_t PAMI_TYPE_SIGNED_LONG        = new TypeContig(sizeof(signed long));
pami_type_t PAMI_TYPE_SIGNED_LONG_LONG   = new TypeContig(sizeof(signed long long));
pami_type_t PAMI_TYPE_UNSIGNED_INT       = new TypeContig(sizeof(unsigned int));
pami_type_t PAMI_TYPE_UNSIGNED_LONG      = new TypeContig(sizeof(unsigned long));
pami_type_t PAMI_TYPE_UNSIGNED_LONG_LONG = new TypeContig(sizeof(unsigned long long));

pami_data_function PAMI_DATA_COPY = NULL;

extern "C" {

  // BGQ does not use exceptions
#ifdef __pami_target_bgq__
#ifdef try
#undef try
#endif
#define try
#ifdef catch
#undef catch
#endif
#define catch(a) if(0)
#endif

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
    if (!type_obj->IsCompleted()) {
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

pami_result_t PAMI_Type_transform_data (void               * src_addr,
                                        pami_type_t          src_type,
                                        size_t               src_offset,
                                        void               * dst_addr,
                                        pami_type_t          dst_type,
                                        size_t               dst_offset,
                                        size_t               size,
                                        pami_data_function   data_fn,
                                        void               * cookie)
{
    TypeCode * src_type_obj = (TypeCode *)src_type;
    TypeCode * dst_type_obj = (TypeCode *)dst_type;
    if (!src_type_obj->IsCompleted() || !dst_type_obj->IsCompleted()) {
        RETURN_ERR_PAMI(PAMI_INVAL, "Using incomplete type.\n");
    }

    if (dst_type_obj->IsContiguous()) {
        // packing: non-contiguous to contiguous
        TypeMachine packer(src_type_obj);
        packer.SetCopyFunc(data_fn, cookie);
        packer.MoveCursor(src_offset);
        packer.Pack((char *)dst_addr + dst_offset, src_addr, size);

    } else if (src_type_obj->IsContiguous()) {
        // unpacking: contiguous to non-contiguous
        TypeMachine unpacker(dst_type_obj);
        unpacker.SetCopyFunc(data_fn, cookie);
        unpacker.MoveCursor(dst_offset);
        unpacker.Unpack(dst_addr, (char *)src_addr + src_offset, size);

    } else {
        // generic: non-contiguous to non-contiguous
        TypeMachine packer(src_type_obj);
        packer.MoveCursor(src_offset);

        TypeMachine unpacker(dst_type_obj);
        unpacker.SetCopyFunc(data_fn, cookie);
        unpacker.MoveCursor(dst_offset);

        // use a temporary buffer to copy in and out data
        const size_t TMP_BUF_SIZE = 8192;
        char tmp_buf[TMP_BUF_SIZE];

        for (size_t offset = 0; offset < size; offset += TMP_BUF_SIZE) {
            size_t bytes_to_copy = std::min(size - offset, TMP_BUF_SIZE);
            packer.Pack(tmp_buf, src_addr, bytes_to_copy);
            unpacker.Unpack(dst_addr, tmp_buf, bytes_to_copy);
        }
    }

    return PAMI_SUCCESS;
}

} // extern "C"
