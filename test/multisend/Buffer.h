/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/multisend/Buffer.h
 * \brief Multicast test buffer support class
 */
#ifndef __test_multisend_Buffer_h__
#define __test_multisend_Buffer_h__

#include <stdio.h>

#include "sys/xmi.h"

#include "PipeWorkQueue.h"
#include "Topology.h"

#define DBG_FPRINTF(x) //fprintf x
#define DBGF_FUNCTIONNAME DBG_FPRINTF((stderr,"%.*s\n",_function_name_len(__PRETTY_FUNCTION__),_function_name(__PRETTY_FUNCTION__)))

namespace XMI
{
  namespace Test
  {
    char* _function_name(const char*name)
    {
      char* pName = strstr((char*)name,">::");
      pName += strlen(">::");
      return pName;
    }
    unsigned _function_name_len(const char*name)
    {
      char* pName = _function_name(name);
      char* pEndName = strstr(pName,"[with");
      return(pEndName - pName);
    }


    template <size_t T_BufSize>
    class Buffer
    {
    private:
      char                _source[T_BufSize];
      XMI::PipeWorkQueue  _srcPwq;
      XMI::PipeWorkQueue *_pSrcPwq;

      char                _destination[T_BufSize];
      XMI::PipeWorkQueue  _dstPwq;
      XMI::PipeWorkQueue *_pDstPwq;

    public:
      Buffer(bool isRoot  = false) :
      _pSrcPwq(&_srcPwq),
      _pDstPwq(&_dstPwq)
      {
        DBG_FPRINTF((stderr,"<%#8.8X>%s src %p/%p, dst %p/%p\n",(unsigned)this,__PRETTY_FUNCTION__, &_srcPwq, _source, &_dstPwq, _destination));
        reset(isRoot);
      }

      ~Buffer()
      {
      }
      XMI::PipeWorkQueue * srcPwq()
      {
        return _pSrcPwq;
      }
      XMI::PipeWorkQueue * dstPwq()
      {
        return _pDstPwq;
      }
      inline void reset(bool isRoot  = false)
      {
        DBGF_FUNCTIONNAME;
        return reset(&_srcPwq,&_dstPwq,isRoot);
      }

      inline void reset(XMI::PipeWorkQueue* src,
                        XMI::PipeWorkQueue* dst,
                        bool isRoot  = false)
      {
        DBGF_FUNCTIONNAME;
        DBG_FPRINTF((stderr,"<%#8.8X> src %p, dst %p\n",(unsigned)this,src, dst));
        src->configure(NULL, _source, sizeof(_source), sizeof(_source));
        src->reset();

        dst->configure(NULL, _destination, sizeof(_destination), 0);
        dst->reset();

        return setup(src, dst, isRoot);
      }

      inline void set(XMI::PipeWorkQueue* src,
                      XMI::PipeWorkQueue* dst)
      {
        DBGF_FUNCTIONNAME;
        DBG_FPRINTF((stderr,"<%#8.8X> src %p, dst %p\n",(unsigned)this,src, dst));
        _pSrcPwq = src;
        _pDstPwq = dst;
        return;
      }

      char* buffer()
      {
        return _source;
      }
      inline void setup(bool isRoot  = false)
      {
        DBGF_FUNCTIONNAME;
        return setup(&_srcPwq,&_dstPwq,isRoot);
      }

      inline void setup(XMI::PipeWorkQueue* src,
                        XMI::PipeWorkQueue* dst,
                        bool isRoot  = false,
                        size_t count = T_BufSize)
      {
        DBGF_FUNCTIONNAME;

        _pSrcPwq = src;
        _pDstPwq = dst;

        char* source      = _pSrcPwq->bufferToConsume();
        char* destination = _pDstPwq->bufferToProduce();
        DBG_FPRINTF((stderr, "src %p/%p, dst %p/%p\n",
                     src, source, dst, destination));

        size_t count_of_unsigneds = count/sizeof(unsigned);
        unsigned value = 0;
        xmi_result_t rc;
        size_t x;

        for(x = 0; x < count_of_unsigneds; ++x)
        {
          ((unsigned *)source)[x] = isRoot? value++ : -1;
          ((unsigned *)destination)[x] = -1;
        }
        return ;
      }

      inline xmi_result_t validate(size_t &bytesConsumed,
                                   size_t &bytesProduced,
                                   bool isRoot  = false,
                                   bool isDest  = true,
                                   size_t count = T_BufSize)
      {
        return validate(_pSrcPwq,
                        _pDstPwq,
                        bytesConsumed,
                        bytesProduced,
                        isRoot,
                        isDest,
                        count);
      }
      inline xmi_result_t validate(XMI::PipeWorkQueue* src,
                                   XMI::PipeWorkQueue* dst,
                                   size_t &bytesConsumed,
                                   size_t &bytesProduced,
                                   bool isRoot  = false,
                                   bool isDest  = true,
                                   size_t count = T_BufSize)
      {
        DBGF_FUNCTIONNAME;

        _pSrcPwq = src;
        _pDstPwq = dst;

        bytesConsumed = _pSrcPwq->getBytesConsumed();
        bytesProduced = _pDstPwq->getBytesProduced();

        _pSrcPwq->reset();
        _pDstPwq->reset();

        char* source      = _pSrcPwq->bufferToConsume();
        char* destination = _pDstPwq->bufferToProduce();

        DBG_FPRINTF((stderr, "src %p/%p, dst %p/%p, bytesConsumed %zd, bytesProduced %zd\n",
                     src, source, dst, destination, bytesConsumed, bytesProduced));

        unsigned errors = 0;
        unsigned value = 0;
        size_t count_of_unsigneds = count/sizeof(unsigned);
        size_t x;


        if(isRoot) // validate untouched src buffer
          for(x = 0; x < count_of_unsigneds && errors < 5 ; ++x)
          {
            if(((unsigned *)source)[x] != value++)
            {
              fprintf(stderr, "Corrupted source buffer(%d) at index %zd.\n",((unsigned *)source)[x], x);
              errors++;//break;
            }
          }
        else // validate untouched buffer (-1)
          for(x = 0; x < count_of_unsigneds  && errors < 5 ; ++x)
          {
            if(((unsigned *)source)[x] != (unsigned)-1)
            {
              fprintf(stderr, "Corrupted source buffer(%d) at index %zd.\n",((unsigned *)source)[x], x);
              errors++;//break;
            }
          }

        value = 0;
        // Validate the output data is correct
        if(isDest) // validate expected results
          for(x = 0; x < count_of_unsigneds && errors < 5 ; ++x)
          {
            if(((unsigned *)destination)[x] != value++)
            {
              fprintf(stderr, "Incorrect result buffer(%d) at index %zd.\n",((unsigned *)destination)[x], x);
              errors++;//break;
            }
          }
        else // validate untouched buffer (-1)
          for(x = 0; x < count_of_unsigneds && errors < 5 ; ++x)
          {
            if(((unsigned *)destination)[x] != (unsigned)-1)
            {
              fprintf(stderr, "Corrupted result buffer(%d) at index %zd.\n",((unsigned *)destination)[x], x);
              errors++;//break;
            }
          }
        if(errors) //(x - errors) < count_of_unsigneds)
        {
          fprintf(stderr, "FAIL validation %d\n",errors);
          return XMI_ERROR;
        }
        fprintf(stderr, "PASS validation\n");
        return XMI_SUCCESS;
      }
    }; // class Buffer

  }; // namespace Test
}; // namespace XMI

#endif // __test_multisend_buffer_h__
