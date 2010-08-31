#ifndef _PAMI_TYPE_CODE_H
#define _PAMI_TYPE_CODE_H

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
            TypeCode(bool to_optimize);
            TypeCode(void *code_addr, size_t code_size);
            TypeCode(size_t code_size);
            ~TypeCode();

            void AddShift(size_t shift);
            void AddSimple(size_t bytes, size_t stride, size_t reps);
            void AddTyped(TypeCode *sub_type, size_t stride, size_t reps);
            void Complete();

            bool IsCompleted() const;
            bool IsContiguous() const;

            void * GetCodeAddr() const;
            size_t GetCodeSize() const;
            size_t GetDataSize() const;
            size_t GetExtent() const;
            size_t GetDepth() const;
            size_t GetNumBlocks() const;
            size_t GetUnit() const;
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
                size_t  code_size;
                size_t  depth;
                size_t  data_size;
                size_t  extent;
                size_t  num_blocks;
                size_t  unit;
                size_t  atom_size;

                Begin() 
                    : Op(BEGIN), code_size(0), depth(1), data_size(0), extent(0),
                    num_blocks(0), unit(0), atom_size(0) { }

                void Show(int pc) const {
                    printf("%4d: Begin code_size %zu depth %zu data_size %zu "
                            "extent %zu num_blocks %zu unit %zu atom_size %zu\n",
                            pc, code_size, depth, data_size, extent, num_blocks, 
                            unit, atom_size);
                }
            };

            struct Copy : Op {
                size_t  bytes;
                size_t  stride;
                size_t  reps;

                Copy(size_t bytes, size_t stride, size_t reps)
                    : Op(COPY), bytes(bytes), stride(stride), reps(reps) { }

                void Show(int pc) const {
                    printf("%4d: Copy bytes %zu stride %zu reps %zu\n",
                            pc, bytes, stride, reps);
                }
            };

            struct Call : Op {
                size_t  sub_type;
                size_t  stride;
                size_t  reps;

                Call(size_t sub_type, size_t stride, size_t reps)
                    : Op(CALL), sub_type(sub_type), stride(stride), reps(reps) { }

                void Show(int pc) const {
                    printf("%4d: Call sub %zu stride %zu reps %zu\n",
                            pc, pc + sub_type, stride, reps);
                }
            };

            struct Shift : Op {
                size_t  shift;

                Shift(size_t shift) : Op(SHIFT), shift(shift) { }

                void Show(int pc) const {
                    printf("%4d: Shift %zu\n", pc, shift);
                }
            };

            struct End : Op {
                End() : Op(END) { }

                void Show(int pc) const {
                    printf("%4d: End\n", pc);
                }
            };

            char  *code;
            size_t code_buf_size;
            size_t code_cursor;
            bool   completed;
            bool   to_optimize;

            void CheckCodeBuffer(size_t inc_code_size);
            void ResizeCodeBuffer(size_t new_size);
            void AddCodeSize(size_t inc_code_size);
            void AddDataSize(size_t inc_data_size);
            void AddExtent(size_t inc_extent);
            void UpdateDepth(size_t call_depth);
            void AddNumBlocks(size_t inc_num_blocks);
            void UpdateUnit(size_t new_unit);
            void CopySubTypes();
    };

    inline TypeCode::TypeCode(bool to_optimize = false)
        : code(NULL), code_buf_size(0), code_cursor(0), completed(false),
        to_optimize(to_optimize)
    {
        ResizeCodeBuffer(sizeof(Begin) + sizeof(Copy)*4);
        *(Begin *)code = Begin();
        code_cursor += sizeof(Begin);

        AddCodeSize(sizeof(Begin));
    }

    inline TypeCode::TypeCode(void *code_addr, size_t code_size)
        : code(NULL), code_buf_size(0), code_cursor(0), completed(true),
        to_optimize(false)
    {
        ResizeCodeBuffer(code_size);
        memcpy(code, code_addr, code_size);
    }

    inline TypeCode::TypeCode(size_t code_size)
        : code(NULL), code_buf_size(0), code_cursor(0), completed(true),
        to_optimize(false)
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
      // Begin &begin = *(Begin *)code;
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

    inline void   TypeCode::SetAtomSize(size_t atom_size)
    {
        assert(GetUnit() % atom_size == 0);
        ((Begin *)code)->atom_size = atom_size;
    }

    inline bool TypeCode::IsCompleted() const
    {
        return completed;
    }

    inline bool TypeCode::IsContiguous() const
    {
        return GetDataSize() == GetExtent();
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

    inline void TypeCode::UpdateDepth(size_t call_depth)
    {
        size_t &depth = ((Begin *)code)->depth;
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
            CheckCodeBuffer(sizeof(Shift));
            *(Shift *)(code + code_cursor) = Shift(shift);
            code_cursor += sizeof(Shift);
            AddCodeSize(sizeof(Shift));
            AddExtent(shift);
        }
    }

    inline void TypeCode::AddSimple(size_t bytes, size_t stride, size_t reps)
    {
        assert(!IsCompleted());

        if (reps > 0) {
            if (to_optimize && bytes == stride) {
                stride  = bytes *= reps;
                reps    = 1;
            } 
            CheckCodeBuffer(sizeof(Copy));
            *(Copy *)(code + code_cursor) = Copy(bytes, stride, reps);
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
            CheckCodeBuffer(sizeof(Call));
            *(Call *)(code + code_cursor) = Call((size_t)sub_type, stride, reps);
            code_cursor += sizeof(Call);

            AddCodeSize(sizeof(Call) + sub_type->GetCodeSize());
            AddDataSize(sub_type->GetDataSize() * reps);
            UpdateDepth(sub_type->GetDepth() + 1);
            AddNumBlocks(sub_type->GetNumBlocks()*reps);
            UpdateUnit(sub_type->GetUnit());
            AddExtent(stride * reps);
        }
    }

    inline void TypeCode::Complete()
    {
        assert(!IsCompleted());

        CheckCodeBuffer(sizeof(End));
        *(End *)(code + code_cursor) = End();
        code_cursor += sizeof(End);

        AddCodeSize(sizeof(End));

        assert(code_cursor <= GetCodeSize());
        if (code_cursor < GetCodeSize())
            CopySubTypes();

        completed = true;
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
    }

    class TypeContig : public TypeCode
    {
        public:
            TypeContig();
            ~TypeContig();
    };

    inline TypeContig::TypeContig()
    {
        AddSimple(ULONG_MAX, ULONG_MAX, 1);
        Complete();
        SetAtomSize(1);
        AcquireReference();
    }

    inline TypeContig::~TypeContig()
    {
        assert(!"Can't delete TypeContig");
    }

  }
}

#endif // _PAMI_TYPE_CODE_H

