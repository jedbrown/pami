/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/internals/multisend/mpi/multicast.h
 * \brief Multicast test support class
 */
#ifndef __xmi_test_internals_multisend_mpi_multicast_h__
#define __xmi_test_internals_multisend_mpi_multicast_h__
#include <stdio.h>
#include "sys/xmi.h"

#include "PipeWorkQueue.h"
#include "Topology.h"
#include "Global.h"

#define DBG_FPRINTF(x) 
//#define DBG_FPRINTF(x) fprintf x

namespace XMI
{
  namespace MPI
  {
    namespace Test
    {
      namespace Multisend
      {

        template <class T_Model, class T_Device, size_t T_BufSize>
        class Multicast
        {

        public:

          XMI::PipeWorkQueue _ipwq;
          XMI::PipeWorkQueue _opwq;

        private:
          T_Device _device;
          T_Model _model;

          char _source[T_BufSize];
          char _destination[T_BufSize];


          xmi_result_t _status;
          volatile int _doneCountdown;
          char *_name;
          size_t _task_id;

          static void _done_cb(xmi_context_t context, void *cookie, xmi_result_t result)
          {
            XMI::MPI::Test::Multisend::Multicast<T_Model, T_Device,T_BufSize> *thus =
            (XMI::MPI::Test::Multisend::Multicast<T_Model, T_Device,T_BufSize> *)cookie;
            DBG_FPRINTF((stderr, "... completion callback for %s, done %d ++\n", thus->_name, thus->_doneCountdown));
            --thus->_doneCountdown;
          }

        public:
          Multicast(size_t& task_id) :
          _device(),
          _model(0,NULL,_device),
          _status(XMI_SUCCESS),
          _doneCountdown(0),
          _task_id(task_id)
          {
            DBG_FPRINTF((stderr,"%s\n",__PRETTY_FUNCTION__));
            DBG_FPRINTF((stderr,"%s\n",function_name(__PRETTY_FUNCTION__)));
            format_class_name((char*)__PRETTY_FUNCTION__); 

            if(_status != XMI_SUCCESS)
            {
              fprintf(stderr, "Failed to register multicast \"%s\" %zd\n", _name, (size_t)_status);
            }
            _ipwq.configure(NULL, _source, sizeof(_source), sizeof(_source));
            _ipwq.reset();

            _opwq.configure(NULL, _destination, sizeof(_destination), 0);
            _opwq.reset();



          }

          ~Multicast()
          {
          }

          inline xmi_result_t reset_test(size_t task_id, 
                                         size_t num_tasks,
                                         xmi_multicast_t *mcast)
          {
            DBG_FPRINTF((stderr,"%.*s\n",function_name_len(__PRETTY_FUNCTION__),function_name(__PRETTY_FUNCTION__)));
            if(_status != XMI_SUCCESS)
            {
              fprintf(stderr, "Failed to register multicast \"%s\" %zd\n", _name, (size_t)_status);
            }
            _ipwq.configure(NULL, _source, sizeof(_source), sizeof(_source));
            _ipwq.reset();

            _opwq.configure(NULL, _destination, sizeof(_destination), 0);
            _opwq.reset();

            return XMI_SUCCESS;
          }
          inline xmi_result_t setup_test(size_t task_id, 
                                         size_t num_tasks,
                                         xmi_multicast_t *mcast)
          {
            DBG_FPRINTF((stderr,"%.*s bytes %u\n",function_name_len(__PRETTY_FUNCTION__),function_name(__PRETTY_FUNCTION__),mcast->bytes));
            if(_status != XMI_SUCCESS)
            {
              fprintf(stderr, "Failed to register multicast \"%s\"\n", _name);
              return XMI_ERROR;
            }

            // caller may have switched pwq's, so don't assume _source/_destination.            
            XMI::PipeWorkQueue * ipwq = (XMI::PipeWorkQueue*)mcast->src;
            XMI::PipeWorkQueue * opwq = (XMI::PipeWorkQueue*)mcast->dst;
            char* source = ipwq->bufferToConsume();
            char* destination = opwq->bufferToProduce();

//          ipwq->configure(NULL, source, mcast->bytes, mcast->bytes);
//          ipwq->reset();
//          opwq->configure(NULL, destination, mcast->bytes, 0);
//          opwq->reset();

            // simple bcast on the local ranks...
//            mcast->request = malloc(T_Model::sizeof_msg); 
            mcast->cb_done = (xmi_callback_t)
            {
              _done_cb, (void *)this
            };

            size_t count = mcast->bytes / sizeof(unsigned);
            unsigned value = 0;
            xmi_result_t rc;
            size_t x;

            bool isRoot = (((XMI::Topology*)(mcast->src_participants))->isRankMember(_task_id));
            for(x = 0; x < count; ++x)
            {
              ((unsigned *)source)[x] = isRoot? value++ : -1;
              ((unsigned *)destination)[x] = -1;
            }
            return XMI_SUCCESS;
          }

          inline xmi_result_t perform_test(size_t task_id, 
                                           size_t num_tasks,
                                           xmi_multicast_t *mcast)
          {
            xmi_result_t rc;

            DBG_FPRINTF((stderr,"%.*s\n",function_name_len(__PRETTY_FUNCTION__),function_name(__PRETTY_FUNCTION__)));
            if(_status != XMI_SUCCESS)
            {
              fprintf(stderr, "Failed to register multicast \"%s\"\n", _name);
              return XMI_ERROR;
            }

            if(task_id == 0) fprintf(stderr, "Performing %s\n", _name);

            sleep(1);

            ++_doneCountdown;

            bool res = _model.postMulticast(mcast);
            if(!res)
            {
              fprintf(stderr, "Failed to post multicast \"%s\"\n", _name);
              return XMI_ERROR;
            }

            while(_doneCountdown)
            {
              int events = _device.advance_impl();
            }

//            free(mcast->request);

            return XMI_SUCCESS;
          }


          inline xmi_result_t start_test(size_t task_id, 
                                         size_t num_tasks,
                                         xmi_multicast_t *mcast)
          {
            xmi_result_t rc;

            DBG_FPRINTF((stderr,"%.*s\n",function_name_len(__PRETTY_FUNCTION__),function_name(__PRETTY_FUNCTION__)));
            if(_status != XMI_SUCCESS)
            {
              fprintf(stderr, "Failed to register multicast \"%s\"\n", _name);
              return XMI_ERROR;
            }

            if(task_id == 0) fprintf(stderr, "Starting %s\n", _name);

            sleep(1);

            ++_doneCountdown;

            bool res = _model.postMulticast(mcast);
            if(!res)
            {
              fprintf(stderr, "Failed to post multicast \"%s\"\n", _name);
              return XMI_ERROR;
            }
            return XMI_SUCCESS;
          }


          inline xmi_result_t finish_test(size_t task_id, 
                                          size_t num_tasks,
                                          xmi_multicast_t *mcast)
          {
            DBG_FPRINTF((stderr,"%.*s\n",function_name_len(__PRETTY_FUNCTION__),function_name(__PRETTY_FUNCTION__)));
            if(task_id == 0) fprintf(stderr, "Finishing %s\n", _name);
            while(_doneCountdown)
            {
              int events = _device.advance_impl();
            }
            return XMI_SUCCESS;
          }



          inline xmi_result_t validate_test(size_t task_id, 
                                            size_t num_tasks,
                                            xmi_multicast_t *mcast)
          {
            DBG_FPRINTF((stderr,"%.*s\n",function_name_len(__PRETTY_FUNCTION__),function_name(__PRETTY_FUNCTION__)));
            if(_status != XMI_SUCCESS)
            {
              fprintf(stderr, "Failed to register multicast \"%s\"\n", _name);
              return XMI_ERROR;
            }

            // caller may have switched pwq's, so don't assume _source/_destination.            
            XMI::PipeWorkQueue * ipwq = (XMI::PipeWorkQueue*)mcast->src;
            XMI::PipeWorkQueue * opwq = (XMI::PipeWorkQueue*)mcast->dst;
            ipwq->reset();
            opwq->reset();
            char* source = ipwq->bufferToConsume();
            char* destination = opwq->bufferToProduce();

            unsigned errors = 0;
            unsigned value = 0;
            size_t count = mcast->bytes / sizeof(unsigned);
            size_t x;


            if(((XMI::Topology*)(mcast->src_participants))->isRankMember(_task_id)) // validate untouched buffer
              for(x = 0; x < count && errors < 5 ; ++x)
              {
                if(((unsigned *)source)[x] != value++)
                {
                  fprintf(stderr, "Corrupted source buffer(%d) at index %zd.\n",((unsigned *)source)[x], x);
                  errors++;//break;
                }
              }
            else // validate untouched buffer (-1)
              for(x = 0; x < count  && errors < 5 ; ++x)
              {
                if(((unsigned *)source)[x] != (unsigned)-1)
                {
                  fprintf(stderr, "Corrupted source buffer(%d) at index %zd.\n",((unsigned *)source)[x], x);
                  errors++;//break;
                }
              }

            value = 0;
            // Validate the output data is correct
            if(((XMI::Topology*)(mcast->dst_participants))->isRankMember(_task_id)) // validate expected results
              for(x = 0; x < count && errors < 5 ; ++x)
              {
                if(((unsigned *)destination)[x] != value++)
                {
                  fprintf(stderr, "Incorrect result buffer(%d) at index %zd.\n",((unsigned *)destination)[x], x);
                  errors++;//break;
                }
              }
            else // validate untouched buffer (-1)
              for(x = 0; x < count  && errors < 5 ; ++x)
              {
                if(((unsigned *)destination)[x] != (unsigned)-1)
                {
                  fprintf(stderr, "Corrupted result buffer(%d) at index %zd.\n",((unsigned *)destination)[x], x);
                  errors++;//break;
                }
              }
            if((x - errors) < count)
            {
              fprintf(stderr, "Failed %s test\n", _name);
              return XMI_ERROR;
            }
            fprintf(stderr, "PASS %s\n", _name);
            return XMI_SUCCESS;
          }

        private:
// DEBUG/pretty formatting utilities because I HATE the long templated names
#if defined(__xlc__) || defined(__xlC__)
          void format_class_name(char* ctor_fname)
          {
            char pname[512]; strncpy(pname,ctor_fname,511);
            char* pclass = strtok(pname, "<");
            char* pmodel = pname + strlen(pclass) + 1;
            char* pdevice=pmodel;
            int nest = 0;
            while(*pdevice != '\0')
            {
              if((nest == 0) && (*pdevice == ','))
              {
                *pdevice++='\0'; break;
              }
              if(*pdevice == '<')
              {
                *pdevice='\0';++nest;
              }
              if(*pdevice == '>')
              {
                *pdevice='\0';--nest;
              }
              pdevice++;
            }
            pdevice = strtok(pdevice,"<");
            unsigned class_len = strlen(pclass);
            unsigned device_len = strlen(pdevice);
            unsigned model_len = strlen(pmodel);
            _name = (char*)malloc(class_len + 1 + device_len + 1 + model_len + 1 + 9 + 1 + 1);
            char* pcopy = _name;
            strncpy(pcopy, pclass, class_len); pcopy+=class_len;
            *pcopy = '<'; pcopy++;
            strncpy(pcopy, pmodel, model_len); pcopy+=model_len;
            *pcopy = ','; pcopy++;
            strncpy(pcopy, pdevice, device_len); pcopy+=device_len;
            *pcopy = ','; pcopy++;
            sprintf(pcopy, "%-9u ",T_BufSize); 
            char* pspace = strstr(pcopy," "); 
            *pspace++ = '>'; *pspace++ = '\0'; pcopy = pspace;
            DBG_FPRINTF((stderr,"%d/%d-%s\n",(class_len + 1 + device_len + 1 + model_len + 1 + 9 + 1 + 1),strlen(_name),_name));
            return;
          }
#elif defined(__GNUC__)
          void format_class_name(char* ctor_fname)
          {
            char *pname=ctor_fname;
            char* pclass = pname; unsigned class_len = strstr(pclass, "<") - pname;
            char* pdevice = strstr(pname, "T_Device = ") + strlen("T_Device = "); unsigned device_len = strstr(pdevice, "<") - pdevice;
            char* pmodel = strstr(pname,"T_Model = ") + strlen("T_Model = "); unsigned model_len = strstr(pmodel, "<") - pmodel;
            _name = (char*)malloc(class_len + 1 + device_len + 1 + model_len + 1 + 9 + 1 + 1);
            char* pcopy = _name;
            strncpy(pcopy, pclass, class_len); pcopy+=class_len;
            *pcopy = '<'; pcopy++;
            strncpy(pcopy, pmodel, model_len); pcopy+=model_len;
            *pcopy = ','; pcopy++;
            strncpy(pcopy, pdevice, device_len); pcopy+=device_len;
            *pcopy = ','; pcopy++;
            sprintf(pcopy, "%-9u ",T_BufSize); 
            char* pspace = strstr(pcopy," "); 
            *pspace++ = '>'; *pspace++ = '\0'; pcopy = pspace;
            DBG_FPRINTF((stderr,"%d/%d-%s\n",(class_len + 1 + device_len + 1 + model_len + 1 + 9 + 1 + 1),strlen(_name),_name));
            return;
          }
#else // !__xlc__ || __xlC__ || __GNUC__
          void format_class_name(char* ctor_fname)
          {
            _name = ctor_fname;
            DBG_FPRINTF((stderr,_name));
            return;
          }
#endif
          char* function_name(const char*name)
          {
            char* pName = strstr(name,">::");
            pName += strlen(">::");
            return pName;
          }
          unsigned function_name_len(const char*name)
          {
            char* pName = function_name(name);
            char* pEndName = strstr(pName,"[with");
            return(pEndName - pName);
          }

        }; // class Multicast
      }; // namespace Multisend
    }; // namespace Test
  }; // namespace MPI
}; // namespace XMI

#define DBG_PRINTFS_1

#ifndef DBG_PRINTFS_1
  #define DBG_PRINTFS_1 \
  DBG_FPRINTF((stderr,"root %d, rank %d, size %d\n",          \
          root, task_id, tSize));                             \
  if(task_id != root)                                         \
  {                                                           \
    DBG_FPRINTF((stderr,"Not the root\n"));                   \
  }                                                           \
  else DBG_FPRINTF((stderr,"The root\n"))
#endif

#define DBG_PRINTFS_2

#ifndef DBG_PRINTFS_2
  #define DBG_PRINTFS_2                                       \
  if(otopo.isLocalToMe())                                     \
  {                                                           \
    DBG_FPRINTF((stderr,"Is local to me\n"));                 \
  }                                                           \
  else DBG_FPRINTF((stderr,"Is NOT local to me\n"));          \
                                                              \
  if(otopo.isRankMember(task_id))                             \
  {                                                           \
    DBG_FPRINTF((stderr,"Is member (%d)\n",task_id));         \
    DBG_FPRINTF((stderr," otopo size %d, ranks %d, %d, %d, %d\n",  \
            lSize,                                            \
            otopo.index2Rank(0),                              \
            otopo.index2Rank(1),                              \
            otopo.index2Rank(2),                              \
            otopo.index2Rank(3)));                            \
  }                                                           \
  else DBG_FPRINTF((stderr,"Is NOT member (%d)\n",task_id))
#endif

#endif // __xmi_test_internals_multisend_mpi_multicast_h__


