/**
 * \file common/type/TypeCode.h
 * \brief ???
 */
#ifndef __common_type_TypeCode_h__
#define __common_type_TypeCode_h__

/*
   0         1         2         3
   01234567890123456789012345678901 <-- byte displacement in data type
   --------------------------------
   ..#...#...#                       T1=(bytes=1, shift=2, stride=4, reps=3)
   .##..##..##                       T2=(T1, shift=-1, stride=1, reps=2)
   ......##..##..##......##..##..##  T3=(T2, shift=5, stride=16, reps=2)
   --------------------------------
 */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#ifdef USE_ITRACE
#include "lapi_itrace.h"
#else
#define ITRC(...)
#endif
#include "Math.h"
#include "ReferenceCount.h"
//#undef assert
//#define assert(...)

namespace PAMI
{
  namespace Type
  {

    class TypeCode : public ReferenceCount
    {
        public:

            typedef enum
            {
              PRIMITIVE_TYPE_BYTE=0,

              PRIMITIVE_TYPE_SIGNED_CHAR,
              PRIMITIVE_TYPE_SIGNED_SHORT,
              PRIMITIVE_TYPE_SIGNED_INT,
              PRIMITIVE_TYPE_SIGNED_LONG,
              PRIMITIVE_TYPE_SIGNED_LONG_LONG,

              PRIMITIVE_TYPE_UNSIGNED_CHAR,
              PRIMITIVE_TYPE_UNSIGNED_SHORT,
              PRIMITIVE_TYPE_UNSIGNED_INT,
              PRIMITIVE_TYPE_UNSIGNED_LONG,
              PRIMITIVE_TYPE_UNSIGNED_LONG_LONG,

              PRIMITIVE_TYPE_FLOAT,
              PRIMITIVE_TYPE_DOUBLE,
              PRIMITIVE_TYPE_LONG_DOUBLE,

              PRIMITIVE_TYPE_LOGICAL,

              PRIMITIVE_TYPE_SINGLE_COMPLEX,
              PRIMITIVE_TYPE_DOUBLE_COMPLEX,

              PRIMITIVE_TYPE_LOC_2INT,
              PRIMITIVE_TYPE_LOC_2FLOAT,
              PRIMITIVE_TYPE_LOC_2DOUBLE,
              PRIMITIVE_TYPE_LOC_SHORT_INT,
              PRIMITIVE_TYPE_LOC_FLOAT_INT,
              PRIMITIVE_TYPE_LOC_DOUBLE_INT,

              PRIMITIVE_TYPE_COUNT,
            } primitive_type_t;

            typedef unsigned primitive_logical_t; // PRIMITIVE_TYPE_LOGICAL

            //
            // primitive_complex_t<float> ..... PRIMITIVE_TYPE_SINGLE_COMPLEX
            // primitive_complex_t<double> .... PRIMITIVE_TYPE_DOUBLE_COMPLEX
            //
            template <typename T>
            struct primitive_complex_t
            {
              T real;
              T imaginary;
            };

            //
            // primitive_loc_t<int,int> ....... PRIMITIVE_TYPE_LOC_2INT
            // primitive_loc_t<float,float> ... PRIMITIVE_TYPE_LOC_2FLOAT
            // primitive_loc_t<double,double> . PRIMITIVE_TYPE_LOC_2DOUBLE
            // primitive_loc_t<short,int> ..... PRIMITIVE_TYPE_LOC_SHORT_INT
            // primitive_loc_t<float,int> ..... PRIMITIVE_TYPE_LOC_FLOAT_INT
            // primitive_loc_t<double,int> .... PRIMITIVE_TYPE_LOC_DOUBLE_INT
            //
            template <typename T_Value, typename T_Index>
            struct primitive_loc_t
            {
              T_Value value;
              T_Index index;
            };

            TypeCode();
            TypeCode(void *code_addr, size_t code_size);
            TypeCode(size_t code_size, primitive_type_t primitive);
            ~TypeCode();

            void AddShift(size_t shift);
            void AddSimple(size_t bytes, size_t stride, size_t reps);
            void AddTyped(TypeCode *sub_type, size_t stride, size_t reps);
            void Optimize();
            void Complete();

            bool IsCompleted() const;
            bool IsContiguous() const;
            bool IsSimple() const;
            bool IsPrimitive() const;

            void * GetCodeAddr() const;
            size_t GetCodeSize() const;
            size_t GetDataSize() const;
            size_t GetExtent() const;
            size_t GetDepth() const;
            size_t GetNumBlocks() const;
            size_t GetUnit() const;
            primitive_type_t GetPrimitive() const;
            size_t GetAtomSize() const;

            void   SetAtomSize(size_t atom_size);

            void Show() const;

            friend class TypeMachine;

        private:
            enum Opcode { BEGIN, COPY, CALL, SHIFT, END };

            struct Op {
                Opcode  opcode;
                Opcode  prev_opcode;

                Op(Opcode opcode) : opcode(opcode) { }
            };

            struct Begin : Op {
                unsigned int contiguous:1;
                unsigned int simple:1;
                unsigned int depth;
                size_t  code_size;
                size_t  data_size;
                size_t  extent;
                size_t  num_blocks;
                size_t  unit;
                size_t  atom_size;

                Begin()
                    : Op(BEGIN), contiguous(1), simple(1), depth(1), code_size(0),
                    data_size(0), extent(0), num_blocks(0), unit(0), atom_size(0) { }

                void Show(int pc) const {
                    ITRC(IT_TYPE, "%d: Begin: contiguous %d simple %d code_size %zu depth %u "
                            "data_size %zu extent %zu num_blocks %zu unit %zu atom_size %zu\n",
                            pc, contiguous, simple, code_size, depth, data_size, extent,
                            num_blocks, unit, atom_size);
                }
            };

            struct Copy : Op {
                size_t  bytes;
                size_t  stride;
                size_t  reps;

                Copy(size_t bytes = 0, size_t stride = 0, size_t reps = 0)
                    : Op(COPY), bytes(bytes), stride(stride), reps(reps) { }

                inline void Show(int pc) const {
                    ITRC(IT_TYPE, "%d: Copy: bytes %zu stride %zd reps %zu\n",
                            pc, bytes, stride, reps);
                }
            };

            struct Call : Op {
                size_t  sub_type;
                size_t  stride;
                size_t  reps;

                Call(size_t sub_type = 0, size_t stride = 0, size_t reps = 0)
                    : Op(CALL), sub_type(sub_type), stride(stride), reps(reps) { }

                inline void Show(int pc) const {
                    ITRC(IT_TYPE, "%d: Call: sub_type 0x%zx stride %zd reps %zu\n",
                            pc, pc + sub_type, stride, reps);
                }
            };

            struct Shift : Op {
                size_t  shift;

                Shift(size_t shift = 0) : Op(SHIFT), shift(shift) { }

                inline void Show(int pc) const {
                    ITRC(IT_TYPE, "%d: Shift: shift %zd\n", pc, shift);
                }
            };

            struct End : Op {
                End() : Op(END) { }

                inline void Show(int pc) const {
                    ITRC(IT_TYPE, "%d: End\n", pc);
                }
            };

            template <class T> void Push(const T &op);
            template <class T> bool Top(T &op);
            void Pop();

            char  *code;
            size_t code_buf_size;
            size_t prev_cursor;
            size_t code_cursor;
            bool   completed;
            // bool   to_optimize;

            void CheckCodeBuffer(size_t inc_code_size);
            void ResizeCodeBuffer(size_t new_size);
            void AddCodeSize(size_t inc_code_size);
            void AddDataSize(size_t inc_data_size);
            void AddExtent(size_t inc_extent);
            void UpdateDepth(unsigned int call_depth);
            void AddNumBlocks(size_t inc_num_blocks);
            void UpdateUnit(size_t new_unit);
            // void CopySubTypes();
            void SetContiguous(bool);
            void SetSimple(bool);

          protected:
            primitive_type_t   primitive;
    };

    inline TypeCode::TypeCode()
        : code(NULL), code_buf_size(0), prev_cursor(0), code_cursor(0),
        completed(false), primitive(PRIMITIVE_TYPE_COUNT)
    {
        ResizeCodeBuffer(sizeof(Begin) + sizeof(Copy)*4);
        Push(Begin());
    }

    inline TypeCode::TypeCode(void *code_addr, size_t code_size)
        : code(NULL), code_buf_size(0), prev_cursor(0), code_cursor(0),
        completed(true), primitive(PRIMITIVE_TYPE_COUNT)
    {
        ResizeCodeBuffer(code_size);
        memcpy(code, code_addr, code_size);
    }

    inline TypeCode::TypeCode(size_t code_size, primitive_type_t primitive_type = PRIMITIVE_TYPE_COUNT)
        : code(NULL), code_buf_size(0), prev_cursor(0), code_cursor(0),
        completed(true), primitive(primitive_type)
    {
        ResizeCodeBuffer(code_size);
    }

    inline TypeCode::~TypeCode()
    {
        delete[] code;
    }

    inline void TypeCode::ResizeCodeBuffer(size_t new_size)
    {
        if (code_buf_size >= new_size)
            return;

        char *new_code = new char[new_size];
        if (code) {
            memcpy(new_code, code, code_cursor);
            delete[] code;
        }
        code = new_code;
        code_buf_size = new_size;
    }

    inline void TypeCode::CheckCodeBuffer(size_t inc_code_size)
    {
        if (code_cursor + inc_code_size > code_buf_size) {
            ResizeCodeBuffer(code_buf_size * 2);
        }
    }

    template <class T>
        void TypeCode::Push(const T &op)
        {
            CheckCodeBuffer(sizeof(op) + sizeof(struct Op));
            T *top = (T *)(code + code_cursor);

            // save previous opcode so it's not overwritten
            Opcode prev_opcode = top->prev_opcode;
            *top = op;
            // recover previous opcode
            top->prev_opcode = prev_opcode;

            // save current opcode as previous opcode in the next instruction
            (++top)->prev_opcode = op.opcode;

            code_cursor += sizeof(op);
            AddCodeSize(sizeof(op));
        }

    template <class T>
        bool TypeCode::Top(T &op)
        {
            Op &top = *(Op *)(code + code_cursor);
            if (top.prev_opcode != op.opcode)
                return false;

            op = *(T *)(code + code_cursor - sizeof(T));
            return true;
        }

    inline void TypeCode::Pop()
    {
        static const int op_size[] = {
            sizeof(Begin), sizeof(Copy), sizeof(Call), sizeof(Shift), sizeof(End)
        };

        assert(0<code_cursor);

        Op &top = *(Op *)(code + code_cursor);
        code_cursor -= op_size[top.prev_opcode];
        AddCodeSize(0-op_size[top.prev_opcode]);
    }

    inline void * TypeCode::GetCodeAddr() const
    {
        assert(IsCompleted());
        return code;
    }

    inline size_t TypeCode::GetCodeSize() const
    {
        return ((Begin *)code)->code_size;
    }

    inline size_t TypeCode::GetDataSize() const
    {
        return ((Begin *)code)->data_size;
    }

    inline size_t TypeCode::GetExtent() const
    {
        return ((Begin *)code)->extent;
    }

    inline size_t TypeCode::GetDepth() const
    {
        return ((Begin *)code)->depth;
    }

    inline size_t TypeCode::GetNumBlocks() const
    {
        return ((Begin *)code)->num_blocks;
    }

    inline size_t TypeCode::GetUnit() const
    {
        return ((Begin *)code)->unit;
    }

    inline size_t TypeCode::GetAtomSize() const
    {
        return ((Begin *)code)->atom_size;
    }

    inline TypeCode::primitive_type_t TypeCode::GetPrimitive() const
    {
      return primitive;
    }

    inline void   TypeCode::SetAtomSize(size_t atom_size)
    {
        assert(GetUnit() % atom_size == 0);
        ((Begin *)code)->atom_size = atom_size;
    }

    inline bool TypeCode::IsCompleted() const
    {
        return completed;
    }

    inline void TypeCode::SetSimple(bool is_simple)
    {
        ((Begin *)code)->simple = is_simple;
    }

    inline bool TypeCode::IsSimple() const
    {
        return ((Begin *)code)->simple;
    }

    inline void TypeCode::SetContiguous(bool is_contig)
    {
        ((Begin *)code)->contiguous = is_contig;
    }

    inline bool TypeCode::IsContiguous() const
    {
        return ((Begin *)code)->contiguous;
    }

    inline bool TypeCode::IsPrimitive() const
    {
        return (primitive < PRIMITIVE_TYPE_COUNT);
    }

    inline void TypeCode::AddCodeSize(size_t inc_code_size)
    {
        ((Begin *)code)->code_size += inc_code_size;
    }

    inline void TypeCode::AddDataSize(size_t inc_data_size)
    {
        ((Begin *)code)->data_size += inc_data_size;
    }

    inline void TypeCode::AddExtent(size_t inc_extent)
    {
        ((Begin *)code)->extent += inc_extent;
    }

    inline void TypeCode::AddNumBlocks(size_t inc_num_blocks)
    {
        ((Begin *)code)->num_blocks += inc_num_blocks;
    }

    inline void TypeCode::UpdateDepth(unsigned int call_depth)
    {
        unsigned int &depth = ((Begin *)code)->depth;
        if (depth < call_depth)
            depth = call_depth;
    }

    inline void TypeCode::UpdateUnit(size_t new_unit)
    {
        size_t &unit = ((Begin *)code)->unit;
        if (unit == 0)
            unit = new_unit;
        else
            unit = Math::GCD(unit, new_unit);
    }

    inline void TypeCode::AddShift(size_t shift)
    {
        assert(!IsCompleted());

        Shift prev_shift;
        Copy prev_copy;

        ITRC(IT_TYPE, "AddShift(): this 0x%zx shift %zd\n", this, shift);

        do {
            if (0 == shift) {
                // SHIFT 0                  => NO-OP
                break;
            }
            if (Top(prev_shift)) {
                // SHIFT shift1
                // SHIFT shift2             => SHIFT shift1+shift2
                ITRC(IT_TYPE, " AddShift(): this 0x%zx modify prev SHIFT\n", this);
                Pop();
                AddShift(prev_shift.shift + shift);
                break;
            }
            if (Top(prev_copy)) {
                // previous instruction is a copy: if single repeat modify its stride
                if (1 == prev_copy.reps) {
                    // SHIFT shift1
                    // COPY  bytes  stride  1   => COPY  bytes  stride+sift  1
                    ITRC(IT_TYPE, " AddShift(): this 0x%zx modify prev COPY\n", this);
                    Pop();
                    AddSimple(prev_copy.bytes, prev_copy.stride + shift, 1);
                    break;
                }
            }
            // No optimization, yet: add a shift instruction
            Push(Shift(shift));
        } while (0);
    }

    inline void TypeCode::AddSimple(size_t bytes, size_t stride, size_t reps)
    {
        assert(!IsCompleted());

        Copy prev_copy;

        ITRC(IT_TYPE, "AddSimple(): this 0x%zx bytes %zu stride %zd reps %zu\n", this, bytes, stride, reps);

        do {
            if (0 == reps) {
                // COPY  bytes  stride  0       => NO-OP
                break;
            }
            if (0 == bytes) {
                // COPY  0  stride  reps        => SHIFT  stride*reps
                ITRC(IT_TYPE, " AddSimple(): this 0x%zx add SHIFT\n", this);
                AddShift(stride * reps);
                break;
            }
            if ((bytes == stride) && (1<reps)) {
                // COPY  bytes  bytes  reps     => COPY  bytes*reps  stride*reps  1
                ITRC(IT_TYPE, " AddSimple(): this 0x%zx add modified COPY\n", this);
                AddSimple(bytes * reps, stride * reps, 1);
                break;
            }
            if (Top(prev_copy)) {
                // the previous instruction was a COPY
                if (prev_copy.bytes==prev_copy.stride && 1==reps) {
                    // COPY  bytes1  bytes1  1
                    // COPY  bytes2  stride  1   => COPY bytes1+bytes2  stride+bytes1  1
                    ITRC(IT_TYPE, " AddSimple(): this 0x%zx modify prev COPY [1]\n", this);
                    Pop();
                    AddSimple(prev_copy.bytes + bytes, prev_copy.stride + stride, 1);
                    break;
                }
                if (prev_copy.bytes==bytes && prev_copy.stride==stride) {
                    // COPY  bytes  stride  reps1
                    // COPY  bytes  stride  reps2   => COPY  bytes  stride  reps1+reps2
                    ITRC(IT_TYPE, " AddSimple(): this 0x%zx modify previous COPY [2]\n", this);
                    Pop();
                    AddSimple(bytes, stride, prev_copy.reps + reps);
                    break;
                }
            }
            // no optimization yet: add a copy instruction
            Push(Copy(bytes, stride, reps));
        } while (0);
    }

    inline void TypeCode::AddTyped(TypeCode *sub_type, size_t stride, size_t reps)
    {
        assert(!IsCompleted());
        assert(sub_type->IsCompleted());

        Call prev_call;

        ITRC(IT_TYPE, "AddTyped(): this 0x%zx sub_type 0x%zx stride %zd reps %zu\n", this, sub_type, stride, reps);
        sub_type->Show();

        do {
            if (0 == reps) {
                // CALL subtype  stride  0      => NO-OP
                break;
            }
            if (sub_type->IsContiguous()) {
                // CALL  subtye  stride  reps
                // subtype is contiguous        => COPY  subtype->bytes  stride  reps
                ITRC(IT_TYPE, " AddTyped(): this 0x%zx add COPY [1]\n", this);
                AddSimple(sub_type->GetDataSize(), stride, reps);
                break;
            }
            if (sub_type->IsSimple()) {
                // CALL  subtype  stride1  reps1
                // subtype is simple
                assert(COPY == ((Op *)(sub_type->code+sizeof(Begin)))->opcode);

                Copy *sub_copy = (Copy *)(sub_type->code+sizeof(Begin));

                if (stride == sub_type->GetExtent()) {
                    // the Call's stride is equal to the subtype's extent
                    // COPY  bytes  stride2  reps2  => COPY  bytes  stride2  reps1*reps2
                    ITRC(IT_TYPE, " AddTyped(): this 0x%zx add COPY [2]\n", this);
                    AddSimple(sub_copy->bytes, sub_copy->stride, sub_copy->reps * reps);
                    break;
                }
                // the Call's stride is different from the subtype's extent
                if (1 == sub_copy->reps) {
                    // the subtype consists of a single copy instruction w/no repeats
                    ITRC(IT_TYPE, " AddTyped(): this 0x%zx add COPY [3]\n", this);
                    AddSimple(sub_copy->bytes, stride, reps);
                    break;
                }
            }
            if (Top(prev_call)) {
                // previous instruction is a call
                if (prev_call.sub_type==(size_t)sub_type && prev_call.stride==stride) {
                    // CALL  bytes  stride  reps1
                    // CALL  bytes  stride  reps2   => CALL  bytes  stride  reps1+reps2
                    Pop();
                    AddTyped(sub_type, stride, prev_call.reps + reps);
                    break;
                }
            }
            // No optimization yet: add a new CALL
            Push(Call((size_t)sub_type, stride, reps));
        } while (0);
    }

    inline void TypeCode::Complete()
    {
        assert(!IsCompleted());

        // push the END instruction
        Push(End());

        assert(code_cursor <= GetCodeSize());

        // calculate the data layout properties: data size/extent, unit, depth
        // copy the sub-types' code, if any

        size_t pc = 0;
        Op *op;
        
        do {
            op = (Op *)(code + pc);
            switch (op->opcode) {
                case BEGIN: pc += sizeof(Begin); break;
                case COPY:
                    // if not the first COPY => not simple
                    if (BEGIN != op->prev_opcode) SetSimple(false);
                    // update data size/extent, unit size and number of blocks
                    AddDataSize(((Copy *)op)->bytes * ((Copy *)op)->reps);
                    AddExtent(((Copy *)op)->stride * ((Copy *)op)->reps);
                    AddNumBlocks(((Copy *)op)->reps);
                    UpdateUnit(((Copy *)op)->bytes);
                    pc += sizeof(Copy); break;
                case CALL:
                    // a CALL instruction breaks simplicity
                    SetSimple(false);
                    // copy the sub-type's code
                    // update data size/extent, unit size, number of blocks and depth
                    {
                        TypeCode *sub_type = (TypeCode *)(((Call *)op)->sub_type);

                        sub_type->Show();

                        // update the type's code size w/the sub-type's code size and resize
                        AddCodeSize(sub_type->GetCodeSize());
                        ResizeCodeBuffer(GetCodeSize());

                        // resize could have changed the location of op
                        op = (Op *)(code + pc);

                        // update the new location of the sub-type
                        ((Call *)op)->sub_type = code_cursor - pc;
                       
                        // copy the sub-type's code
                        memcpy(code + code_cursor, sub_type->code, sub_type->GetCodeSize());
                        code_cursor += sub_type->GetCodeSize();

                        // update the type's properties
                        AddDataSize(sub_type->GetDataSize() * ((Call *)op)->reps);
                        AddExtent(((Call *)op)->stride * ((Call *)op)->reps);
                        UpdateDepth(sub_type->GetDepth() + 1);
                        AddNumBlocks(sub_type->GetNumBlocks() * ((Call *)op)->reps);
                        UpdateUnit(sub_type->GetUnit());
                    }
                    pc += sizeof(Call); break;
                case SHIFT:
                    // a SHIFT instruction breaks simplicity
                    SetSimple(false);
                    // update data extent
                    AddExtent(((Shift *)op)->shift);
                    pc += sizeof(Shift); break;
                case END:
                    // no instruction breaks simplicity
                    if (BEGIN == op->prev_opcode) SetSimple(false);
                    pc += sizeof(End); break;
                default:    assert(!"Bogus opcode");
            }
        } while (op->opcode != END);

        // Check for contiguity and simplicity: after the optimizations already
        // performed, the following will be true:
        // - a simple data layout contains a single COPY instruction
        // - a contiguous data layout consists of a single copy instruction that has
        //   the bytes value equal to the stride value
        do {
            if (!IsSimple()) { SetContiguous(false); break; }
            // this is a simple data type, i.e. only one copy instruction
            // check for contiguity
            Copy *single_copy = (Copy *)(code + sizeof(Begin));
            if (single_copy->bytes != single_copy->stride) { SetContiguous(false); break; }
            assert(1 == single_copy->reps);
        } while (0);

        completed = true;
        ITRC(IT_TYPE, "Complete(): this 0x%zx code = 0x%zx; code_buf_size = %zd; code_cursor = %zu; completed = %d\n", this, code, code_buf_size, code_cursor, completed);
        Show();
    }

    inline void TypeCode::Show() const
    {
#ifdef USE_ITRACE
        size_t pc = 0;
        Op *op;
        do {
            op = (Op *)(code + pc);
            switch (op->opcode) {
                case BEGIN: ((Begin *)op)->Show(pc); pc += sizeof(Begin); break;
                case SHIFT: ((Shift *)op)->Show(pc); pc += sizeof(Shift); break;
                case COPY:   ((Copy *)op)->Show(pc); pc += sizeof(Copy); break;
                case CALL:   ((Call *)op)->Show(pc); pc += sizeof(Call); break;
                case END:     ((End *)op)->Show(pc); pc += sizeof(End); break;
                default:    assert(!"Bogus opcode");
            }
        } while (pc < GetCodeSize());
#endif
    }

    class TypeContig : public TypeCode
    {
        public:
            TypeContig(size_t atom_size);
            TypeContig(primitive_type_t primitive);
            ~TypeContig();
    };

    inline TypeContig::TypeContig(size_t atom_size)
        : TypeCode()
    {
        assert(0<atom_size);
        primitive = PRIMITIVE_TYPE_BYTE;
        size_t prim_size = ULONG_MAX - ULONG_MAX%atom_size;
        AddSimple(prim_size, prim_size, 1);
        Complete();
        SetAtomSize(atom_size);
        AcquireReference();
    }

    inline TypeContig::TypeContig(primitive_type_t primitive_type)
    {
      primitive = primitive_type;

      size_t primitive_atom = 0;

      switch (primitive_type)
      {
        case PRIMITIVE_TYPE_BYTE:
          primitive_atom = sizeof(uint8_t);
          break;

        case PRIMITIVE_TYPE_SIGNED_CHAR:
          primitive_atom = sizeof(signed char);
          break;

        case PRIMITIVE_TYPE_SIGNED_SHORT:
          primitive_atom = sizeof(signed short);
          break;

        case PRIMITIVE_TYPE_SIGNED_INT:
          primitive_atom = sizeof(signed int);
          break;

        case PRIMITIVE_TYPE_SIGNED_LONG:
          primitive_atom = sizeof(signed long);
          break;

        case PRIMITIVE_TYPE_SIGNED_LONG_LONG:
          primitive_atom = sizeof(signed long long);
          break;

        case PRIMITIVE_TYPE_UNSIGNED_CHAR:
          primitive_atom = sizeof(unsigned char);
          break;

        case PRIMITIVE_TYPE_UNSIGNED_SHORT:
          primitive_atom = sizeof(unsigned short);
          break;

        case PRIMITIVE_TYPE_UNSIGNED_INT:
          primitive_atom = sizeof(unsigned int);
          break;

        case PRIMITIVE_TYPE_UNSIGNED_LONG:
          primitive_atom = sizeof(unsigned long);
          break;

        case PRIMITIVE_TYPE_UNSIGNED_LONG_LONG:
          primitive_atom = sizeof(unsigned long long);
          break;

        case PRIMITIVE_TYPE_FLOAT:
          primitive_atom = sizeof(float);
          break;

        case PRIMITIVE_TYPE_DOUBLE:
          primitive_atom = sizeof(double);
          break;

        case PRIMITIVE_TYPE_LONG_DOUBLE:
          primitive_atom = sizeof(long double);
          break;

        case PRIMITIVE_TYPE_LOGICAL:
          primitive_atom = sizeof(primitive_logical_t);
          break;

        case PRIMITIVE_TYPE_SINGLE_COMPLEX:
          primitive_atom = sizeof(primitive_complex_t<float>);
          break;

        case PRIMITIVE_TYPE_DOUBLE_COMPLEX:
          primitive_atom = sizeof(primitive_complex_t<double>);
          break;

        case PRIMITIVE_TYPE_LOC_2INT:
          primitive_atom = sizeof(primitive_loc_t<int,int>);
          break;

        case PRIMITIVE_TYPE_LOC_2FLOAT:
          primitive_atom = sizeof(primitive_loc_t<float,float>);
          break;

        case PRIMITIVE_TYPE_LOC_2DOUBLE:
          primitive_atom = sizeof(primitive_loc_t<double,double>);
          break;

        case PRIMITIVE_TYPE_LOC_SHORT_INT:
          primitive_atom = sizeof(primitive_loc_t<short,int>);
          break;

        case PRIMITIVE_TYPE_LOC_FLOAT_INT:
          primitive_atom = sizeof(primitive_loc_t<float,int>);
          break;

        case PRIMITIVE_TYPE_LOC_DOUBLE_INT:
          primitive_atom = sizeof(primitive_loc_t<double,int>);
          break;

        default:
          // Bad!!!
          abort();
          break;
      };


        AddSimple(primitive_atom, primitive_atom, 1);
        Complete();
        SetAtomSize(primitive_atom);
        AcquireReference();
    }

    inline TypeContig::~TypeContig()
    {
        assert(!"Can't delete TypeContig");
    }

  }
}

// Object of primitive PAMI_TYPE_CONTIGOUS with an atom size of 1
// and unit size of ULONG_MAX
extern PAMI::Type::TypeContig *PAMI_TYPE_CONTIG_MAX;

typedef enum
{
  PAMI_BYTE               = PAMI::Type::TypeCode::PRIMITIVE_TYPE_BYTE,

  PAMI_SIGNED_CHAR        = PAMI::Type::TypeCode::PRIMITIVE_TYPE_SIGNED_CHAR,
  PAMI_SIGNED_SHORT       = PAMI::Type::TypeCode::PRIMITIVE_TYPE_SIGNED_SHORT,
  PAMI_SIGNED_INT         = PAMI::Type::TypeCode::PRIMITIVE_TYPE_SIGNED_INT,
  PAMI_SIGNED_LONG        = PAMI::Type::TypeCode::PRIMITIVE_TYPE_SIGNED_LONG,
  PAMI_SIGNED_LONG_LONG   = PAMI::Type::TypeCode::PRIMITIVE_TYPE_SIGNED_LONG_LONG,

  PAMI_UNSIGNED_CHAR      = PAMI::Type::TypeCode::PRIMITIVE_TYPE_UNSIGNED_CHAR,
  PAMI_UNSIGNED_SHORT     = PAMI::Type::TypeCode::PRIMITIVE_TYPE_UNSIGNED_SHORT,
  PAMI_UNSIGNED_INT       = PAMI::Type::TypeCode::PRIMITIVE_TYPE_UNSIGNED_INT,
  PAMI_UNSIGNED_LONG      = PAMI::Type::TypeCode::PRIMITIVE_TYPE_UNSIGNED_LONG,
  PAMI_UNSIGNED_LONG_LONG = PAMI::Type::TypeCode::PRIMITIVE_TYPE_UNSIGNED_LONG_LONG,

  PAMI_FLOAT              = PAMI::Type::TypeCode::PRIMITIVE_TYPE_FLOAT,
  PAMI_DOUBLE             = PAMI::Type::TypeCode::PRIMITIVE_TYPE_DOUBLE,
  PAMI_LONG_DOUBLE        = PAMI::Type::TypeCode::PRIMITIVE_TYPE_LONG_DOUBLE,

  PAMI_LOGICAL            = PAMI::Type::TypeCode::PRIMITIVE_TYPE_LOGICAL,

  PAMI_SINGLE_COMPLEX     = PAMI::Type::TypeCode::PRIMITIVE_TYPE_SINGLE_COMPLEX,
  PAMI_DOUBLE_COMPLEX     = PAMI::Type::TypeCode::PRIMITIVE_TYPE_DOUBLE_COMPLEX,

  PAMI_LOC_2INT           = PAMI::Type::TypeCode::PRIMITIVE_TYPE_LOC_2INT,
  PAMI_LOC_2FLOAT         = PAMI::Type::TypeCode::PRIMITIVE_TYPE_LOC_2FLOAT,
  PAMI_LOC_2DOUBLE        = PAMI::Type::TypeCode::PRIMITIVE_TYPE_LOC_2DOUBLE,
  PAMI_LOC_SHORT_INT      = PAMI::Type::TypeCode::PRIMITIVE_TYPE_LOC_SHORT_INT,
  PAMI_LOC_FLOAT_INT      = PAMI::Type::TypeCode::PRIMITIVE_TYPE_LOC_FLOAT_INT,
  PAMI_LOC_DOUBLE_INT     = PAMI::Type::TypeCode::PRIMITIVE_TYPE_LOC_DOUBLE_INT,
  PAMI_DT_COUNT           = PAMI::Type::TypeCode::PRIMITIVE_TYPE_COUNT
} pami_dt;

#endif // _PAMI_TYPE_CODE_H
