/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2010                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file common/socklinux/Mapping.h
 * \brief ???
 */

#ifndef __common_socklinux_Mapping_h__
#define __common_socklinux_Mapping_h__

#include "Platform.h"
#include "util/common.h"
#include "common/BaseMappingInterface.h"
#include "common/UdpSocketMappingInterface.h"
#include "common/SocketMappingInterface.h"
#include "common/NodeMappingInterface.h"
#include "sys/xmi.h"
#include <errno.h>
#include <unistd.h>
//#include <pmi.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <arpa/inet.h>
#include <netdb.h>

#define XMI_MAPPING_CLASS XMI::Mapping

namespace XMI
{
    class Mapping : public Interface::Mapping::Base<Mapping>,
                       public Interface::Mapping::UdpSocket<Mapping>,
                       public Interface::Mapping::Socket<Mapping>,
                       public Interface::Mapping::Node<Mapping, 1>
    {

    public:
      inline Mapping () :
        Interface::Mapping::Base<Mapping>(),
        Interface::Mapping::UdpSocket<Mapping>(),
        Interface::Mapping::Socket<Mapping>(),
        Interface::Mapping::Node<Mapping, 1>()
      {
      };
      inline ~Mapping ()
      {
        free( _tcpConnTable );
        free( _udpConnTable );
      };

      inline bool isUdpActive()
      {
        return  _udpConnInit;
      }

      inline int activateUdp()
      {
        char * udp_config;
        std::ifstream inFile;
        size_t tmp_task;
        std::string tmp_host_in;
        std::string tmp_port;
        int sockFd;
        int rc;
        struct addrinfo hints, *servinfo;
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_INET;  // AF_UNSPEC for 6 support too
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags |= AI_PASSIVE;

        _udpConnInit = true;
        // Allocate space for the connection table
        _udpConnTable = (udp_conn_t *)malloc(_size * sizeof(udp_conn_t));

        size_t i;
        for ( i=0; i<_size; i++) {
          _udpConnTable[i].send_fd = 0;
        }

        udp_config = getenv ("XMI_UDP_CONFIG");
        if (udp_config == NULL ) {
          std::cout << "Environment variable XMI_UDP_CONFIG must be set" << std::endl;
          abort();
        }
        std::cout << "The current UDP configuration file is: " << udp_config << std::endl;

        // Now open the configuration file
        inFile.open(udp_config);
        if (!inFile )
        {
          std::cout << "Unable to open UDP configuration file: " << udp_config << std::endl;
          abort();
        }

        typedef struct
        {
          char     host[128];
          unsigned peers;
        } node_table_t;
        node_table_t * tmpnodetable = (node_table_t *) malloc (_size * sizeof(node_table_t));
        for (i=0; i<_size; i++)
        {
          tmpnodetable[i].host[0] = NULL;
          tmpnodetable[i].peers   = 0;
        }
        size_t num_global_nodes = 0;

        // Read in the configuration file: rank host port
        for ( i=0; i<_size; i++ )
        {

          inFile >> tmp_task >> tmp_host_in >> tmp_port;
          std::cout << "  Entry: " << tmp_task << " " << tmp_host_in << " " << tmp_port << std::endl;

          // Make sure we can locate the host
          struct hostent *tmp_host;
          tmp_host = gethostbyname(tmp_host_in.data());
          if (tmp_host == NULL )
          {
            std::cout << "Unable to get host by name.  Name in config file: " << tmp_host_in << std::endl;
            abort();
          }

          if ( (rc = getaddrinfo( tmp_host_in.data(), tmp_port.data(), &hints, &servinfo ) ) != 0 )
          {
            std::cout << "getaddrinfo call failed:" << gai_strerror(rc) << std::endl;
          }
          if ( (sockFd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol ) ) == -1 )
          {
            std::cout << "socket call failed" << std::endl;
          }
          //std::cout << "addr " << servinfo->ai_addr << " len  " << servinfo->ai_addrlen << std::endl;
          //std::cout << "ai_canonname " << servinfo->ai_canonname << std::endl;

          unsigned j;
          for (j=0; j<_size; j++)
          {
            if ( strcmp( tmpnodetable[j].host, tmp_host->h_name ) == 0 )
            {
              // Found a previous host entry
              _udpConnTable[tmp_task].node_addr.global = j;
              _udpConnTable[tmp_task].node_addr.local  = tmpnodetable[j].peers;
              tmpnodetable[j].peers++;
              break;
            }
          }
          if (j == _size)
          {
            // Did not find a previous host entry
            _udpConnTable[tmp_task].node_addr.global = num_global_nodes;
            _udpConnTable[tmp_task].node_addr.local  = 0;
            tmpnodetable[num_global_nodes].peers = 1;
            strncpy (tmpnodetable[num_global_nodes].host, tmp_host->h_name, 127);
            num_global_nodes++;
          }



          if (_task == tmp_task)
          {
            char host[128];
            int str_len=128;
            int err = gethostname( host, str_len );
            // std::cout << host << std::endl;
            if ( strcmp( host, tmp_host->h_name ) != 0 )
            {
              std::cout << "Entry for this node" << _task << " is incorrect: " << std::endl;
              std::cout << "     " << host << " does not match " << tmp_host->h_name << std::endl;
            }
            // Set up to receive
            _udpRcvConn = sockFd;
            if ( bind(_udpRcvConn, servinfo->ai_addr, servinfo->ai_addrlen ) == -1)
            {
              close (_udpRcvConn);
              std::cout << "bind call failed" << std::endl;
              abort();
            }

          } else {
            // Save info for sending
          _udpConnTable[tmp_task].send_fd =  sockFd;
          memcpy( &(_udpConnTable[tmp_task].send_addr), servinfo->ai_addr, servinfo->ai_addrlen );
          _udpConnTable[tmp_task].send_addr_len = servinfo->ai_addrlen;
          }
        }
        inFile.close();

        std::cout << "num_global_nodes: " << num_global_nodes << std::endl;
        for (i=0; i<num_global_nodes; i++)
        {
          std::cout << "node[" << i << "].host:  " << tmpnodetable[i].host << std::endl;
          std::cout << "node[" << i << "].peers: " << tmpnodetable[i].peers << std::endl;
        }

        _peers = tmpnodetable[_udpConnTable[_task].node_addr.global].peers;

        free (tmpnodetable);

        return 0;
      }

      inline int deactivateUdp()
      {
        size_t i;
        _udpConnInit = false;
        close( _udpRcvConn );
        for (i=0; i<_size; i++ )
        {
          close( _udpConnTable[i].send_fd );
        }
        return 0;
      }

      inline bool isTcpActive()
      {
        return  _tcpConnInit;
      }

      inline int activateTcp()
      {
        _tcpConnInit = true;
        // Allocate space for the connection table
        _tcpConnTable = (xmi_coord_t *)malloc(_size * sizeof(xmi_coord_t));

        // All are sockets entries
        size_t i;
        for ( i=0; i<_size; i++) {
          _tcpConnTable[i].network = XMI_SOCKET_NETWORK;
        }
        std::cout << "TCP not supported yet " << std::endl;
        abort ();
        return 0;
      }

     inline int deactivateTcp()
        {
          _tcpConnInit = false;
          return 0;
        }

    protected:
	size_t            _task;
	size_t            _size;
        size_t            _peers;
        // static const int  __pmiNameLen = 128;
        // char              __pmiName[__pmiNameLen];

        typedef struct
        {
          int                            send_fd;
          sockaddr_storage               send_addr;
          int                            send_addr_len;
          Interface::Mapping::nodeaddr_t node_addr;
        } udp_conn_t;

        xmi_coord_t *     _tcpConnTable;
        bool              _tcpConnInit;
        udp_conn_t *      _udpConnTable;
        int               _udpRcvConn;
        bool              _udpConnInit;


	static int rank_compare(const void *a, const void *b) {
		size_t *aa = (size_t *)a, *bb = (size_t *)b;
		size_t r1 = *aa;
		size_t r2 = *bb;
		int x = r1 - r2;
		return x;
	}
    public:
      inline xmi_result_t init(size_t &min_rank, size_t &max_rank,
				size_t &num_local, size_t **local_ranks)
      {
        char * tmp;
        tmp = getenv("XMI_SOCK_SIZE");
        if (tmp == NULL ) {
          std::cout << "Environment variable XMI_SOCK_SIZE must be set" << std::endl;
          abort();
        }
        _size = strtoul( tmp, NULL, 0 );
        std::cout << "Size = " << _size << std::endl;
        tmp = getenv("XMI_SOCK_TASK");
        if (tmp == NULL ) {
          std::cout << "Environment variable XMI_SOCK_TASK must be set" << std::endl;
          abort();
        }
        _task = strtoul( tmp, NULL, 0 );
        std::cout << "Task = " << _task << std::endl;
        if ( _task >= _size )
        {
          std::cout << "Task " << _task << "is >= size " << _size << std::endl;
          abort();
        }

        // None of the ranks are local
        //*local_ranks = xxxx;
        num_local = 0;

	min_rank = 0;
	max_rank = _size-1;

	return XMI_SUCCESS;
      }

      inline size_t task_impl()
        {
          return _task;
        }

      inline size_t size_impl()
        {
          return _size;
        }

      inline xmi_result_t task2network (size_t task, xmi_coord_t *addr, xmi_network type)
      {
        std::cout << "task2network not supported" << std::endl;
        abort();
      }

      inline xmi_result_t network2task_impl(const xmi_coord_t *addr,
						size_t *task,
						xmi_network *type)
      {
        std::cout << "network2task not supported" << std::endl;
        abort();
      }

      inline void udpAddr_impl(int & recvfd) const
      {
	recvfd = _udpRcvConn;
      }

     inline xmi_result_t task2udp_impl( size_t task, int & sendfd, void * sockAddr, int & len )
     {
       if ( task >= _size )
       {
         return XMI_ERROR;
       }
       sendfd = _udpConnTable[task].send_fd;
       len = _udpConnTable[task].send_addr_len;
       memcpy( sockAddr, &(_udpConnTable[task].send_addr), len );
       return XMI_SUCCESS;
     }

     inline xmi_result_t udp2task_impl( int sendfd, void * sockAddr, int len , size_t & task )
     {
       abort();
     }

     inline void socketAddr_impl (size_t & recv_fd, size_t & send_fd) { abort(); }

     inline xmi_result_t task2socket_impl (size_t task, size_t & recv_fd, size_t & send_fd) { abort(); }

     inline xmi_result_t socket2task_impl (size_t recv_fd, size_t send_fd, size_t & task) { abort(); }

     inline size_t globalDims()
     {
          return 1;
     }

     ///
     /// \brief Get the number of possible tasks on any node
     ///
     inline xmi_result_t nodeTasks_impl (size_t global, size_t & tasks)
     {
       XMI_abort();
       return XMI_ERROR;
     };

     ///
     /// \brief Get the number of peer tasks on the local node
     /// \see XMI::Interface::Mapping::Node::nodePeers()
     ///
     inline xmi_result_t nodePeers_impl (size_t & peers)
     {
       peers = _peers;
       return XMI_SUCCESS;
     }

     ///
     /// \brief Determines if two tasks are located on the same node
     ///
     inline bool isPeer_impl (size_t task1, size_t task2)
     {
       return (_udpConnTable[task1].node_addr.global == _udpConnTable[task2].node_addr.global);
     }

     ///
     /// \brief Node address for the local task
     ///
     /// \param[out] address Node address
     ///
     inline void nodeAddr_impl (Interface::Mapping::nodeaddr_t & address)
     {
       address = _udpConnTable[_task].node_addr;
     }

     ///
     /// \brief Node address for a specific task
     ///
     /// The global task identifier monotonically increases from zero to
     /// XMI::Mapping::Interface::Base.size() - 1.
     ///
     /// \param[in]  task    Global task identifier
     /// \param[out] address Node address
     ///
     inline xmi_result_t task2node_impl (size_t task, Interface::Mapping::nodeaddr_t & address)
     {
       address = _udpConnTable[task].node_addr;
       return XMI_SUCCESS;
     }

     ///
     /// \brief Global task identifier associated with a specific node address
     ///
     /// The global task identifier monotonically increases from zero to
     /// XMI::Mapping::Interface::Base.size() - 1.
     ///
     /// \param[in]  address Node address
     /// \param[out] task    Global task identifier
     ///
     inline xmi_result_t node2task_impl (Interface::Mapping::nodeaddr_t & address, size_t & task)
     {
       XMI_abort();
       return XMI_ERROR;
     }

     ///
     /// \brief Peer identifier associated with a specific node address
     ///
     /// The local peer identifier monotonically increases from zero to
     /// XMI::Mapping::Interface::Node.nodePeers() - 1.
     ///
     /// \param[in]  address Node address
     /// \param[out] peer    peer identifier
     ///
     inline xmi_result_t node2peer_impl (Interface::Mapping::nodeaddr_t & address, size_t & peer)
     {
       peer = address.local;
       return XMI_SUCCESS;
     }

   }; // class Mapping
};	// namespace XMI
#endif // __common_socklinux_Mapping_h__
