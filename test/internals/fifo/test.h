/**
 * \file test/internals/fifo/test.h
 * \brief ???
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "components/fifo/FifoInterface.h"
#include "components/fifo/PacketInterface.h"


class Consumer : public PAMI::Fifo::Interface::PacketConsumer <Consumer>
{
  public:

    friend class PAMI::Fifo::Interface::PacketConsumer <Consumer>;

    Consumer (const char * name = NULL) :
        _name (name)
    {
    };

  protected:

    template <class T_FifoPacket>
    inline bool consume_impl (T_FifoPacket & packet)
    {
      void * header  = packet.getHeader();
      size_t from = *((size_t *)header);

      fprintf (stdout, "[%s] - received a packet from %zu\n", _name, from);

      return true;
    };

    const char * _name;
};

class Producer : public PAMI::Fifo::Interface::PacketProducer <Producer>
{


  public:

    friend class PAMI::Fifo::Interface::PacketProducer <Producer>;

    inline Producer (size_t value = (size_t) - 1) :
        _value (value)
    {};

  protected:

    template <class T_Packet>
    inline bool produce_impl (T_Packet & packet)
    {
      void * header = packet.getHeader ();
      *((size_t *) header) = _value;

      return true;
    };

    template <class T_Packet>
    inline bool produce_impl (T_Packet & packet, bool & done)
    {
      produce_impl (packet);
      done = true;
      return true;
    };

    size_t _value;

};


template <class T_Fifo>
class Test
{

  public:

    template <class T_MemoryManager>
    inline void init (T_MemoryManager * mm, size_t task, size_t size)
    {
      _task = task;
      _size = size;

      size_t fnum;

      for (fnum = 0; fnum < 64; fnum++)
        {
          char fifokey[1024];
          snprintf (fifokey, 1023, "/fifo-%zu", fnum);
          _ififo[fnum].initialize (mm, fifokey, 64, fnum);
        }

      _rfifo.initialize (_ififo[_task]);
    };


    void functional (const char * name = NULL)
    {
      Producer producer(_task);
      Consumer consumer(name);

      if (_task != 0)
        {
          while (! _ififo[0].producePacket(producer));

          //size_t sequence = _ififo[0].lastPacketProduced();
          //fprintf (stdout, "[%s] produced packet %zu into fifo 0\n", name, sequence);

          // bug!
          //while (ififo[0].lastPacketConsumed() < sequence);

          //fprintf (stdout, "packet %zu was consumed\n", sequence);
        }
      else
        {
          size_t expected = _size - 1;

          while (expected > 0)
            {
              if (_rfifo.consumePacket(consumer))
                {
                  //fprintf (stdout, "[%s] consumed packet %zu\n", name, _rfifo.lastPacketConsumed());
                  expected--;
                }
            }
        }

    };

  protected:

    T_Fifo _ififo[64];
    T_Fifo _rfifo;
    size_t _task;
    size_t _size;
};


