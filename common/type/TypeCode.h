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

            TypeCode(bool to_optimize);
            TypeCode(void *code_addr, size_t code_size);
            TypeCode(size_t code_size, primitive_type_t primitive = PRIMITIVE_TYPE_COUNT);
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

                Op(Opcode opcode) : opcode(opcode) { }
            };

            struct Begin : Op {
                unsigned int contiguous:1;
                unsigned int simple:1;
                unsigned int optimized:1;
                unsigned int depth;
                size_t  code_size;
                size_t  data_size;
                size_t  extent;
                size_t  num_blocks;
                size_t  unit;
                size_t  atom_size;

                Begin()
                    : Op(BEGIN), contiguous(1), simple(1), optimized(0), depth(1), code_size(0),
                    data_size(0), extent(0), num_blocks(0), unit(0), atom_size(0) { }

                void Show(int pc) const {
                    ITRC(IT_TYPE, "%d: Begin: contiguous %d simple %d code_size %zu depth %u "
                            " data_size %zu extent %zu num_blocks %zu unit %zu atom_size %zu\n",
                            pc, contiguous, simple, code_size, depth, data_size, extent,
                            num_blocks, unit, atom_size);
                }
            };

            struct Copy : Op {
                size_t  bytes;
                size_t  stride;
                size_t  reps;

                Copy(size_t bytes, size_t stride, size_t reps)
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

                Call(size_t sub_type, size_t stride, size_t reps)
                    : Op(CALL), sub_type(sub_type), stride(stride), reps(reps) { }

                inline void Show(int pc) const {
                    ITRC(IT_TYPE, "%d: Call: sub_type %zu stride %zd reps %zu\n",
                            pc, pc + sub_type, stride, reps);
                }
            };

            struct Shift : Op {
                size_t  shift;

                Shift(size_t shift) : Op(SHIFT), shift(shift) { }

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

            char  *code;
            size_t code_buf_size;
            size_t prev_cursor;
            size_t code_cursor;
            bool   completed;
            bool   to_optimize;

            void CheckCodeBuffer(size_t inc_code_size);
            void ResizeCodeBuffer(size_t new_size);
            void AddCodeSize(size_t inc_code_size);
            void AddDataSize(size_t inc_data_size);
            void AddExtent(size_t inc_extent);
            void UpdateDepth(unsigned int call_depth);
            void AddNumBlocks(size_t inc_num_blocks);
            void UpdateUnit(size_t new_unit);
            void CopySubTypes();
            void SetContiguous(bool);
            void SetSimple(bool);

          protected:
            primitive_type_t   primitive;
    };

    inline TypeCode::TypeCode(bool to_optimize = true)
        : code(NULL), code_buf_size(0), prev_cursor(0), code_cursor(0), completed(false),
        to_optimize(to_optimize), primitive(PRIMITIVE_TYPE_COUNT)
    {
        ResizeCodeBuffer(sizeof(Begin) + sizeof(Copy)*4);
        *(Begin *)code = Begin();
        code_cursor += sizeof(Begin);

        AddCodeSize(sizeof(Begin));
    }

    inline TypeCode::TypeCode(void *code_addr, size_t code_size)
        : code(NULL), code_buf_size(0), prev_cursor(0), code_cursor(0), completed(true),
        to_optimize(true), primitive(PRIMITIVE_TYPE_COUNT)
    {
        ResizeCodeBuffer(code_size);
        memcpy(code, code_addr, code_size);
    }

    inline TypeCode::TypeCode(size_t code_size, primitive_type_t primitive_type)
        : code(NULL), code_buf_size(0), prev_cursor(0), code_cursor(0), completed(true),
        to_optimize(true), primitive(primitive_type)
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

    inline void TypeCode::SetSimple(bool smpl)
    {
        ((Begin *)code)->simple = smpl;
    }

    inline bool TypeCode::IsSimple() const
    {
        return ((Begin *)code)->simple;
    }

    inline void TypeCode::SetContiguous(bool contig)
    {
        ((Begin *)code)->contiguous = contig;
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

        if (shift != 0) {
            ITRC(IT_TYPE, "AddShift(): this 0x%zx shift %zd\n", this, shift);
            if ((COPY == ((Op *)(code+prev_cursor))->opcode) && (1 == ((Copy *)(code+prev_cursor))->reps)) {
                ITRC(IT_TYPE, "AddShift(): this 0x%zx modify prev copy stride by %zd\n", this, shift);
                ((Copy *)(code+prev_cursor))->stride += shift;
            } else if (SHIFT == ((Op *)(code+prev_cursor))->opcode) {
                ITRC(IT_TYPE, "AddShift(): this 0x%zx modify prev shift by %zd\n", this, shift);
                ((Shift *)(code+prev_cursor))->shift += shift;
            } else {
                ITRC(IT_TYPE, "AddShift(): this 0x%zx add shift by %zd\n", this, shift);
                SetSimple(false);
                CheckCodeBuffer(sizeof(Shift));
                *(Shift *)(code + code_cursor) = Shift(shift);
                prev_cursor = code_cursor;
                code_cursor += sizeof(Shift);
                AddCodeSize(sizeof(Shift));
            }
            AddExtent(shift);
        }
    }

    inline void TypeCode::AddSimple(size_t bytes, size_t stride, size_t reps)
    {
        assert(!IsCompleted());

        if (reps > 0) {
            ITRC(IT_TYPE, "AddSimple(): this 0x%zx bytes %zu stride %zd reps %zu\n", this, bytes, stride, reps);
            if (IsContiguous() && (GetDataSize() != GetExtent())) SetContiguous(false);
            if (to_optimize && (bytes == stride)) {
                stride  = bytes *= reps;
                reps    = 1;
            }
            // if not the first copy instruction or reps more than 1,
            // then not simple anymore
            if ((0 != prev_cursor) || (1 != reps)) SetSimple(false);

            CheckCodeBuffer(sizeof(Copy));
            *(Copy *)(code + code_cursor) = Copy(bytes, stride, reps);
            prev_cursor = code_cursor;
            code_cursor += sizeof(Copy);

            AddCodeSize(sizeof(Copy));
            AddDataSize(bytes * reps);
            AddNumBlocks(reps);
            UpdateUnit(bytes);
            AddExtent(stride * reps);
        }
    }

    inline void TypeCode::AddTyped(TypeCode *sub_type, size_t stride, size_t reps)
    {
        assert(!IsCompleted());
        assert(sub_type->IsCompleted());

        if (reps > 0) {
            ITRC(IT_TYPE, "AddTyped(): this 0x%zx sub_type 0x%zx stride %zd reps %zu\n", this, sub_type, stride, reps);
            sub_type->Show();
            // if the subtype is simple and reps is greater than one, then change the reps
            // in the subtype
            if ((1 != reps) && sub_type->IsSimple()) {
                assert(COPY == ((Op *)(sub_type->code+sizeof(Begin)))->opcode);
                // this subtype is a simple copy: add a simple copy instead of a call
                AddSimple(((Copy *)(sub_type->code+sizeof(Begin)))->bytes, stride, reps);
            } else if (sub_type->IsContiguous()) {
                // the subtype is contiguous: add a simple copy instead of a call
                // add a single copy instead of a call
                AddSimple(sub_type->GetDataSize(), stride, reps);
            } else {
                if (IsContiguous()) {
                    if (GetDataSize() != GetExtent()) SetContiguous(false);
                    else SetContiguous(sub_type->IsContiguous());
                }
                CheckCodeBuffer(sizeof(Call));
                *(Call *)(code + code_cursor) = Call((size_t)sub_type, stride, reps);
                prev_cursor = code_cursor;
                code_cursor += sizeof(Call);

                AddCodeSize(sizeof(Call) + sub_type->GetCodeSize());
                AddDataSize(sub_type->GetDataSize() * reps);
                UpdateDepth(sub_type->GetDepth() + 1);
                AddNumBlocks(sub_type->GetNumBlocks()*reps);
                UpdateUnit(sub_type->GetUnit());
                AddExtent(stride * reps);
            }
        }
    }

    inline void TypeCode::Optimize()
    {
        // optimize the code, by lumping together identical copy instructions;
        // this routine needs to be called during the completion of the type code
        // and before the addition of the sub-types, when there is only a single
        // Begin-End sequence
        size_t pc = 0, opc = 0, copyseqstart = 0, copyseqcnt = 0, copyreps = 0;
        size_t ocode_size = GetCodeSize();
        Op *op;

        do {
            op = (Op *)(code + pc);
            switch (op->opcode) {
                case BEGIN:
                    ITRC(IT_TYPE, "Optimize(): this 0x%zx: Begin %zu %zu\n", this, opc, pc);
                    // only one begin instruction allowed, i.e. the first instruction
                    assert(!((Begin *)code)->optimized);
                    opc = pc += sizeof(Begin);
                    break;
                case COPY:
                    if (0 == copyseqstart) {
                        // initialize the sequence
                        ITRC(IT_TYPE, "Optimize(): this 0x%zx: start copy sequence %zu %zu\n", this, opc, pc);
                        copyseqstart = pc;
                        copyseqcnt = 0;
                        copyreps = ((Copy *)op)->reps;
                        pc += sizeof(Copy);
                        break;
                    } else {
                        // this is not the first copy instruction in the sequence
                        if ((((Copy *)(code+copyseqstart))->bytes == ((Copy *)op)->bytes) &&
                            (((Copy *)(code+copyseqstart))->stride == ((Copy *)op)->stride)) {
                            // the sequence continues
                            ITRC(IT_TYPE, "Optimize(): this 0x%zx: add to copy sequence %zu %zu\n", this, opc, pc);
                            copyseqcnt++;
                            copyreps += ((Copy *)op)->reps;
                            pc += sizeof(Copy);
                            break;
                        }
                        // intentional fall through to default
                    }
                default:
                    // not a copy instruction or a copy instr. that breaks the copy sequence
                    if (copyseqstart) {
                        // a copy sequence was in progress; check if more than one copy
                        // instruction in the sequence
                        ITRC(IT_TYPE, "Optimize(): this 0x%zx: break copy sequence %zu %zu\n", this, opc, pc);
                        if ((SHIFT == op->opcode) && (0 == ((Shift *)(code+pc))->shift)) {
                            // this can happen due to type transformations that may have
                            // occured just get rid of it an don't break the copy sequence
                            ITRC(IT_TYPE, "Optimize(): this 0x%zx: zero Shift %zu %zu\n", this, opc, pc);
                            // adjust code size and code cursor for the skipped shift
                            // instruction
                            AddCodeSize(0-sizeof(Shift)); code_cursor -= sizeof(Shift);
                            pc += sizeof(Shift); break;
                        }
                        if (0 < copyseqcnt) {
                            // compress the sequence
                            ITRC(IT_TYPE, "Optimize(): this 0x%zx: compress copy sequence %zu %zu\n", this, opc, pc);
                            assert(opc < pc);
                            ((Copy *)(code+copyseqstart))->reps = copyreps;
                            if (opc < copyseqstart) {
                                memmove(code+opc, code+copyseqstart, sizeof(Copy));
                            }
                            // update code size and code cursor for the skipped copy
                            // instructions
                            AddCodeSize(0-copyseqcnt*sizeof(Copy));
                            code_cursor -= copyseqcnt*sizeof(Copy);
                        }
                        opc += sizeof(Copy);
                        copyseqstart = 0;
                    }

                    switch(op->opcode) {
                        case CALL:
                            ITRC(IT_TYPE, "Optimize(): this 0x%zx: Call %zu %zu\n", this, opc, pc);
                            if (opc < pc) memmove(code+opc, code+pc, sizeof(Call));
                            pc += sizeof(Call); opc += sizeof(Call); break;
                        case COPY:
                            ITRC(IT_TYPE, "Optimize(): this 0x%zx: Copy %zu %zu\n", this, opc, pc);
                            if (opc < pc) memmove(code+opc, code+pc, sizeof(Copy));
                            pc += sizeof(Copy); opc += sizeof(Copy); break;
                        case SHIFT:
                            if (0 == ((Shift *)(code+pc))->shift) {
                                // this can happen due to type transformations that may have occured
                                // just get rid of it altogether
                                ITRC(IT_TYPE, "Optimize(): this 0x%zx: skipping zero Shift %zu %zu\n", this, opc, pc);
                                // adjust code size and code cursor for the skipped shift
                                // instruction
                                AddCodeSize(0-sizeof(Shift)); code_cursor -= sizeof(Shift);
                            } else {
                                ITRC(IT_TYPE, "Optimize(): this 0x%zx: Shift %zu %zu\n", this, opc, pc);
                                if (opc < pc) memmove(code+opc, code+pc, sizeof(Shift));
                                opc += sizeof(Shift);
                            }
                            pc += sizeof(Shift); break;
                        case END:
                            ITRC(IT_TYPE, "Optimize(): this 0x%zx: End %zu %zu\n", this, opc, pc);
                            if (opc < pc) memmove(code+opc, code+pc, sizeof(End));
                            pc += sizeof(End); opc += sizeof(End); break;
                        default: assert(!"Bogus opcode"); 
                    }
                    break;
            }
        } while (END != op->opcode);
        // ((Begin *)code)->code_size = ocode_size;
        ((Begin *)code)->optimized = true;
    }

    inline void TypeCode::Complete()
    {
        assert(!IsCompleted());

        CheckCodeBuffer(sizeof(End));
        *(End *)(code + code_cursor) = End();
        code_cursor += sizeof(End);

        AddCodeSize(sizeof(End));

        assert(code_cursor <= GetCodeSize());

        if (to_optimize) Optimize();

        if (code_cursor < GetCodeSize())
            CopySubTypes();

        if (IsContiguous() && (GetDataSize() != GetExtent())) SetContiguous(false);

        completed = true;
        ITRC(IT_TYPE, "Complete(): this 0x%zx code = 0x%zx; code_buf_size = %zd; code_cursor = %zu; completed = %d; to_optimize = %d\n", this, code, code_buf_size, code_cursor, completed, to_optimize);
        Show();
    }

    inline void TypeCode::CopySubTypes()
    {
        ResizeCodeBuffer(GetCodeSize());

        size_t pc = 0;
        Op *op;
        do {
            op = (Op *)(code + pc);
            switch (op->opcode) {
                case CALL:
                    {
                        size_t &sub_type_loc = ((Call *)op)->sub_type;
                        TypeCode *sub_type = (TypeCode *)sub_type_loc;
                        sub_type->Show();
                        sub_type_loc = code_cursor - pc;
                        memcpy(code + code_cursor, sub_type->code,
                                sub_type->GetCodeSize());
                        code_cursor += sub_type->GetCodeSize();
                        pc += sizeof(Call);
                        break;
                    }
                case BEGIN: pc += sizeof(Begin); break;
                case SHIFT: pc += sizeof(Shift); break;
                case COPY:  pc += sizeof(Copy); break;
                case END:   pc += sizeof(End); break;
                default:    assert(!"Bogus opcode");
            }
        } while (op->opcode != END);
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
        : TypeCode(true)
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
