/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/datagram/DatagramSimple.h
/// \brief Simple datagram send protocol for reliable devices.
///
/// The DatagramSimple class defined in this file uses C++ templates and
/// the device "message" interface - which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __xmi_p2p_protocol_send_datagram_datagramsimple_h__
#define __xmi_p2p_protocol_send_datagram_datagramsimple_h__

#include "components/memory/MemoryAllocator.h"
#include "components/devices/generic/ProgressFunctionMsg.h"
#include "p2p/protocols/send/datagram/DatagramConnection.h"
#include "util/queue/Queue.h"

#define WINDOW_SIZE 8                

#define STD_RATE_RECV 1000000
#define STD_RATE_SEND 1000000
#ifndef TRACE_ERR
#define TRACE_ERR(x) fprintf x
#endif

namespace XMI
{
  namespace Protocol
  {
    namespace Send
    {

      ///
      /// \brief Datagram simple send protocol class for reliable network devices.
      ///
      /// \tparam T_Model   Template packet model class
      /// \tparam T_Device  Template packet device class
      /// \tparam T_Message Template packet message class
      ///
      /// \see XMI::Device::Interface::PacketModel
      /// \see XMI::Device::Interface::PacketDevice
      ///
      template <class T_Model, class T_Device, bool T_LongHeader>
      class DatagramSimple
      {
        protected:
		
		  typedef uint8_t pkt_t[T_Model::packet_model_state_bytes];
         
		
		//forward declaration
		  class  send_state_t;
		  struct recv_state_t;
		  
		  
		  
		   //rts_ack header (metadata)
		struct __attribute__((__packed__)) header_ack_t
        {
          send_state_t *            va_send;     ///virtual address receiver
		  recv_state_t *            va_recv;   ///virtual address receiver
          size_t                    wsize;    ///window size fo ack 
		  size_t                    wrate;    ///ack rate 
		};
		
		
		//data header (metadata)
		  struct __attribute__((__packed__)) header_metadata_t
        {
          recv_state_t *            va_recv;   ///virtual address receiver
          size_t                    seqno;     ///packet sequence number 
		  size_t                    bsend;     ///bytes send
		};
		  
		  
		 //acks_info   32 bytes
		   struct rts_info_t{
			   send_state_t *                       va_send;   ///virtual address sender
               xmi_task_t                           fromRank;   ///dest_rank
			   xmi_task_t                           destRank;   ///dest_rank
			   size_t                               wsize;      ///window size fo ack 
			   size_t                               wrate;      ///ack rate
               size_t                               bytes;      ///total bytes to send
			   size_t                               mbytes;     ///total bytes application metadata
            }; 
			
		
		
		struct timer_t {
	     unsigned long long count;    ///define interval
	     unsigned long long t0;       ///starting time
	     unsigned long max;           ///time out condition
		 void *                va;    ///virtual address sender or receiver
      	 bool             start;      ///start flag
		 bool             close;      ///close flag
         };
		
        //receiver status 
		  struct recv_state_t
        {  
          size_t              pkgnum;                            /// Number of pkg to receive from the origin rank.		
		  header_ack_t        ack;                               /// ack info
		  pkt_t               pkt;                               /// Message Object	  
          DatagramSimple<T_Model, T_Device, T_LongHeader> * datagram;  /// Pointer to protocol
          xmi_recv_t           info;                 			/// Application receive information.
  		  size_t               mbytes;             			    /// Message info bytes	
		  xmi_task_t           fromRank;         			    ///origen rank
          //char *             msgbuff;          			    /// RTS data buffer
		  size_t               rpkg;		        		    /// received packages
		  
		  size_t               fbytes;            /// short package bytes
		  size_t               rate;              /// rate timers
		  size_t               bytes;             /// bytes to receive
		   
		  char                                            pmsgbuf[2][XMI::Device::ProgressFunctionMdl::sizeof_msg];   ///timers' buffer
		  timer_t                                         timer1;   ///timer 1
		  timer_t                                         timer0;   ///timer 2
		  size_t              wmaxseq;           ///Max sequence id in control mechanism (CM)             
		  size_t              wminseq;           ///Min sequence id in CM
		  size_t *            lost_list;         ///Lost list array 
		  size_t *            rcv_list;          ///Received  list array 
		  size_t              nlost;             ///number of lost packages
		  size_t               last_nlost;       /// last number of lost elements
		  bool                fseq;              ///Flag used to record minimun wminseq
		 };		
			
			
		  //package structure	
		 struct package_t{
		     header_metadata_t             header;            ///header
		     pkt_t                         pkt;               ///packet to send	
		 };
		           
		 //window structure
		 struct window_t{
		     send_state_t *               va_send;             ///virtual address sender         
			 package_t                    pkg[WINDOW_SIZE];    ///Array of packages	
		 };	
				       
		//send_state_class
		
		class send_state_t: public QueueElem  
		 {
          public:
		  
		  size_t               pkgnum;           /// Number of pkg to send from the origin rank.
          recv_state_t *       va_recv;          /// Receiver Virtual Address
		  char *               send_buffer;      /// Send Buffer address
		  pkt_t                pkt;              /// packet
		  rts_info_t           rts;              /// RTS struct info
		  
		  header_metadata_t    header;           /// aux header
		 
		  xmi_event_function   cb_data;         /// Callback to execute when data have been sent
		  xmi_event_function   cb_rts;          /// Callback to execute when rts have been sent
		  void   *             pf;              /// Pointer to receiver parameters
		  void                 * msginfo;       /// Message info
          xmi_event_function   local_fn;        /// Local Completion callback.
          xmi_event_function   remote_fn;       /// Remote Completion callback.
          void                 * cookie;        /// Cookie
		  
          size_t               rate;             /// used to setup timers rate	
          size_t               pkgsend;          /// number of packages send	
          size_t               last_seqno;       /// last sequence number saved			  
          size_t *             lost_list;        /// lost list array
		  size_t               nlost;            /// number of lost elements
		  size_t               fbytes;           /// short_data bytes, first package 
		  
		  char                                          pmsgbuf[2][XMI::Device::ProgressFunctionMdl::sizeof_msg];   /// timers' buffer
		  timer_t                                       timer1;  ///timer 1
		  timer_t                                       timer0;  ///timer 2
          DatagramSimple<T_Model, T_Device, T_LongHeader> * datagram;  ///pointer to protocol
		  
		   window_t        window[2];               ///define 2 windows
			
        };
		
		
         

        public:

          ///
          /// \brief Datagram simple send protocol constructor.
          ///
          /// \param[in]  dispatch     Dispatch identifier
          /// \param[in]  dispatch_fn  Dispatch callback function
          /// \param[in]  cookie       Opaque application dispatch data
          /// \param[in]  device       Device that implements the message interface
          /// \param[in]  origin_task  Origin task identifier
          /// \param[in]  context      Communication context
          /// \param[out] status       Constructor status
          ///
          inline DatagramSimple (size_t                     dispatch,
                              xmi_dispatch_callback_fn   dispatch_fn,
                              void                     * cookie,
                              T_Device                 & device,
                              xmi_task_t                 origin_task,
                              xmi_client_t              client,
                              size_t                     contextid,
                              xmi_result_t             & status) :
              _rts_model (device, client, contextid),
			  _rts_ack_model(device, client, contextid),
			  _ack_model (device, client, contextid),
			  _data_model (device, client, contextid),
			  _short_data_model (device, client, contextid),
              _device (device),
              _fromRank (origin_task),
              _client (client),
			  _contextid (contextid),
              _dispatch_fn (dispatch_fn),
              _cookie (cookie),
              _connection ((void **)NULL),
              _connection_manager (device),
              _cont(0)
			
			 {
		    // ----------------------------------------------------------------
            // Compile-time assertions
            // ----------------------------------------------------------------

            // This protocol only works with reliable networks.
            //COMPILE_TIME_ASSERT(T_Model::reliable_packet_model == true);

            // This protcol only works with deterministic models.
            //COMPILE_TIME_ASSERT(T_Model::deterministic_packet_model == true);

            // Assert that the size of the packet metadata area is large
            // enough to transfer a single xmi_task_t. This is used in the
            // various postMessage() calls to transfer long header and data
            // messages.
            //COMPILE_TIME_ASSERT(sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes);

            // Assert that the size of the packet payload area is large
            // enough to transfer a single virtual address. This is used in
            // the postPacket() calls to transfer the ack information.
            COMPILE_TIME_ASSERT(sizeof(void *) <= T_Model::packet_model_payload_bytes);

            // ----------------------------------------------------------------
            // Compile-time assertions
            // ----------------------------------------------------------------

		  		  
            _connection = _connection_manager.getConnectionArray (client, contextid);
			
			
			 //no tested yet 
		          //allocate memory
			        _queue = (Queue *) malloc(sizeof(Queue)* _device.peers());
				    
										
					//Initializing queue
				    for(size_t i=0;i< _device.peers();i++)
				        new (&_queue[i]) Queue();
								
	       //not used
			        _lostqueue = (Queue *) malloc(sizeof(Queue)* _device.peers());
				    
										
					//Initializing queue
				    for(size_t i=0;i< _device.peers();i++)
				        new (&_lostqueue[i]) Queue();
						
			//not used
			        _recvqueue = (Queue *) malloc(sizeof(Queue)* _device.peers());
				    
										
					//Initializing queue
				    for(size_t i=0;i< _device.peers();i++)
				        new (&_recvqueue[i]) Queue();			
	
           
			
			status = _rts_model.init (dispatch,
                                           dispatch_rts_direct, this,
                                            dispatch_rts_read, this);
										   
					
	        		
						
            if(status == XMI_SUCCESS)
              {
			  
			     status = _rts_ack_model.init (dispatch,
                                              dispatch_rts_ack_direct, this,
                                              dispatch_rts_ack_read, this);
				 TRACE_ERR((stderr, "DatagramSimple() [2] status = %d\n", status));							  
                							   
										   
           
           if (status == XMI_SUCCESS)
              {
			  
			     status = _ack_model.init (dispatch,
                                              dispatch_ack_direct, this,
                                              dispatch_ack_read, this);
				 TRACE_ERR((stderr, "DatagramSimple() [3] status = %d\n", status));							  
                
              
						 
						 if (status == XMI_SUCCESS)
                           {
                                status = _data_model.init (dispatch,
                                           dispatch_data_direct, this,
                                           dispatch_data_read, this);
                                TRACE_ERR((stderr, "DatagramSimple() [6] status = %d\n", status));
                           
						   
						   
						  if (status == XMI_SUCCESS)
                           {
                                status = _short_data_model.init (dispatch,
                                           dispatch_short_data_direct, this,
                                           dispatch_short_data_read, this);
                                TRACE_ERR((stderr, "DatagramSimple() [7] status = %d\n", status));
						   
						   
						   
						   }
						   
						   
						   
						   
						   
						   }					  
                 
                    }						  
                 
                  }
             
			
          }
		  
		  
		 
		  
   
          ///
          /// \brief Start a new simple send datagram operation.
          ///
          /// \see XMI::Protocol::Send:simple
          ///
		  
		  inline xmi_result_t simple_impl (xmi_send_t * parameters)
          {
          
                      
			TRACE_ERR((stderr,"*** Datagram_DatagramSimple implemented , msginfo_bytes = %d  , bytes =%d ***\n",parameters->send.header.iov_len, parameters->send.data.iov_len));
			//TRACE_ERR((stderr," T_Model::packet_model_metadata_bytes = %d\n T_Model::packet_model_payload_bytes = %d \n sizeof(rts_info_t) = %d\n sizeof(cts_info_t) = %d\n sizeof(send_state_t) = %d\n sizeof(recv_state_t) = %d\n sizeof(header_metadata_t) = %d\n sizeof(header_rts_ack_t) = %d\n",T_Model::packet_model_metadata_bytes,T_Model::packet_model_payload_bytes , sizeof(_info_t),sizeof(cts_info_t), sizeof(send_state_t),sizeof(recv_state_t),sizeof(header_metadata_t),sizeof(header_rts_ack_t)));  					
            TRACE_ERR((stderr," T_Model::packet_model_metadata_bytes = %d\n T_Model::packet_model_payload_bytes = %d \n sizeof(header_metadata_t) = %d \n , sizeof(header_ack_t)= %d \n, sizeof(send_state_t) = %d \n, sizeof(recv_state_t) = %d \n ",T_Model::packet_model_metadata_bytes,T_Model::packet_model_payload_bytes , sizeof(header_metadata_t) , sizeof(header_ack_t) , sizeof(send_state_t), sizeof(recv_state_t)));  					
     
			
            // Allocate memory to maintain the state of the send.
            send_state_t * send = allocateSendState ();
			
			
					  
			
			//Save data in send_state_t
              send->cookie   = parameters->events.cookie;                         ///save cookie
              send->local_fn = parameters->events.local_fn;                       ///save Callback function to execute after local done
			  send->remote_fn = parameters->events.remote_fn;                     ///save Callback function to execute after remote done
			  send->datagram    = this;                           					  ///Save pointer to protocol
				 
			  send->send_buffer=(char *) parameters->send.data.iov_base;         ///sender buffer address		  
			  			  			  
			  send->rts.fromRank=_fromRank;                   				      ///Origen Rank
			  send->rts.bytes= parameters->send.data.iov_len;                    ///Total bytes to send
			  send->rts.va_send= send;                                           ///Virtual Address sender
			  send->rts.destRank=parameters->send.task;                          ///Target Rank
			  send->rts.mbytes = parameters->send.header.iov_len;                ///metadata Number of  bytes
			  send->rts.wrate = STD_RATE_SEND;                                   ///Initial value for window rate
			  send->rts.wsize = 2;                                               /// window size			  
			  send->msginfo=parameters->send.header.iov_base;                    ///Message info
			  send->pkgsend = 0;	  					                         ///Initialize pkgsend
			  send->cb_data  =  cb_data_send;                                    ///register Callback function to execute after data package have been sent
			 // send->cb_rts  =  cb_rts_send;                                    ///register Callback function to execute after rts package have been sent
			  
			  
			send->header.seqno = 0;                           ///Initialize sequence number
            send->header.bsend = 0;                           ///Initialize bytes to send
			send->window[0].va_send = send;                   ///Save sender virtual address on window[0]
			send->window[1].va_send = send;                   ///Save sender virtual address on window[1] 
              		

              //Send if queue is empty
			 if (_queue[_fromRank].isEmpty()){ 
					
			  TRACE_ERR((stderr,">>   Datagram_DatagramSimple implemented .. Info sender  cookie=%p , send =%p , Sender rank  = %d, msinfo= %p , msgbytes = %d , bytes = %d \n",parameters->events.cookie, send, parameters->send.task,   parameters->send.header.iov_base,  parameters->send.header.iov_len, parameters->send.data.iov_len));
			
			
			
		
			    					 if (sizeof(rts_info_t) <= T_Model::packet_model_metadata_bytes){		
										      TRACE_ERR((stderr, "DatagramSimple::simple_impl() ..  protocol header_info_t fits in the packet metadata, application metadata fits in a single packet payload\n"));
                                              struct iovec v[1];
	                                                v[0].iov_base = (void *)send->msginfo;
													v[0].iov_len  = send->rts.mbytes;
											  //OK CASE
						                     _rts_model.postPacket (send->pkt,                          ///T_Message to send
														NULL,                                           ///Callback to execute when done 
														(void *) parameters->events.cookie,             /// Cookie
														 parameters->send.task,                         ///Target Rank
														(void *)&send->rts,          					 ///rts_info_t struct
														sizeof(rts_info_t),                              ///sizeof rts_info_t
														v);     										 ///Message info
									}else{

                                             		TRACE_ERR((stderr, "DatagramSimple::simple_impl() ..  protocol header_info_t doesn't fit in the packet metadata, application metadata fits in a single packet payload\n"));
    
											  
											  
											   struct iovec v[2];
													v[0].iov_base = (void *)&send->rts;
													v[0].iov_len  = sizeof(rts_info_t);
													v[1].iov_base = (void *)send->msginfo;
													v[1].iov_len  = send->rts.mbytes;
											  
											  //OK CASE
						                     _rts_model.postPacket (send->pkt,                          ///T_Message to send
														NULL,                                           ///Callback to execute when done 
														(void *) parameters->events.cookie,             /// Cookie
														 parameters->send.task,                        ///Target Rank
														(void *)NULL,         
														0,                     
														v) ;     ///rts_info and mesage info							
					
				                     }
				 
				 
				 //Start timer (initial value = 0, max number of cycles = 5, rate = 10000 , sender, function send_rts, timer0)
				 StartTxTimer(0, 5 ,STD_RATE_SEND, send , send_rts ,0) ; 
				 
				 
		    } else{
			TRACE_ERR((stderr,">>   Datagram_DatagramSimple implemented ..  queue no empty  cookie=%p , send =%p , Sender rank  = %d, msinfo= %p , msgbytes = %d , bytes = %d \n",parameters->events.cookie, send, parameters->send.task,   parameters->send.header.iov_base,  parameters->send.header.iov_len, parameters->send.data.iov_len));
			
                //save request for later			   
			   _queue[_fromRank].pushTail(send);  
			}

                 			
			
            return XMI_SUCCESS;
          };

        protected:
          inline send_state_t * allocateSendState ()
          {
            return (send_state_t *) _send_allocator.allocateObject ();
          }

          inline void freeSendState (send_state_t * object)
          {
            _send_allocator.returnObject ((void *) object);
          }

          inline recv_state_t * allocateRecvState ()
          {
            return (recv_state_t *) _recv_allocator.allocateObject ();
          }

          inline void freeRecvState (recv_state_t * object)
          {
            _recv_allocator.returnObject ((void *) object);
          }

          inline void setConnection (xmi_task_t task, void * arg)
          {
            size_t peer = _device.task2peer (task);
            TRACE_ERR((stderr, ">> DatagramSimple::setConnection(%zd, %p) .. _connection[%zd] = %p\n", task, arg, peer, _connection[peer]));
            XMI_assert(_connection[peer] == NULL);
            _connection[peer] = arg;
            TRACE_ERR((stderr, "<< DatagramSimple::setConnection(%zd, %p)\n", task, arg));
          }

          inline void * getConnection (xmi_task_t task)
          {
            size_t peer = _device.task2peer (task);
            TRACE_ERR((stderr, ">> DatagramSimple::getConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
            XMI_assert(_connection[peer] != NULL);
            TRACE_ERR((stderr, "<< DatagramSimple::getConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
            return _connection[peer];
          }

          inline void clearConnection (xmi_task_t task)
          {
            size_t peer = _device.task2peer (task);
            TRACE_ERR((stderr, ">> DatagramSimple::clearConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
            _connection[peer] = NULL;
            TRACE_ERR((stderr, "<< DatagramSimple::clearConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
          }


          MemoryAllocator < sizeof(send_state_t), 16 > _send_allocator;
          MemoryAllocator < sizeof(recv_state_t), 16 > _recv_allocator;

          T_Model         _rts_model;
		  T_Model         _rts_ack_model;
		  T_Model         _ack_model;
		  T_Model         _data_model;
		  T_Model         _short_data_model;
    	  T_Device      & _device;
          xmi_task_t      _fromRank;
          xmi_client_t    _client;
		  size_t      _contextid;
          xmi_dispatch_callback_fn   _dispatch_fn;
          void                     * _cookie;
          void ** _connection;
          // Support up to 100 unique contexts.
          //static datagram_connection_t _datagram_connection[];
          DatagramConnection<T_Device> _connection_manager;
          
		  static Queue *    _queue;      ///queue send requests
		  static Queue *    _recvqueue;  ///not used
		  static Queue *    _lostqueue;  ///not used
		  
		  XMI::Device::ProgressFunctionMdl    _progfmodel;   ///used to setup timers
		   
		  send_state_t  * _lastva_send;   //used in CM
		
		  size_t _cont;
		  
		
		
		
		//send lost list sender
		static int send_lost_list(send_state_t * send){
		 					  
			struct iovec v[1];
			 size_t i = 0;
		                      TRACE_ERR((stderr, "	DatagramSimple::send_lost_list() .. nlost =%d \n",send->nlost));
																		
																for(i = 0; i< send->nlost-1;i++){
													   
																		 
																		 TRACE_ERR((stderr, "	DatagramSimple::send_lost_list() .. data lost =%d was sent\n",send->lost_list[i]));
																		

																		send->header.seqno=send->lost_list[i];																		
																		if(send->lost_list[i]==0){
																		   v[0].iov_base = (void *)(send->send_buffer );
																		   v[0].iov_len  = (size_t)send->fbytes;
																		   send->header.bsend = send->fbytes;
																		}else{
																		   v[0].iov_base = (void *)(send->send_buffer + ((send->lost_list[i]-1)*T_Model::packet_model_payload_bytes + send->fbytes));
																		   v[0].iov_len  = (size_t)send->header.bsend;
																		   send->header.bsend = T_Model::packet_model_payload_bytes; 
																		}						
																			
																			send->datagram->_data_model.postPacket (send->pkt,
																							NULL,
																							(void *)send,
																							send->rts.destRank,  
																							(void *) &send->header,
																							sizeof (header_metadata_t),
																							v);
																	  
																	  }
													  
																		TRACE_ERR((stderr, "	DatagramSimple::send_lost_list() .. data lost package =%d was sent\n",send->lost_list[i]));
																	 send->header.seqno=send->lost_list[i];
																	  if(send->lost_list[i]==0){
																		   v[0].iov_base = (void *)(send->send_buffer );
																		   v[0].iov_len  = (size_t)send->fbytes;
																		   send->header.bsend = send->fbytes;
																		}else{
																		   v[0].iov_base = (void *)(send->send_buffer + ((send->lost_list[i]-1)*T_Model::packet_model_payload_bytes + send->fbytes));
																		   v[0].iov_len  = (size_t)send->header.bsend;
																		   send->header.bsend = T_Model::packet_model_payload_bytes; 
																		}						
																			
																			send->datagram->_data_model.postPacket (send->pkt,
																							send->cb_data,
																							(void *)send,
																							send->rts.destRank,  
																							(void *) &send->header,
																							sizeof (header_metadata_t),
																							v);
													 
													                     send->nlost=0;
																		 send->header.seqno=send->last_seqno;			
																		 
			
                 return 0;			
			}															 
		
		//send lost list receiver
		int send_lost_list_rx(recv_state_t * send){
		     
							  
			struct iovec v[1];
			 size_t i = 0;
		                      TRACE_ERR((stderr, "	DatagramSimple::send_lost_list() .. nlost =%d \n",send->nlost));
																		
																for(i = 0; i< send->nlost-1;i++){
													   
																		 
																		 TRACE_ERR((stderr, "	DatagramSimple::send_lost_list() .. data lost =%d was sent\n",send->lost_list[i]));
																		

																		send->header.seqno=send->lost_list[i];																		
																		if(send->lost_list[i]==0){
																		   v[0].iov_base = (void *)(send->send_buffer );
																		   v[0].iov_len  = (size_t)send->fbytes;
																		   send->header.bsend = send->fbytes;
																		}else{
																		   v[0].iov_base = (void *)(send->send_buffer + ((send->lost_list[i]-1)*T_Model::packet_model_payload_bytes + send->fbytes));
																		   v[0].iov_len  = (size_t)send->header.bsend;
																		   send->header.bsend = T_Model::packet_model_payload_bytes; 
																		}						
																			
																			send->datagram->_data_model.postPacket (send->pkt,
																							NULL,
																							(void *)send,
																							send->rts.destRank,  
																							(void *) &send->header,
																							sizeof (header_metadata_t),
																							v);
																	  
																	  }
													  
																		TRACE_ERR((stderr, "	DatagramSimple::send_lost_list() .. data lost package =%d was sent\n",send->lost_list[i]));
																	 send->header.seqno=send->lost_list[i];
																	  if(send->lost_list[i]==0){
																		   v[0].iov_base = (void *)(send->send_buffer );
																		   v[0].iov_len  = (size_t)send->fbytes;
																		   send->header.bsend = send->fbytes;
																		}else{
																		   v[0].iov_base = (void *)(send->send_buffer + ((send->lost_list[i]-1)*T_Model::packet_model_payload_bytes + send->fbytes));
																		   v[0].iov_len  = (size_t)send->header.bsend;
																		   send->header.bsend = T_Model::packet_model_payload_bytes; 
																		}						
																			
																			send->datagram->_data_model.postPacket (send->pkt,
																							send->cb_data,
																							(void *)send,
																							send->rts.destRank,  
																							(void *) &send->header,
																							sizeof (header_metadata_t),
																							v);
													 
													                     send->nlost=0;
																		 send->header.seqno=send->last_seqno;			
																		 
			
                 return 0;			
			}															 
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		//Timer send data function 
	    static xmi_result_t send_data(xmi_context_t ctx, void *cd) {
	                     timer_t * stimer = (timer_t *) cd;
						 send_state_t * send = (send_state_t *) stimer->va;
						 size_t i=0;
						 
						 if(stimer->close){
					             TRACE_ERR((stderr,"   DatagramSimple::Timer send_data() .. Tx Close Timer\n")); 			    
                                 return XMI_SUCCESS; 
						}else if(stimer->start){		 
								   if (stimer->t0 == 0) {
													stimer->t0 = (unsigned long long)__global.time.time();
													TRACE_ERR((stderr, "	DatagramSimple::Timer send_data().. Tx Starting work at tick %llu, waiting until %llu\n", stimer->t0, stimer->t0 + stimer->count));
													 }
											
									unsigned long long t1 = (unsigned long long)__global.time.time();
									if (t1 - stimer->t0 >= stimer->count) {
									
									
									
									
												TRACE_ERR((stderr, "	DatagramSimple::Timer send_data().. Tx Finished at tick %llu (%ld calls)\n", t1,stimer->max));
												stimer->t0 = 0;             //reset timer
												--stimer->max;              //decrement package to send
												if (stimer->max == 0){      //End if time out
												
												    									  
																TRACE_ERR((stderr,"   DatagramSimple::Timer send_data() ..  Time Out\n")); 
																	if (send->local_fn) 
																				   send->local_fn (XMI_Client_getcontext(send->datagram->_client, send->datagram->_contextid),send->cookie, XMI_ERROR); 
										  
																	if (send->remote_fn)
																		send->remote_fn (XMI_Client_getcontext(send->datagram->_client, send->datagram->_contextid), send->cookie , XMI_ERROR);
												
																	if (send->remote_fn == NULL)
																		{
																		send->datagram->freeSendState (send);
																		} 
										                 
										
												
												
													              return XMI_ERROR;
												}else{
													   //send package again
								   
																	
													TRACE_ERR((stderr, "	DatagramSimple::Timer send_data() .. Sending data, protocol header_info_t fits in the packet metadata, application metadata fits in a single packet payload\n"));

												 //Post First Data package 

												    struct iovec v[1];
													
															v[0].iov_base = &send->send_buffer;
														    v[0].iov_len  = send->header.bsend;
														
                                                      if(send->header.seqno == 0){	

													  
                                                                     									 
																				  TRACE_ERR((stderr, "	DatagramSimple::Timer send_data() .. data seqno=0  package =%d was sent\n",send->lost_list[i]));
																																						  
																				  send->datagram->_short_data_model.postPacket   ( send->pkt,          ///T_message to send
																											send->cb_data,         ///Callback to execute when done
																											(void *)send,
																											send->rts.destRank,                 ///target Rank
																											&send->header,         ///metadata
																											sizeof (header_metadata_t),    ///metadata size   
																											v);                      ///iovec to send
																											
																															
														}else if(send->nlost>0){
														       
															       TRACE_ERR((stderr, "	DatagramSimple::Timer send_data() .. sending lost list"));
												
															      //send lost list type=true (sender)
															       send_lost_list((send_state_t *)send);
															   
																		 
														              TRACE_ERR((stderr, "	DatagramSimple::Timer send_data() .. data (%d) was sent\n", ++send->datagram->_cont));
													}else
																																			
														TRACE_ERR((stderr, "	DatagramSimple::Timer send_data() .. Nothing send (%d) was sent\n", ++send->datagram->_cont));
                                                    
									        
										}
								}
					}			 
	return XMI_EAGAIN;
        }	
		
		
		//Timer send ack function 
		 static xmi_result_t send_ack(xmi_context_t ctx, void *cd) {
		 
		    
	                     timer_t *rtimer = (timer_t *)cd;
						 recv_state_t * rcv = (recv_state_t *) rtimer->va;
						 
						if(rtimer->close){
					             TRACE_ERR((stderr,"   DatagramSimple::Timer send_ack() .. Rx Close Timer\n")); 			    
                                 return XMI_SUCCESS; 
						}else if(rtimer->start){ 
								   if (rtimer->t0 == 0) {
													rtimer->t0 = (unsigned long long)__global.time.time();
													TRACE_ERR((stderr, "    DatagramSimple::Timer send_ack() .. Rx Starting work at tick %llu, waiting until %llu\n", rtimer->t0, rtimer->t0 + rtimer->count));
													 }
											
									unsigned long long t1 = (unsigned long long)__global.time.time();
									if (t1 - rtimer->t0 >= rtimer->count) {
												TRACE_ERR((stderr, "    DatagramSimple::Timer send_ack() .. Rx Finished at tick %llu (%ld calls)\n", t1,rtimer->max));
												rtimer->t0 = 0;             //reset timer
												--rtimer->max;              //decrement package to send
												if (rtimer->max == 0){      //End if time out
												
												    										  
																TRACE_ERR((stderr,"   DatagramSimple::Timer send_ack() ..  Time Out\n")); 
															    if (rcv->info.local_fn) 
																				   rcv->info.local_fn (XMI_Client_getcontext(rcv->datagram->_client, rcv->datagram->_contextid),rcv->info.cookie, XMI_ERROR); 
										  
																rcv->datagram->freeRecvState (rcv);
																									 
										
												
												
													              return XMI_ERROR;
												}else{
												
												

                                                      TRACE_ERR((stderr, "   DatagramSimple::Timer send_ack() sending ack.  Protocol header_info_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));
																		 
											//Post cts package 
													struct iovec v[1];
															v[0].iov_base = rcv->lost_list;
														    v[0].iov_len  = 4*rcv->nlost;											
							                    rcv->datagram->_ack_model.postPacket (rcv->pkt,
													NULL,
													rcv->info.cookie,
													rcv->fromRank,
													&rcv->ack,
													sizeof (header_ack_t),
													v);	
													
                                            } 													
								}
					
					}					 
	return XMI_EAGAIN;
        }
		
		
		
		 //Timer send rts function  
		   static xmi_result_t send_rts(xmi_context_t ctx, void *cd) {
	                     timer_t * stimer = (timer_t *) cd;
						 send_state_t * send = (send_state_t *) stimer->va;
						 
						 if(stimer->close){
					             TRACE_ERR((stderr,"   DatagramSimple::Timer send_rts() .. Tx Close Timer\n")); 			    
                                 return XMI_SUCCESS; 
						}else if(stimer->start){						 
								   if (stimer->t0 == 0) {
													stimer->t0 = __global.time.timebase();
													TRACE_ERR((stderr, "	DatagramSimple::Timer send_rts().. Tx Starting work at tick %llu, waiting until %llu\n", stimer->t0, stimer->t0 + stimer->count));
													 }
											
									unsigned long long t1 = __global.time.timebase();
									if (t1 - stimer->t0 >= stimer->count) {
												TRACE_ERR((stderr, "	DatagramSimple::Timer send_rts().. Tx Finished at tick %llu (%ld calls)\n", t1,stimer->max));
												stimer->t0 = 0;             //reset timer
												--stimer->max;              //decrement package to send
												if (stimer->max == 0){      //End if time out
												
												    										  
																TRACE_ERR((stderr,"   DatagramSimple::Timer send_rts() ..  Time Out\n")); 
																	   if (send->local_fn) 
																				   send->local_fn (XMI_Client_getcontext(send->datagram->_client, send->datagram->_contextid),send->cookie, XMI_ERROR); 
										  
																	if (send->remote_fn)
																		send->remote_fn (XMI_Client_getcontext(send->datagram->_client, send->datagram->_contextid), send->cookie , XMI_ERROR);
												
																	if (send->remote_fn == NULL)
																		{
																		send->datagram->freeSendState (send);
																		} 
										 
										
												
												
													              return XMI_ERROR;
												}else{
													   //send package again
								   
																	
													TRACE_ERR((stderr, "	DatagramSimple::Timer rts_send() .. Sending RTS, protocol header_info_t fits in the packet metadata, application metadata fits in a single packet payload\n"));

														  //OK CASE
														  struct iovec v[1];
															v[0].iov_base = send->msginfo;
														    v[0].iov_len  = send->rts.mbytes;		
															
															
															
															if (sizeof(rts_info_t) <= T_Model::packet_model_metadata_bytes){		
																						
																						  send->datagram->_rts_model.postPacket (send->pkt,       ///T_Message to send
																										NULL,                    ///Callback to execute when done 
																										(void *) send->cookie,             /// Cookie
																										send->rts.destRank,                        ///Target Rank
																										(void *)&send->rts,          ///rts_info_t struct
																										sizeof(rts_info_t),          ///sizeof rts_info_t
																										v);  		             ///iovec to send
																				}else{

																						  
																						   struct iovec v[2];
																								v[0].iov_base = (void *)&send->rts;
																								v[0].iov_len  = sizeof(rts_info_t);
																								v[1].iov_base = (void *)send->msginfo;
																								v[1].iov_len  = send->rts.mbytes;
																						  
																						  //OK CASE
																						 send->datagram->_rts_model.postPacket (send->pkt,       ///T_Message to send
																										NULL,                    ///Callback to execute when done 
																										(void *) send->cookie,             /// Cookie
																										send->rts.destRank,                        ///Target Rank
																										(void *)NULL,          ///rts_info_t struct
																										0,          ///sizeof rts_info_t
																										v);  		             ///iovec to send
																
																				 }
															
															
															
															
															
															
															
														
																
																	
														TRACE_ERR((stderr, "	DatagramSimple::Timer rts_send() .. rts (%d) was sent\n", ++send->datagram->_cont));

									            
										}
								}
					}
	return XMI_EAGAIN;
        }
		  
		  //Timer send rts ack function 
		  static xmi_result_t send_rts_ack(xmi_context_t ctx, void *cd) {
	                     timer_t *rtimer = (timer_t *)cd;
						 recv_state_t * rcv = (recv_state_t *) rtimer->va;
						 
						if(rtimer->close){
					             TRACE_ERR((stderr,"   DatagramSimple::Timer send_rts_ack() .. Rx Close Timer\n")); 			    
                                 return XMI_SUCCESS; 
						}else if(rtimer->start){						 
								   if (rtimer->t0 == 0) {
													rtimer->t0 = __global.time.timebase();
													TRACE_ERR((stderr, "    DatagramSimple::Timer send_rts_ack() .. Rx Starting work at tick %llu, waiting until %llu\n", rtimer->t0, rtimer->t0 + rtimer->count));
													 }
											
									unsigned long long t1 = __global.time.timebase();
									if (t1 - rtimer->t0 >= rtimer->count) {
												TRACE_ERR((stderr, "    DatagramSimple::Timer send_rts_ack() .. Rx Finished at tick %llu (%ld calls)\n", t1,rtimer->max));
												rtimer->t0 = 0;             //reset timer
												--rtimer->max;              //decrement package to send
												if (rtimer->max == 0){      //End if time out
												
												    										  
																TRACE_ERR((stderr,"   DatagramSimple::Timer send_rts_ack() ..  Time Out\n")); 
															    if (rcv->info.local_fn) 
																				   rcv->info.local_fn (XMI_Client_getcontext(rcv->datagram->_client, rcv->datagram->_contextid),rcv->info.cookie, XMI_ERROR); 
										  
																rcv->datagram->freeRecvState (rcv);
																									 
										
												
												
													              return XMI_ERROR;
												}else{
												TRACE_ERR((stderr, "    DatagramSimple::Timer send_rts_ack() ..  Sending rts_ack.  Protocol rts_info_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));
																							 
																//Post cts package  
													 struct iovec v[1];
														v[0].iov_base = NULL;
														v[0].iov_len  = 0;	
														
												   rcv->datagram->_rts_ack_model.postPacket (rcv->pkt,
																		NULL,
																		rcv->info.cookie,
																		rcv->fromRank,
																		&rcv->ack,
																		sizeof (header_ack_t),
																		v);
																		
												TRACE_ERR((stderr, "	DatagramSimple::Timer send_rts_ack() .. rts_ack (%d) was sent\n", ++rcv->datagram->_cont));					
											}							
								}
					
					}
					   
					  
                  						 
	return XMI_EAGAIN;
        }
		  
		  
		  
		  //Start Tx Timer  function 
	      static int StartTxTimer( unsigned long long t0,  unsigned long max ,unsigned long long rate, send_state_t * va , xmi_result_t (*func) (xmi_context_t,void *), bool sel) { 
		
		  
		  XMI_ProgressFunc_t                            progf;
		 		
				
				if (sel){
					va->timer1.t0 = t0;                    //initial time
					va->timer1.start = true;               //Start OK
					va->timer1.close = false;              //No Close
			        va->timer1.max=max; 				   //Time out condition
					va->rate = rate;                       //timer rate
					va->timer1.count = va->rate;           //interval
					va->timer1.va=va;                      //Virtual Adress Sender
	        	   progf.request = &va->pmsgbuf[0];
				   progf.clientdata = &va->timer1;  

				}else{
                    va->timer0.t0 = t0;                    //initial time
					va->timer0.start = true;               //Start
					va->timer0.close = false;              //No Close
			        va->timer0.max=max; 				   //Time out condition
					va->rate = rate;                       //timer rate
					va->timer0.count = va->rate;           //interval
					va->timer0.va=va;                      //Virtu
					progf.request = &va->pmsgbuf[1];
					progf.clientdata = &va->timer0;  
				}
				   
				progf.client = va->datagram->_client;
	            progf.context = 0;  
				//_progf.context = _context;
				
				progf.func = func;                      //function to execute
				progf.cb_done = (xmi_callback_t) {NULL};
				
				bool rc = va->datagram->_progfmodel.postWork(&progf);
				if (!rc) {
					TRACE_ERR((stderr, "Failed to generateMessage on progress function\n"));
					return 1;
				}
		  return 0;
		  }
		  
		  //Start Rx Timer  function 
	      static int StartRxTimer( unsigned long long t0, unsigned long max ,unsigned long long rate,  recv_state_t * va, xmi_result_t (*func) (xmi_context_t, void *) , bool sel) { 
		
		  
		  XMI_ProgressFunc_t                            progf;
		 		
				
				if (sel){
					va->timer1.t0 = t0;                    //initial time
					va->timer1.start = true;               //Start
					va->timer1.close = false;              //No Close
			        va->timer1.max=max; 					  //Time out condition
					va->rate = rate;                       //timer rate
					va->timer1.count = va->rate;           //interval
					va->timer1.va=va;                      //Virtual Adress Sender
	        	   progf.request = &va->pmsgbuf[0];
				   progf.clientdata = &va->timer1;  

				}else{
                    va->timer0.t0 = t0;                    //initial time
					va->timer0.start = true;               //Start
					va->timer0.close = false;              //No Close
			        va->timer0.max=max; 					  //Time out condition
					va->rate = rate;                      //timer rate
					va->timer0.count = va->rate;           //interval
					va->timer0.va=va;                      //Virtu
					progf.request = &va->pmsgbuf[1];
					progf.clientdata = &va->timer0;  
				}
				   
				 progf.client = va->datagram->_client;
	             progf.context = 0;     
				//_progf.context = _context;
				
				progf.func = func;                      //function to execute
				progf.cb_done = (xmi_callback_t) {NULL};
				
				bool rc = va->datagram->_progfmodel.postWork(&progf);
				if (!rc) {
					TRACE_ERR((stderr, "Failed to generateMessage on progress function\n"));
					return 1;
				}
		   return 0;
		  }
		    
		  
		  //Reset Rx Timer  function 
		  static int ResetRxTimer( unsigned long long t0, unsigned long max ,unsigned long long rate,  recv_state_t * va ,bool sel ) { 
		
		     if (sel){
		  			va->timer1.t0 = t0;                   //initial time
					va->timer1.close = false;             //No Close
			        va->timer1.max=max;                   //Time out condition
					va->rate = rate;                     //interval
					va->timer1.va=va;                    //Virtual Adress Sender
	        	    va->timer1.count = va->rate;	         //rate
	        		va->timer1.start = true;              //Start	
					
			}else{
			       va->timer0.t0 = t0;                   //initial time
					va->timer0.close = false;             //No Close
			        va->timer0.max=max;                   //Time out condition
					va->rate = rate;                     //interval
					va->timer0.va=va;                    //Virtual Adress Sender
	        	    va->timer0.count = va->rate;	         //rate
	        		va->timer0.start = true;              //Start	
			 
			 
			 }

			return 0;
		  }
		  
		  //Start Tx Timer  function 
		  static int ResetTxTimer( unsigned long long t0, unsigned long max ,unsigned long long rate,  send_state_t * va,  bool sel) { 
		
		       if (sel){
		  			va->timer1.t0 = t0;                   //initial time
					va->timer1.close = false;             //No Close
			        va->timer1.max=max;                   //Time out condition
					va->rate = rate;                     //interval
					va->timer1.va=va;                    //Virtual Adress Sender
	        	    va->timer1.count = va->rate;	         //rate
	        		va->timer1.start = true;              //Start	
					
			   }else{
			       va->timer0.t0 = t0;                   //initial time
					va->timer0.close = false;             //No Close
			        va->timer0.max=max;                   //Time out condition
					va->rate = rate;                     //interval
					va->timer0.va=va;                    //Virtual Adress Sender
	        	    va->timer0.count = va->rate;	         //rate
	        		va->timer0.start = true;              //Start	
			 
			 
			 }


			return 0;
		  }
		  
		  
		  //Stop Rx Timer  function 
		  static int StopRxTimer( recv_state_t * va, bool sel ) { 
					  if(sel){
						 va->timer1.start = false;
						  TRACE_ERR((stderr,"   DatagramSimple::Timer1 () .. Rx Stop Timer\n")); 
					  }else{
						 va->timer0.start = false;
						  TRACE_ERR((stderr,"   DatagramSimple::Timer2 () .. Rx Stop Timer\n")); 
					  }
					  
		  return 0;
		 }
		 
		  //Start Tx Timer  function 
		  static int StopTxTimer( send_state_t * va, bool sel ) { 
					 if(sel){
						 va->timer1.start = false;
						  TRACE_ERR((stderr,"   DatagramSimple::Timer1 () .. Tx Stop Timer\n")); 
					  }else{
						 va->timer0.start = false;
						  TRACE_ERR((stderr,"   DatagramSimple::Timer2 () .. Tx Stop Timer\n")); 
					  }		  
			return 0;
		 }
		 
		  //Close Tx Timer  function 
		  static int CloseTxTimer( send_state_t * va, bool sel ) { 
					  if(sel){
						 va->timer1.close = true;
					  }else{
						 va->timer0.close = true;
					  }
			return 0;
		 }
		 
		  //Close Rx Timer  function 
		  static int CloseRxTimer( recv_state_t * va , bool sel) { 
					 if(sel){
						 va->timer1.close = true;
					  }else{
						 va->timer0.close = true;
					  }
		  return 0;
		 }

 
		 
		  
		  ///
          /// \brief Direct send rts packet dispatch.
          ///
		  
		   static int dispatch_rts_direct (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
		   
			TRACE_ERR((stderr, ">> DatagramSimple::process_rts()\n"));
		   
			rts_info_t * rts;
			void * msginfo ;
			
					if (sizeof(rts_info_t) <= T_Model::packet_model_metadata_bytes)
					  {
					      TRACE_ERR((stderr, "   DatagramSimple::process_rts() .. rts_info_t coming inside metadata\n"));
		   
						  rts= (rts_info_t *) metadata;          
						  msginfo= (void *) (payload); 			
					}else{
					
					     TRACE_ERR((stderr, "   DatagramSimple::process_rts() .. rts_info_t coming inside payload\n"));
		   
						  rts= (rts_info_t *) payload;          
						   msginfo= (void *) (rts+1); 	
					 }
			
		  
			DatagramSimple<T_Model, T_Device, T_LongHeader> * datagram =
              (DatagramSimple<T_Model, T_Device, T_LongHeader> *) recv_func_parm;
			  
			
			
			TRACE_ERR((stderr, ">> DatagramSimple::process_rts() rts(%d) received , rts->va_send = %p  datagram->_lastva_send=%p \n", ++datagram->_cont,rts->va_send, datagram->_lastva_send));
			
			 
			
            if(datagram->_lastva_send == rts->va_send ){
						
			       return 0;
			} else{	
			       datagram->_cont=0;                      ///counter for reference
				  
                   datagram->_lastva_send = rts->va_send;  ///update  va_send
			
								
													
										  // Allocate memory to maintain the state of the rcv.			
										recv_state_t * rcv = datagram->allocateRecvState ();
																					
										//save pointer to protocol 
										rcv->datagram = datagram;            
																				   
										   //Save data in recv_state_t			  
										  rcv->ack.va_send  = rts->va_send;             ///Virtual address sender
										  rcv->ack.va_recv = rcv;                        ///Virual Address Receiver
										  rcv->ack.wsize = rts->wsize;                   ///Save window size
										  rcv->ack.wrate = rts->wrate;                   ///Save window rate
										  
										  rcv->bytes=rts->bytes;                       ///Total Bytes to send
										  rcv->fromRank  = rts->fromRank;              ///Origin Rank
										  rcv->mbytes  = rts->mbytes;                   ///Metadata application bytes 
										  rcv->rpkg  = 0;                               ///Metadata application bytes 
										  
										  rcv->nlost =0;                                ///Number of lost packages
										  rcv->last_nlost =0;                           ///Last number of lost packages
                                          rcv->fseq =true;					            ///Flag to true						  
										  rcv->wmaxseq =0;	                            ///Min seqno								  
										  rcv->wminseq =0;	                            ///Max seqno
										  
										  ///need to implement longheader in the future
										  //rcv->msgbuff = (char *) malloc (sizeof (char*)*rts->mbytes);  ///Allocate buffer for Metadata
										  //rcv->msgbytes = 0;                                            ///Initalized received bytes 
												
                          
						                 ///initliaze Array with zeros   
            							   rcv->lost_list = (size_t *) malloc(sizeof(size_t)* rts->bytes);
                                           rcv->rcv_list = (size_t *) malloc(sizeof(size_t)* rts->bytes);
											
											 for(size_t i=0; i< rts->bytes ;i++)
				                                     rcv->lost_list[i]= 0;
											 
                                             for(size_t i=0; i< rts->bytes ;i++)
				                                     rcv->rcv_list[i]= 0;											 
						                 /// end initialize array					
										 
                                          
										  //Determine short_data package bytes, first one to receive
										   if((rts->bytes % (T_Model::packet_model_payload_bytes) == 0)) {            
											   rcv->fbytes = T_Model::packet_model_payload_bytes;
											   rcv->pkgnum = rts->bytes/T_Model::packet_model_payload_bytes;
										  }else{
												rcv->fbytes = (rts->bytes % (T_Model::packet_model_payload_bytes));
												rcv->pkgnum = rts->bytes/T_Model::packet_model_payload_bytes+1;
			  									}				
													
											TRACE_ERR((stderr,"   DatagramSimple::process_rts() ..  _context=%p , -contextid =%d , cookie=%p ,Sender = %d, msinfo= %p , msgbytes = %d , bytes = %d, Recv_inof addr = %p \n",datagram->_client,datagram->_contextid,datagram->_cookie, rts->fromRank,   msginfo,  rts->mbytes, rts->bytes ,(void *)(rcv->info.data.simple.addr)));
															  
											
											// Invoke the registered dispatch function.
											datagram->_dispatch_fn.p2p (datagram->_client,  // Communication context
													 datagram->_contextid,                //context id
													 datagram->_cookie,                   // Dispatch cookie
													 rts->fromRank,                   // Origin (sender) rank
													 msginfo,                          // Application metadata
													 rts->mbytes,                      // Metadata bytes
													 NULL,                             // No payload data
													 rts->bytes,                      // Number of msg bytes
													 (xmi_recv_t *) &(rcv->info));     //Recv_state struct
							  
										    
											struct iovec v[1];
																v[0].iov_base = NULL;
																v[0].iov_len  = 0;

											// Only handle simple receives .. until the non-contiguous support
											// is available
											XMI_assert(rcv->info.kind == XMI_AM_KIND_SIMPLE);

											if (rts->bytes == 0) // Move this special case to another dispatch funtion to improve latency in the common case.
													{
													
													TRACE_ERR((stderr, "   DatagramSimple::process_rts() .. Application Metadata send in only one package, 0 bytes received\n"));
										
													// No data packets will follow this envelope packet. Invoke the
													// recv done callback and, if an acknowledgement packet was
													// requested send the acknowledgement. Otherwise return the recv
													// state memory which was allocated above.
												 
													
													
													
									               TRACE_ERR((stderr, "   DatagramSimple::process_rts() ..  Sending rts_ack.  Protocol rts_info_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));
																							 
																//Post cts package  
													
																
												   datagram->_rts_ack_model.postPacket (rcv->pkt,
																		receive_complete,
																		(void *)rcv,
																		rts->fromRank,
																		&rcv->ack,
																		sizeof (header_ack_t),
																		v);
													
													
													rcv->datagram->_lastva_send=NULL;
													
																										
												  return 0;
												  }
												  
														
																		  
																	  
																			
																						

									           TRACE_ERR((stderr, "   DatagramSimple::process_rts() ..  Sending rts_ack.  Protocol rts_info_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));
																							 
																//Post rts_ack package  
																
												   datagram->_rts_ack_model.postPacket (rcv->pkt,
																		NULL,
																		rcv->info.cookie,
																		rts->fromRank,
																		&rcv->ack,
																		sizeof (header_ack_t),
																		v);
																							
																  
									            //Start timer (initial value = 0, max number of cycles = 5, rate = 10000 , receiver, function send_rts_ack, timer0)
								                StartRxTimer(0, 5 ,STD_RATE_RECV, rcv ,send_rts_ack, 0) ; 
								
											
												TRACE_ERR((stderr, ">> DatagramSimple::process_rts() rts_ack was sent\n"));
									
												
				return 0;
			  }
          };
		  
		  
		  
		 ///
          /// \brief Direct send ack   packet dispatch.
          ///
		  
		  static int dispatch_rts_ack_direct (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
		  
		   TRACE_ERR((stderr, ">> DatagramSimple::process_ack()\n"));
		   
		   //Pointer to Protocol object
            DatagramSimple<T_Model, T_Device, T_LongHeader> * datagram=
              (DatagramSimple<T_Model, T_Device, T_LongHeader> *) recv_func_parm;
			  
			
		  
		   TRACE_ERR((stderr, "   DatagramSimple::process_rts_ack() .. header_acks_t coming inside metadata\n")); 			 
           		   
		   header_ack_t * ack= (header_ack_t *) metadata;
		 
		   
			StopTxTimer(ack->va_send, 0);     //stop timer0; 	
            
			
			  if (ack->va_send->rts.bytes ==0){
			  
			  
			                        TRACE_ERR((stderr,"   DatagramSimple::rts_ack() ..  0 bytes condition \n")); 
				                           
										CloseTxTimer(ack->va_send,0);   //close timer0; 	
                                        //call terminate function										
										send_complete (XMI_Client_getcontext(datagram->_client, datagram->_contextid),(void *) ack->va_send, XMI_SUCCESS);
                                      
										   
												 
			 return 0;
			 }
			 
			 			  
			
					
								
			TRACE_ERR((stderr,"   DatagramSimple::process_rts_ack() .. va_recv = %p , to_send = %d , payload_bytes = %d, pkgnum= %d , total of bytes=%d\n",ack->va_recv, ack->va_send->header.bsend, T_Model::packet_model_payload_bytes, ack->va_send->pkgnum,ack->va_send->rts.bytes));
				
								
				ack->va_send->header.va_recv  = ack->va_recv;        ///Virtual address reciver
				ack->va_send->header.seqno  = 0;                     ///Initialize seqno to zero
				ack->va_send->pf = recv_func_parm;                   ///Save pointer to recv_func_param
				
				//Determine short_data package and total of packages to send
				   if((ack->va_send->rts.bytes % (T_Model::packet_model_payload_bytes) == 0)) {            
				       ack->va_send->fbytes = T_Model::packet_model_payload_bytes;
					   ack->va_send->pkgnum = ack->va_send->rts.bytes/T_Model::packet_model_payload_bytes;
				   }else{
					    ack->va_send->fbytes = (ack->va_send->rts.bytes % (T_Model::packet_model_payload_bytes));
						ack->va_send->pkgnum = ack->va_send->rts.bytes/T_Model::packet_model_payload_bytes+1;
			        }
			
				ack->va_send->header.bsend=ack->va_send->fbytes;  //update header
			
				++ack->va_send->pkgsend;                          //update send packages number
			
				TRACE_ERR((stderr, "   DatagramSimple::process_rts_ack() .. protocol header_metadata_t fits in the packet metadata\n"));
			
				TRACE_ERR((stderr, "   DatagramSimple::process_rts_ack() ..  Sending data.  Protocol header_metadata_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));
            
												struct iovec v[1];
																v[0].iov_base = &ack->va_send->send_buffer;
																v[0].iov_len  = ack->va_send->header.bsend;
			
							 //Post First Data package   			 
							datagram->_short_data_model.postPacket   ( ack->va_send->pkt,          ///T_message to send
												ack->va_send->cb_data,         ///Callback to execute when done
												(void *)ack->va_send,
												ack->va_send->rts.destRank,                 ///target Rank
											    &ack->va_send->header,         ///metadata
												sizeof (header_metadata_t),    ///metadata size   
												v);                      ///iovec to send
						 
			
				 
								
				TRACE_ERR((stderr, ">> DatagramSimple::process_ack_rts() Data was Sent\n"));				
	
	            //Remove Head from queue
		        datagram->_queue[datagram->_fromRank].popHead();
				
		        //Check queue is not empty
		       	if(!datagram->_queue[datagram->_fromRank].isEmpty()){	
                    
			
				 //recover next object to send			
			     send_state_t * send = (send_state_t *) (datagram->_queue[datagram->_fromRank].peekHead()); 
					
										TRACE_ERR((stderr, "DatagramSimple::process_ack_rts() .. Sending queue request,  protocol header_info_t fits in the packet metadata, application metadata fits in a single packet payload\n"));
												
												struct iovec v[1];
																v[0].iov_base = send->msginfo;
																v[0].iov_len  = send->rts.mbytes;
											  //OK CASE
						                     datagram->_rts_model.postPacket (send->pkt,       ///T_Message to send
														NULL,                    ///Callback to execute when done 
														(void *) send->cookie,             /// Cookie
														send->rts.destRank,                        ///Target Rank
														(void *)&send->rts,          ///rts_info_t struct
														sizeof(rts_info_t),          ///sizeof rts_info_t
														v);  		             ///Number of bytes
					
				 
				//reset timer 
				ResetTxTimer(0, 5 ,STD_RATE_SEND, send ,0) ; 	     			
	            return 0;
	          }
			  
			 CloseTxTimer(ack->va_send, 0);     //close timer0; 
             
             //Start timer (initial value = 0, max number of cycles = 10, rate = 1200 , sender, function send_data, timer1)	                			 
	         StartTxTimer(0, 10 ,1200, ack->va_send , send_data, 1) ; 
	        
				
			
			  return 0;
       };			  
		  
		  
		  
		   ///
          /// \brief Direct send short_data   packet dispatch.
          ///
          static int dispatch_short_data_direct   (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {				
			TRACE_ERR((stderr, ">> DatagramSimple::process_short_data()\n"));
            DatagramSimple<T_Model, T_Device, T_LongHeader> * datagram=
              (DatagramSimple<T_Model, T_Device, T_LongHeader> *) recv_func_parm;
			 
       		   
		      //Pointer to metadata
		      header_metadata_t * header = (header_metadata_t *) metadata;
		   
		      CloseRxTimer(header->va_recv, 0);     //close timer; 	
			  
		      
			   header->va_recv->rcv_list[header->seqno] = header->seqno;  //saved seqno in received list
			
				TRACE_ERR((stderr, "   DatagramSimple::process_short_data() .. header_metadata_t coming inside metadata\n"));
				    
				TRACE_ERR((stderr,"   DatagramSimple::process_short_data() .. buffer address =%p ,  bytes for receiving =%d, offset = %d, bsend = %d\n", header->va_recv->info.data.simple.addr, header->va_recv->info.data.simple.bytes , header->seqno, header->va_recv->fbytes));
			 
				  				  
				 memcpy ((char *)(header->va_recv->info.data.simple.addr) + header->seqno, (payload), header->va_recv->fbytes); 
				 																		
   
                //Update total of bytes received
                header->va_recv->info.data.simple.bytes -= header->bsend;
			 
			    //update total of received packages 
			    ++header->va_recv->rpkg;
			   
			    
				//Start timer (initial value = 0, max number of cycles = 20, rate = 1200 , receiver, function send_ack, timer1)		                
			    //StartRxTimer(0, 20 ,1200, header->va_recv , send_ack, 1) ; 
				
				header->va_recv->lost_list[0] =0;  //set lost list to zero 
		      				 
				 //are all bytes received?
				 if ((header->va_recv->info.data.simple.bytes == 0) && ( header->va_recv->pkgnum ==1 )  )					 
			      {
				              struct iovec v[1];
											v[0].iov_base = (void *)header->va_recv->lost_list;
											v[0].iov_len  = 4;
		 
										    header->va_recv->ack.wrate=0;   //window rate to zero = end condition
															  
				              datagram->_ack_model.postPacket (header->va_recv->pkt,
																NULL,
																header->va_recv->info.cookie,
																header->va_recv->fromRank,
																&header->va_recv->ack,
																sizeof (header_ack_t),
																v);
				  			
                              TRACE_ERR((stderr,"   DatagramSimple::process_short_data() ..  all data Received \n")); 
				   
							  CloseRxTimer(header->va_recv, 0); 	 //close timer
							  CloseRxTimer(header->va_recv, 1);   //close timer
				   
				              header->va_recv->datagram->_lastva_send=NULL; //set to NULL
				              
							  //call terminate function
					          receive_complete (XMI_Client_getcontext(datagram->_client, datagram->_contextid),(void *) header->va_recv, XMI_SUCCESS);
                   
				   
                       return 0;
			    }
			   
				  
	      	 return 0;
          };
		 
		  
		  //Control mechanism
		  static int control_ack(header_metadata_t * header){
		     
			 struct iovec v[1];
			 
					if(header->va_recv->rpkg == header->va_recv->ack.wsize){
			   
			                  
							  header->va_recv->rpkg = 0;                                       //reset for next window
														  
					 ///Implement control mechanism 
							  //Check from winseq to maxseq
							  for(size_t i = header->va_recv->wminseq ; i< header->va_recv->wmaxseq ; i++){
							  
							        //TRACE_ERR((stderr,"   DatagramSimple::control_ack() ..  wminseq =%d, wmaxseq=%d, rcv_list[%d]=%d  i = %d \n",header->va_recv->wminseq, header->va_recv->wmaxseq, i, header->va_recv->rcv_list[i], i )); 
							          
									  if (header->va_recv->rcv_list[i] == i){
									      if(header->va_recv->fseq)
							                header->va_recv->wminseq++; //update minimun seqno received OK
										   
									  }else{
									        header->va_recv->fseq = false; //stop saving min seqno
											header->va_recv->lost_list[header->va_recv->nlost++] = i;  //update lost list
									 }		
								}	
                                     header->va_recv->fseq = true;  //reset flag
							

							 //Update wrate and wsize
							  if (header->va_recv->last_nlost < header->va_recv->nlost){
							         header->va_recv->ack.wsize = header->va_recv->ack.wsize/2;      //decrement window size
							         header->va_recv->ack.wrate = header->va_recv->ack.wrate/2;      //decrement window rate
							  }else{			  
							         header->va_recv->ack.wsize = header->va_recv->ack.wsize*2;      //increment window size
							         header->va_recv->ack.wrate = header->va_recv->ack.wrate*2;      //increment window rate
							  }
							  	 
								 header->va_recv->last_nlost = header->va_recv->nlost; //save nlost
					
					
					
                                 TRACE_ERR((stderr, "   DatagramSimple::control_ack()  Window size = %d , wminseq = %d , wmaxseq = %d , nlost = %d ",header->va_recv->ack.wsize, header->va_recv->wminseq ,header->va_recv->wmaxseq ,header->va_recv->nlost ));
											
                                
								  v[0].iov_base = (void *)header->va_recv->lost_list;
								  v[0].iov_len  = 4* header->va_recv->nlost;
								
								
								  if ((header->va_recv->wmaxseq+1 == header->va_recv->pkgnum)&& (header->va_recv->nlost==0)){
								           header->va_recv->ack.wrate=0;         //end condition  
									  
									  
									  //post ack package 
										header->va_recv->datagram->_ack_model.postPacket (header->va_recv->pkt,
																			NULL,
																			header->va_recv->info.cookie,
																			header->va_recv->fromRank,
																			&header->va_recv->ack,
																			sizeof (header_ack_t),
																			v);
										
										TRACE_ERR((stderr,"   DatagramSimple::control_ack() ..  all data Received \n")); 
							   
							   
										CloseRxTimer(header->va_recv, 0); 	//close timer
										CloseRxTimer(header->va_recv, 1);   //close timer

										header->va_recv->datagram->_lastva_send=NULL; 	

										//call terminate function
										receive_complete (XMI_Client_getcontext(header->va_recv->datagram->_client, header->va_recv->datagram->_contextid),(void *) header->va_recv, XMI_SUCCESS);
																
									
										  
									}							
									else{
									
									 //Post ack package
										   header->va_recv->datagram->_ack_model.postPacket (header->va_recv->pkt,
																NULL,
																header->va_recv->info.cookie,
																header->va_recv->fromRank,
																&header->va_recv->ack,
																sizeof (header_ack_t),
																v);
									
									
									  TRACE_ERR((stderr, ">>   DatagramSimple::control_ack() Packed was received and ack was sent\n"));	
								
							        	}
					///End control Mechanism				
		  
		       }
			   return 0;
			}
		  
		  
		  
		  
		
          ///
          /// \brief Direct send data   packet dispatch.
          ///
          static int dispatch_data_direct   (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {				
			 TRACE_ERR((stderr, ">> DatagramSimple::process_data()\n"));
			 
              DatagramSimple<T_Model, T_Device, T_LongHeader> * datagram=
                (DatagramSimple<T_Model, T_Device, T_LongHeader> *) recv_func_parm;
			  
			  		   
		      //Pointer to metadata
		   	   header_metadata_t * header = (header_metadata_t *) metadata;
		   
		      StopRxTimer(header->va_recv, 1);     //stop timer; 	
			  
		      
		        //Save maximun received seqno  
		  		if(header->seqno > header->va_recv->wmaxseq)
				      header->va_recv->wmaxseq = header->seqno; 
								
                
				 //save received seqno 
		  		 header->va_recv->rcv_list[header->seqno] = header->seqno;
				  
				 TRACE_ERR((stderr,"   DatagramSimple::process_data() ..  wminseq =%d, wmaxseq=%d, rcv_list[%d]=%d  header->seqno = %d \n",header->va_recv->wminseq, header->va_recv->wmaxseq, header->seqno, header->va_recv->rcv_list[header->seqno], header->seqno )); 
							        
				 TRACE_ERR((stderr, "   DatagramSimple::process_data() .. header_metadata_t coming inside metadata\n"));
				    
				 
				  //copy data to buffer
				  memcpy ((char *)(header->va_recv->info.data.simple.addr) + ((header->seqno-1)*header->bsend + header->va_recv->fbytes), (payload), header->bsend);      				  
			   
		       
                 //Update total of bytes received
                 header->va_recv->info.data.simple.bytes -= header->bsend;
			 
			     //update total of received packages 
			     ++header->va_recv->rpkg;
			   
			     TRACE_ERR((stderr,"   DatagramSimple::process_data() .. buffer address =%p ,  bytes for receiving =%d, offset = %d, bsend = %d\n", header->va_recv->info.data.simple.addr, header->va_recv->info.data.simple.bytes , header->seqno, header->bsend));
			 
			     TRACE_ERR((stderr, "   DatagramSimple::process_data() Window size = %d, rpkg = %d , pkgnum = %d .  Protocol header_info_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n",header->va_recv->ack.wsize, header->va_recv->rpkg,header->va_recv->pkgnum ));
							
			     struct iovec v[1];
						  
			    //are all bytes received?
				if ((header->va_recv->info.data.simple.bytes == 0) && (header->va_recv->wmaxseq+1 == header->va_recv->pkgnum))					 
			      {
							header->va_recv->lost_list[0] =0;    //nothing lost
							header->va_recv->nlost = 0;
							v[0].iov_base = (void *)header->va_recv->lost_list;
							v[0].iov_len  = 4* header->va_recv->nlost;
							header->va_recv->ack.wrate=0;         //end condition
			                
							//post ack package 
				            datagram->_ack_model.postPacket (header->va_recv->pkt,
																NULL,
																header->va_recv->info.cookie,
																header->va_recv->fromRank,
																&header->va_recv->ack,
																sizeof (header_ack_t),
																v);
				  			
							TRACE_ERR((stderr,"   DatagramSimple::process_data() ..  all data Received \n")); 
				   
                   
							CloseRxTimer(header->va_recv, 0); 	//close timer
							CloseRxTimer(header->va_recv, 1);   //close timer

							header->va_recv->datagram->_lastva_send=NULL; 	

                            //call terminate function
                            receive_complete (XMI_Client_getcontext(datagram->_client, datagram->_contextid),(void *) header->va_recv, XMI_SUCCESS);
                                 					
				  			 
                     return 0;
					 
					 
					
			     } else  {
				 
				            //call control function
							 control_ack((header_metadata_t *)header);
				 		  	
				} 
				  
			  ResetRxTimer(0, 10 ,header->va_recv->ack.wrate, header->va_recv , 1) ; 
								   
				  
	      	 return 0;
          };

		  
		  		
  static int dispatch_ack_direct (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
		  
		   TRACE_ERR((stderr, ">> DatagramSimple::process_ack()\n"));
		  
		   
            TRACE_ERR((stderr, "   DatagramSimple::process_ack() .. header_info coming inside metadata\n"));   		   
           
		   //ack is inside metadata		   
		    header_ack_t * ack = (header_ack_t *) metadata; 
		    
			//lost list is coming inside payload
		    size_t * lost = (size_t *) payload;
		    
			size_t i=0;
			struct iovec v[1];
			
            //Pointer to Protocol object
            DatagramSimple<T_Model, T_Device, T_LongHeader> * datagram=
              (DatagramSimple<T_Model, T_Device, T_LongHeader> *) recv_func_parm;
				
   	        
			StopTxTimer(ack->va_send,1); //stop timer

            
			//update window size and rate
			ack->va_send->rts.wsize=ack->wsize;
			ack->va_send->rts.wrate=ack->wrate;
			
			 TRACE_ERR((stderr, "   DatagramSimple::process_ack() .. lost_list =%d ,bytes =%d \n",lost[0], bytes));   		   
			
			//lost package?
			if(lost[0]== 0){
			
								  TRACE_ERR((stderr, "   DatagramSimple::process_ack() .. lost_list empty pkgnum = %d  seqno =%d\n",ack->va_send->pkgnum, ack->va_send->header.seqno));
								  
						
						   if (ack->wrate==0)
								{
								//END CONDITION
								
								CloseTxTimer(ack->va_send, 0);   //close timer
								CloseTxTimer(ack->va_send, 1);   //close timer
								
								TRACE_ERR((stderr, ">>   DatagramSimple::Callback_data_send() wrate==0 .. all data was sent\n"));
								
								send_complete (XMI_Client_getcontext(datagram->_client, datagram->_contextid),(void *) ack->va_send, XMI_SUCCESS);
                                   
								return 0;
							}else{
							        //send more data
								    TRACE_ERR((stderr, ">>   DatagramSimple::Callback_data_send() wrate!=0 .. more data to send\n"));
								   
								    if (ack->va_send->pkgnum-1  > ack->va_send->header.seqno){
								        ack->va_send->pkgsend=0;
								        cb_data_send(XMI_Client_getcontext(datagram->_client, datagram->_contextid), (void *) ack->va_send, XMI_SUCCESS);     
								    }
                                 
								 }

			 }else{
						  TRACE_ERR((stderr, "   DatagramSimple::process_ack() .. lost_list is not empty\n"));
						  ack->va_send->lost_list = lost;
						  ack->va_send->nlost = bytes>>2;
						  
						  TRACE_ERR((stderr, "   DatagramSimple::process_ack() .. lost_list[0] =%d ,nlost =%d \n",lost[0], bytes>>2));   		   

						   // send lost list  (sender)
						   send_lost_list((send_state_t *)ack->va_send);
							
						 
						   TRACE_ERR((stderr, "	DatagramSimple::process_ack() .. last_seqno =%d , header.seqno = %d\n",ack->va_send->last_seqno, ack->va_send->header.seqno));
						 
						  										
 
				
				}
			  
					
                //reset timer 
                ResetTxTimer(0, 10 ,ack->va_send->rts.wrate, ack->va_send , 1) ;
				
		  return 0;
		  };
		  

		

		

          ///
          /// \brief function to send window
          ///

          static int send_window (window_t * window,  size_t iolen, size_t wsize){
		  
		    size_t i= 0;
			
			  
					for ( i = 0; i< WINDOW_SIZE -1 ; i++){
					  //sleep(1);
					
												
							//Check if data to send
									if ((window->va_send->pkgsend == wsize) || ((window->va_send->pkgnum-1) <= window->va_send->header.seqno))
									  {
										window->va_send->pkgsend=0;
										window->va_send->last_seqno = window->va_send->header.seqno+1;
										TRACE_ERR((stderr, ">>   BGNAPSimple::Send_Window() .. all data was sent\n"));
										 

										return 0;
									  }					
								
								

	                         TRACE_ERR((stderr,"   DatagramSimple::Send_Window() .. window= %p , iolen =%d, wsize = %d, window->va_send->header.seqno =%d , window->va_send->pkgsend = %d , window->va_send->pkgnum =%d \n" ,window,iolen, wsize,window->va_send->header.seqno,window->va_send->pkgsend, window->va_send->pkgnum ));
			       
		                			   
														
								
								 if (sizeof(header_metadata_t) <= T_Model::packet_model_metadata_bytes)
												  {
										window->va_send->header.bsend = T_Model::packet_model_payload_bytes;                ///Update bytes to window->va_send
								  }
								else
								  {
										window->va_send->header.bsend = (T_Model::packet_model_payload_bytes - sizeof(header_metadata_t));              ///Update bytes to window->va_send
								  }  
								
								  ++window->va_send->header.seqno;        //increment seqno
             
			                      ++window->va_send->pkgsend;             //increment pksens
								
							  

								//TRACE_ERR((stderr,"   AdaptiveSimple::Callback_data_window->va_send() .. cookie= %p , window->va_send->cts->va_window->va_send = %p ,window->va_send->cb_data = %p\n" ,cookie,window->va_send->rts.va_window->va_send, window->va_send->cb_data));
					 
								
                                  //save header info inside package
								  window->pkg[i].header.va_recv= window->va_send->header.va_recv;
								  window->pkg[i].header.seqno= window->va_send->header.seqno;
								  window->pkg[i].header.bsend= window->va_send->header.bsend;
									 
									
														  
														  
								  struct iovec v1[1];   
                                  struct iovec v2[2];		 
					              
								  
								  //update iovec
								  if(iolen==2){  
									   v2[0].iov_base = (void *) &window->pkg[i].header;
									   v2[0].iov_len  = sizeof (header_metadata_t);
									   v2[1].iov_base = (void *)(window->va_send->send_buffer + ((window->va_send->header.seqno-1)*T_Model::packet_model_payload_bytes + window->va_send->fbytes));
									   v2[1].iov_len  = (size_t)window->pkg[1].header.bsend;
								  } else{
								   					   
									   v1[0].iov_base = (void *)(window->va_send->send_buffer + ((window->va_send->header.seqno-1)*T_Model::packet_model_payload_bytes + window->va_send->fbytes));
									   v1[0].iov_len  = (size_t)window->pkg[1].header.bsend;
					               }
						
					
						
								   TRACE_ERR((stderr, "   AdaptiveSimple::Send_Window() .. window pkt = %p , pktno=%d\n",&window->pkg[i].pkt, i));
						      

                                //send data
							    if (iolen==1){  
							         TRACE_ERR((stderr, "   AdaptiveSimple::Send_Window() .. v1[0].iov_base = %p ,v1[0].iov_len=%d\n",v1[0].iov_base, v1[0].iov_len));
						              
										window->va_send->datagram->_data_model.postPacket (window->pkg[i].pkt,
																  NULL ,
																  (void *)NULL,
																  window->va_send->rts.destRank,
																  (void *) &window->pkg[i].header,
																  sizeof (header_metadata_t),
																  v1);
																  
																  
								} else if (iolen==2){
									  
										   window->va_send->datagram->_data_model.postPacket (window->pkg[i].pkt,
																	 NULL,
																	 (void *)NULL,
																	 window->va_send->rts.destRank,
																	 (void *)NULL,
																	 0,
																	 v2);
										}	

                            		
					
					           TRACE_ERR((stderr, "   AdaptiveSimple::Send_Window() pkt(%d).. data total bytes= %d  ,window = %p , seqno = %d , winodw.seqno=%d , bsend=%d \n", i, window->va_send->rts.bytes,  window, window->va_send->header.seqno, window->pkg[i].header.seqno, window->pkg[i].header.bsend));
								
					}									 
					   //call callback
					   
					   
								if ((window->va_send->pkgsend == wsize) || ((window->va_send->pkgnum-1) <= window->va_send->header.seqno))
														  {
														    window->va_send->pkgsend=0;
															window->va_send->last_seqno = window->va_send->header.seqno+1;
															TRACE_ERR((stderr, ">>   BGNAPSimple::Send_Window() .. all data was sent\n"));
															 
				   
															return 0;
														  }			
					   
					   
					    
					            TRACE_ERR((stderr,"   DatagramSimple::Send_Window() .. window= %p , iolen =%d, wsize = %d, window->va_send->header.seqno =%d , window->va_send->pkgsend = %d , window->va_send->pkgnum =%d \n" ,window,iolen, wsize,window->va_send->header.seqno,window->va_send->pkgsend, window->va_send->pkgnum ));
			       
					   
					        // update_header(window);
							 
						        if (sizeof(header_metadata_t) <= T_Model::packet_model_metadata_bytes)
												  {
										window->va_send->header.bsend = T_Model::packet_model_payload_bytes;                ///Update bytes to window->va_send
								  }
								else
								  {
										window->va_send->header.bsend = (T_Model::packet_model_payload_bytes - sizeof(header_metadata_t));              ///Update bytes to window->va_send
								  }  
								
								
								  ++window->va_send->header.seqno;                                 
             
								  ++window->va_send->pkgsend; 									  
									 
								 
								  window->pkg[i].header.va_recv= window->va_send->header.va_recv;
								  window->pkg[i].header.seqno= window->va_send->header.seqno;
								  window->pkg[i].header.bsend= window->va_send->header.bsend;
									 
										
					
					               struct iovec v1[1];   
                                   struct iovec v2[2];		 
					              if(iolen==2){  
									   v2[0].iov_base = (void *) &window->pkg[i].header;
									   v2[0].iov_len  = sizeof (header_metadata_t);
									   v2[1].iov_base = (void *)(window->va_send->send_buffer + ((window->va_send->header.seqno-1)*T_Model::packet_model_payload_bytes + window->va_send->fbytes));
									   v2[1].iov_len  = (size_t)window->pkg[1].header.bsend;
								  } else{
								   					   
									   v1[0].iov_base = (void *)(window->va_send->send_buffer + ((window->va_send->header.seqno-1)*T_Model::packet_model_payload_bytes + window->va_send->fbytes));
									   v1[0].iov_len  = (size_t)window->pkg[1].header.bsend;
					               }
								   
								   						
								
					             //TRACE_ERR((stderr, "   AdaptiveSimple::Callback_data_window->va_send() .. after window->va_send->fblock= %s\n", (window->va_send->fblock)?"true":"false"));						
									
					  
                                TRACE_ERR((stderr, "   AdaptiveSimple::Send_Window() .. window pkt = %p , pktno=%d\n",&window->pkg[i].pkt, i));
							     //pkt_t * dummy = (pkt_t *)malloc(sizeof(pkt_t));							   
							
							    if (iolen==1){
										TRACE_ERR((stderr, "   AdaptiveSimple::Send_Window() .. v1[0].iov_base = %p ,v1[0].iov_len=%d\n",v1[0].iov_base, v1[0].iov_len));
						      
										window->va_send->datagram->_data_model.postPacket (window->pkg[i].pkt,
							                             cb_data_send,
                                                         (void *)window->va_send,
                                                         window->va_send->rts.destRank,
                                                         (void *) &window->pkg[i].header,
                                                         sizeof (header_metadata_t),
                                                         v1);
							
		                        } else if (iolen==2){
						  
						                window->va_send->datagram->_data_model.postPacket (window->pkg[i].pkt,
							                             cb_data_send,
                                                         (void *)window->va_send,
                                                         window->va_send->rts.destRank,
                                                         (void *) NULL,
                                                         0,
                                                         v2);
						  
		                        }
						 
						 
						 //Check if data to send
														
		  
		  return 0;
		  }
		  


		  ///
          /// \brief Callback invoked after send a data_send packet.
          ///

          static void cb_data_send (xmi_context_t   context,
                                     void          * cookie,
                                     xmi_result_t    result)
          {
		  
				     TRACE_ERR((stderr, ">> DatagramSimple::Callback_data_send()\n"));
				
					 //Pointer to send state
					 send_state_t * send = (send_state_t *) cookie;
					
					
					  //Pointer to Protocol 
					  DatagramSimple<T_Model, T_Device, T_LongHeader> * datagram=
					  (DatagramSimple<T_Model, T_Device, T_LongHeader> *) send->pf;
					
						
					  TRACE_ERR((stderr,"   DatagramSimple::Callback_data_send() .. data total bytes= %d  , send->pkgnum = %d ,  send->rts.window = %d, pkgsend = %d , offset=%d , bsend=%d \n", send->rts.bytes, send->pkgnum ,  send->rts.wsize, send->pkgsend,send->header.seqno,T_Model::packet_model_payload_bytes));
					 //TRACE_ERR((stderr,"   DatagramSimple::Callback_data_send() .. cookie= %p , send->header->va_recv = %p ,send->cb_data = %p\n" ,cookie,send->header.va_recv, send->cb_data));
					
					  TRACE_ERR((stderr,"   DatagramSimple::Callback_data_send() .. window= %p , iolen =%d, wsize = %d\n" ,&send->window[0],2, send->rts.wsize));
											
						
					   ///function to send window	
					   send_window ((window_t *) &send->window[0],  1, send->rts.wsize);
					   
								
					   TRACE_ERR((stderr, "   DatagramSimple::Callback_data_send() ..  Stop Sending, send->pkgsend =%d,  send->rts.window= %d , send->header.seqno = %d \n",send->pkgsend , send->rts.wsize, send->header.seqno));
								
					
      
            return;
          }

     
	     ///
          /// \brief Local receive completion event callback.
          ///
          /// This callback will invoke the application local receive
          /// completion callback and free the receive state object
          /// memory.
          ///
          static void receive_complete (xmi_context_t   context,
                                        void          * cookie,
                                        xmi_result_t    result)
          {
				TRACE_ERR((stderr, "BurpSimple::receive_complete() >> \n"));
				
				recv_state_t * rcv = (recv_state_t *) cookie;
				
				 xmi_event_function   local_fn = rcv->info.local_fn;
				 void               * fn_cookie = rcv->info.cookie;
				
				DatagramSimple<T_Model, T_Device, T_LongHeader> * datagram =
				  (DatagramSimple<T_Model, T_Device, T_LongHeader> *) rcv->datagram;
				  
				  //free memory
				  datagram->freeRecvState (rcv);
				  
				  
				  //invoke local_fn
				  if (local_fn)
						  local_fn (XMI_Client_getcontext(datagram->_client, datagram->_contextid),
											  fn_cookie,
											   XMI_SUCCESS);


				   
			   

				TRACE_ERR((stderr, "DatagramSimple::receive_complete() << \n"));
				return;
          };
	 
	 
	      ///
          /// \brief Local send completion event callback.
          ///
          /// This callback will invoke the application local completion
          /// callback function and, if notification of remote receive
          /// completion is not required, free the send state memory.
          ///
          static void send_complete (xmi_context_t   context,
                                     void          * cookie,
                                     xmi_result_t    result)
          {
				TRACE_ERR((stderr, "DatagramSimple::send_complete() >> \n"));
				send_state_t * send = (send_state_t *) cookie;

				DatagramSimple<T_Model, T_Device, T_LongHeader> * datagram =
				  (DatagramSimple<T_Model, T_Device, T_LongHeader> *) send->datagram;
				  
				xmi_event_function   local_fn = send->local_fn;
				xmi_event_function    remote_fn = send->remote_fn;
				void               * fn_cookie = send->cookie;
				 
				//free VA memory 
				datagram->freeSendState (send); 
				
				//invoke local_fn
				if (local_fn != NULL)
				  {
					local_fn (XMI_Client_getcontext(datagram->_client, datagram->_contextid), fn_cookie, XMI_SUCCESS);
				  }
				
				//invoke remote_fn
				if (remote_fn != NULL)
				  {
					remote_fn (XMI_Client_getcontext(datagram->_client, datagram->_contextid), fn_cookie, XMI_SUCCESS);
				  }  
				  
				

				
				TRACE_ERR((stderr, "DatagramSimple::send_complete() << \n"));
				return;
          }
		  
	 
	 
	 
	 
	 
	 
	 
	 
	 
		  
		  ///
          /// \brief Read-access  send rts packet callback.
          ///
         
          static int dispatch_rts_read (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
#if 0
            TRACE_ERR((stderr, "(%zd) DatagramFactory::dispatch_rts_read() .. \n"));

            DatagramFactory<T_Model, T_Device, T_LongHeader> * pf =
              (DatagramFactory<T_Model, T_Device, T_LongHeader> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.

            uint8_t stackData[pf->getDevice()->getPacketPayloadSize()];
            void * p = (void *) & stackData[0];
            pf->getDevice()->readData(channel, (char *) p, bytes);

            dispatch_rts_direct (channel, metadata, p, bytes, recv_func_parm);
#else
            assert(0);
#endif
            return 0;
          };
		  
		  
		  //
          /// \brief Read-access  send ack packet callback.
          ///
		  
		  static int dispatch_ack_read (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
#if 0
            TRACE_ERR((stderr, "(%zd) DatagramFactory::dispatch_rts_read() .. \n"));

            DatagramFactory<T_Model, T_Device, T_LongHeader> * pf =
              (DatagramFactory<T_Model, T_Device, T_LongHeader> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.

            uint8_t stackData[pf->getDevice()->getPacketPayloadSize()];
            void * p = (void *) & stackData[0];
            pf->getDevice()->readData(channel, (char *) p, bytes);

            dispatch_ack_direct (channel, metadata, p, bytes, recv_func_parm);
#else
            assert(0);
#endif
            return 0;
          };
		  
		  
		  
		  ///
          /// \brief Read-access  send rts_ack packet callback.
          ///
		  
		  
		  static int dispatch_rts_ack_read (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
#if 0
            TRACE_ERR((stderr, "(%zd) DatagramFactory::dispatch_rts_ack_read() .. \n"));

            DatagramFactory<T_Model, T_Device, T_LongHeader> * pf =
              (DatagramFactory<T_Model, T_Device, T_LongHeader> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.

            uint8_t stackData[pf->getDevice()->getPacketPayloadSize()];
            void * p = (void *) & stackData[0];
            pf->getDevice()->readData(channel, (char *) p, bytes);

            dispatch_rts_ack_direct (channel, metadata, p, bytes, recv_func_parm);
#else
            assert(0);
#endif
            return 0;
          };
		  
		  
		 
		  ///
          /// \brief Read-access  send rts_data packet callback.
          ///
		  
		  static int dispatch_rts_data_read (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
#if 0
            TRACE_ERR((stderr, "(%zd) DatagramFactory::dispatch_rts_data_read() .. \n"));

            DatagramFactory<T_Model, T_Device, T_LongHeader> * pf =
              (DatagramFactory<T_Model, T_Device, T_LongHeader> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.

            uint8_t stackData[pf->getDevice()->getPacketPayloadSize()];
            void * p = (void *) & stackData[0];
            pf->getDevice()->readData(channel, (char *) p, bytes);

            dispatch_rts_data_direct (channel, metadata, p, bytes, recv_func_parm);
#else
            assert(0);
#endif
            return 0;
          };

          
		  ///
          /// \brief Read-access  send data packet callback.
          ///
          
          static int dispatch_data_read     (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
#if 0
            TRACE_ERR((stderr, "DatagramFactory::dispatch_data_read() .. \n"));

            DatagramFactory<T_Model, T_Device, T_LongHeader> * pf =
              (DatagramFactory<T_Model, T_Device, T_LongHeader> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.

            uint8_t stackData[pf->getDevice()->getPacketPayloadSize()];
            void * p = (void *) & stackData[0];
            pf->getDevice()->readData(channel, (char *) p, bytes);

            dispatch_data_read (channel, metadata, p, bytes, recv_func_parm);
#else
            assert(0);
#endif
            return 0;
          };
		  
		  ///
          /// \brief Read-access  send data packet callback.
          ///
          
          static int dispatch_short_data_read     (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
#if 0
            TRACE_ERR((stderr, "DatagramFactory::dispatch_data_read() .. \n"));

            DatagramFactory<T_Model, T_Device, T_LongHeader> * pf =
              (DatagramFactory<T_Model, T_Device, T_LongHeader> *) recv_func_parm;

            // This packet device DOES NOT provide the data buffer(s) for the
            // message and the data must be read on to the stack before the
            // recv callback is invoked.

            uint8_t stackData[pf->getDevice()->getPacketPayloadSize()];
            void * p = (void *) & stackData[0];
            pf->getDevice()->readData(channel, (char *) p, bytes);

            dispatch_short_data_read (channel, metadata, p, bytes, recv_func_parm);
#else
            assert(0);
#endif
            return 0;
          };
		  
		  
	
      };
	  template <class T_Model, class T_Device, bool T_LongHeader >
	  Queue * DatagramSimple< T_Model,  T_Device, T_LongHeader>::_lostqueue = NULL;
	  
	  template <class T_Model, class T_Device, bool T_LongHeader >
	  Queue * DatagramSimple< T_Model,  T_Device, T_LongHeader>::_recvqueue = NULL;
	  
	  template <class T_Model, class T_Device, bool T_LongHeader >
	  Queue * DatagramSimple< T_Model,  T_Device, T_LongHeader>::_queue = NULL;
    };
  };
};
#undef TRACE_ERR
#endif // __xmi_p2p_protocol_send_datagram_datagramsimple_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
