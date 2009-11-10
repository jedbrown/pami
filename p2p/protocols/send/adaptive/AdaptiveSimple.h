/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* ---------------------------------------------------------------- */
/* (C)Copyright IBM Corp.  2009, 2009                               */
/* IBM CPL License                                                  */
/* ---------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
///
/// \file p2p/protocols/send/eager/EagerSimple.h
/// \brief Simple eager send protocol for reliable devices.
///
/// The EagerSimple class defined in this file uses C++ templates and
/// the device "message" interface - which also uses C++ templates.
///
/// C++ templates require all source code to be #include'd from a header file.
///
#ifndef __xmi_p2p_protocol_send_adaptive_adaptivesimple_h__
#define __xmi_p2p_protocol_send_adaptive_adaptivesimple_h__

#include "components/memory/MemoryAllocator.h"

#include "p2p/protocols/send/adaptive/AdaptiveConnection.h"
#include "util/queue/Queue.h"


#ifndef TRACE_ERR
#define TRACE_ERR(x) //fprintf x
#endif

namespace XMI
{
  namespace Protocol
  {
    namespace Send
    {

      ///
      /// \brief Adaptive simple send protocol class for reliable network devices.
      ///
      /// \tparam T_Model   Template packet model class
      /// \tparam T_Device  Template packet device class
      /// \tparam T_Message Template packet message class
      ///
      /// \see XMI::Device::Interface::PacketModel
      /// \see XMI::Device::Interface::PacketDevice
      ///
      template <class T_Model, class T_Device, bool T_LongHeader>
      class AdaptiveSimple
      {
        protected:
		
		  typedef uint8_t pkt_t[T_Model::packet_model_state_bytes];
      		
		//forward declaration
		  class  send_state_t;
		  struct recv_state_t;
		  
		  
		 //cts_info 
		   struct cts_info_t{
			   send_state_t *                   va_send;   ///virtual address sender
               recv_state_t *                   va_recv;   ///virtual address receiver
               size_t                           bytes;	   ///total bytes to send
               xmi_task_t                       destRank;  /// destination rank	
            }; 
		
       			
        //receiver status 
		  struct recv_state_t
        {   
		  cts_info_t          cts;                               /// cts info
		  pkt_t               pkt;                            /// Message Object	  
          AdaptiveSimple<T_Model, T_Device, T_LongHeader> * adaptive;  /// Pointer to protocol
          xmi_recv_t     info;                                   /// Application receive information.
  		  size_t                           count;                /// Message info bytes	
		  xmi_task_t                       fromRank;             ///origen rank
          char *                           msgbuff;              /// RTS data buffer
		  size_t                           msgbytes;		     /// RTS bytes received
        };		
			
				
		  //data header (metadata)
		  struct __attribute__((__packed__)) header_metadata_t
        {
          recv_state_t *            va_recv;   ///virtual address receiver
          size_t                    offset;    ///Offset 
		  size_t                    bsend;     ///bytes send
		};
		
		
		 //rts_ack header (metadata)
		struct __attribute__((__packed__)) header_rts_ack_t
        {
          recv_state_t *            va_recv;     ///virtual address receiver
		  send_state_t *            va_send;     ///virtual address receiver
          size_t                    destRank;    ///Offset 
		};
		
		
		  //rts_info
		struct rts_info_t
            {
              send_state_t *               va_send;	  ///virtual address sender
              size_t                       bytes;	  ///total bytes to send	
              xmi_task_t                   fromRank;  ///origen rank	 		  
			  xmi_task_t                   destRank;  ///target rank
              size_t                       count;     /// Number of quads to send				  
           };
			
			
		
      
		//send_state_class
		
		class send_state_t: public QueueElem  
		 {
          public:
		  
		  size_t               sendlen;         /// Number of bytes to send from the origin rank.
          size_t               next_offset;	    /// Next offset
		  recv_state_t *       va_recv;         /// Receiver Virtual Address
		  size_t               send_bytes;      /// Total bytes to send
		  char *               send_buffer;     /// Send Buffer address
		  pkt_t                pkt;          /// packet
		  rts_info_t           rts;             /// RTS struct info
		  header_metadata_t    header;          /// header_metadata
		  xmi_event_function   cb_data;         /// Callback to execute when data have been sent
		  xmi_event_function   cb_rts;          /// Callback to execute when rts have been sent
		  void   *             pf;              /// Pointer to receiver parameters
		  void                 * msginfo;       /// Message info
          xmi_event_function   local_fn;        /// Local Completion callback.
          xmi_event_function   remote_fn;       /// Remote Completion callback.
          void                 * cookie;        /// Cookie
		
          AdaptiveSimple<T_Model, T_Device, T_LongHeader> * adaptive;
		  
        };
		
		
	 
          

         

        public:

          ///
          /// \brief Adaptive simple send protocol constructor.
          ///
          /// \param[in]  dispatch     Dispatch identifier
          /// \param[in]  dispatch_fn  Dispatch callback function
          /// \param[in]  cookie       Opaque application dispatch data
          /// \param[in]  device       Device that implements the message interface
          /// \param[in]  origin_task  Origin task identifier
          /// \param[in]  context      Communication context
          /// \param[out] status       Constructor status
          ///
          inline AdaptiveSimple (size_t                     dispatch,
                              xmi_dispatch_callback_fn   dispatch_fn,
                              void                     * cookie,
                              T_Device                 & device,
                              xmi_task_t                 origin_task,
                              xmi_context_t              context,
                              size_t                     contextid,
                              xmi_result_t             & status) :
              _rts_model (device, context),
			  _rts_ack_model (device, context),
			  _rts_data_model (device, context),
              _data_model (device, context),
              _cts_model (device, context),
              _device (device),
              _fromRank (origin_task),
              _context (context),
              _dispatch_fn (dispatch_fn),
              _cookie (cookie),
              _connection ((void **)NULL),
              _connection_manager (device),
              _contextid (contextid)
			 {
		    // ----------------------------------------------------------------
            // Compile-time assertions
            // ----------------------------------------------------------------

            // This protocol only works with reliable networks.
            COMPILE_TIME_ASSERT(T_Model::reliable_packet_model == true);

            

            // Assert that the size of the packet metadata area is large
            // enough to transfer a single xmi_task_t. This is used in the
            // various postMessage() calls to transfer long header and data
            // messages.
            COMPILE_TIME_ASSERT(sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes);

            // Assert that the size of the packet payload area is large
            // enough to transfer a single virtual address. This is used in
            // the postPacket() calls to transfer the ack information.
            COMPILE_TIME_ASSERT(sizeof(void *) <= T_Model::packet_model_payload_bytes);

            // ----------------------------------------------------------------
            // Compile-time assertions
            // ----------------------------------------------------------------

		  
		  
            _connection = _connection_manager.getConnectionArray (context);
			
			
			   //No tested
		       //Queue Setup
			   //allocate memory
			        _queue = (Queue *) malloc(sizeof(Queue)* _device.peers());
				    
										
					//Initializing queue
				    for(size_t i=0;i< _device.peers();i++)
				        new (&_queue[i]) Queue();
								
	                  

            TRACE_ERR((stderr, "AdaptiveSimple() [0]\n"));
            status = _rts_model.init (dispatch,
                                           dispatch_rts_direct, this,
                                           dispatch_rts_read, this);
           
            TRACE_ERR((stderr, "AdaptiveSimple() [1] status = %d\n", status));
            if (status == XMI_SUCCESS)
              {
			  
			     status = _rts_ack_model.init (dispatch,
                                              dispatch_rts_ack_direct, this,
                                              dispatch_rts_ack_read, this);
				 TRACE_ERR((stderr, "AdaptiveSimple() [2] status = %d\n", status));							  
                
              

                 if (status == XMI_SUCCESS)
                  {
                        status = _rts_data_model.init (dispatch,
                                              dispatch_rts_data_direct, this,
                                              dispatch_rts_data_read, this);
						 TRACE_ERR((stderr, "AdaptiveSimple() [3] status = %d\n", status));					  
					
					if (status == XMI_SUCCESS)
                           {
                                status = _data_model.init (dispatch,
                                           dispatch_data_direct, this,
                                           dispatch_data_read, this);
                                TRACE_ERR((stderr, "AdaptiveSimple() [4] status = %d\n", status));
					
					
					    if (status == XMI_SUCCESS)
                            {
                             status = _cts_model.init (dispatch,
                                              dispatch_cts_direct, this,
                                              dispatch_cts_read, this);
						      TRACE_ERR((stderr, "AdaptiveSimple() [5] status = %d\n", status));					  
						 
						    }					  
                 
                    }						  
                 
                  }
              }
			  
			  
          }

          ///
          /// \brief Start a new simple send adaptive operation.
          ///
          /// \see XMI::Protocol::Send:simple
          ///
          inline xmi_result_t simple_impl (xmi_send_simple_t * parameters)
          {
           return simple_impl (parameters->simple.local_fn,
                               parameters->simple.remote_fn,
                               parameters->send.cookie,
                               parameters->send.task,
                               parameters->simple.addr,
                               parameters->simple.bytes,
                               parameters->send.header.addr,
                               parameters->send.header.bytes);
          }

          inline xmi_result_t simple_impl (xmi_event_function   local_fn,
                                           xmi_event_function   remote_fn,
                                           void               * cookie,
                                           xmi_task_t           peer,
                                           void               * src,
                                           size_t               bytes,
                                           void               * msginfo,
                                           size_t               mbytes)
          {
            
			TRACE_ERR((stderr,"*** Adpative_AdaptiveSimple implemented , msginfo_bytes = %d  , bytes =%d ***\n",mbytes, bytes));
			TRACE_ERR((stderr," T_Model::packet_model_metadata_bytes = %d\n T_Model::packet_model_payload_bytes = %d \n sizeof(rts_info_t) = %d\n sizeof(cts_info_t) = %d\n sizeof(send_state_t) = %d\n sizeof(recv_state_t) = %d\n sizeof(header_metadata_t) = %d\n sizeof(header_rts_ack_t) = %d\n",T_Model::packet_model_metadata_bytes,T_Model::packet_model_payload_bytes , sizeof(rts_info_t),sizeof(cts_info_t), sizeof(send_state_t),sizeof(recv_state_t),sizeof(header_metadata_t),sizeof(header_rts_ack_t)));  					
     
			
            // Allocate memory to maintain the state of the send.
            send_state_t * send = allocateSendState ();
            
					  
			  
			  
			
			//Save data in send_state_t
              send->cookie   = cookie;                         ///save cookie
              send->local_fn = local_fn;                       ///save Callback function to execute after local done
			  send->remote_fn = remote_fn;                     ///save Callback function to execute after remote done
			  send->adaptive    = this;                           ///Save pointer to protocol
				 
			  send->send_buffer=(char *) src;                  ///sender buffer address		  
			  send->next_offset=0;                             ///initialized next_offset for RTS
			  			  
			  send->rts.fromRank=_fromRank;                   ///Origen Rank
			  send->rts.bytes=bytes;                          ///Total bytes to send
			  send->rts.va_send= send;                        ///Virtual Address sender
			  send->rts.destRank=peer;                        ///Target Rank
			  send->rts.count = mbytes;                       ///Number of  Quads
			  send->msginfo=msginfo;                          ///Message info
			  
			  send->sendlen=bytes;                            ///Total bytes to send
			  
			  send->cb_data  =  cb_data_send;                 ///register Callback function to execute after data package have been sent
			  send->cb_rts  =  cb_rts_send;                     ///register Callback function to execute after rts package have been sent
			  
              send->header.offset=0;                             //initialized RTS offset
			  send->header.bsend=0;                             //initialized RTS bytes
			  
			  TRACE_ERR((stderr,"\nInfo sender  cookie=%p , send =%p , Sender rank  = %d, msinfo= %p , msgbytes = %d , bytes = %d \n",cookie, send, peer,   msginfo,  mbytes, bytes));
			
			
			 //Send if queue is empty
			 if (_queue[_fromRank].isEmpty()){  
		         	 

                 if (bytes == 0){
				 
				 
				      // This branch should be resolved at compile time and optimized out.
                      if (sizeof(rts_info_t) <= T_Model::packet_model_metadata_bytes)
                            {
							 TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. zero-byte data special case, protocol rts_info_t fits in the packet metadata\n"));

				 
								if (mbytes > T_Model::packet_model_payload_bytes)
								    {
									
									TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. zero-byte data special case, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload\n"));

						                 
										if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											
											  TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. zero-byte data special case, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does fit in the message metadata\n"));
                                                                  											   
                                                //send RTS info without message info
										        _rts_model.postPacket (send->pkt,         ///T_Message to send
																NULL,                     ///Callback to execute when done 
																(void *) send,            /// Cookie
																peer,                     ///Target Rank
																(void *)&send->rts,       ///rts_info_t struct  metadata
																sizeof(rts_info_t),       ///sizeof rts_info_t  metadata
																(void *)NULL,             ///Message info
																0);  				      ///Number of bytes
											
											   
										}else
											{
											TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. zero-byte data special case, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does not fit in the message metadata\n"));
                     						XMI_abort();
											}								
									}				
									else
									{
									
									TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. zero-byte data special case, protocol rts_info_t fits in the packet metadata, application metadata fit in a single packet payload\n"));

						                
										//Everything OK                               ///Post rts package
								        _rts_model.postPacket (send->pkt,             ///T_Message to send
														local_fn,                     ///Callback to execute when done 
														(void *) cookie,             /// Cookie
														peer,                        ///Target Rank
														(void *)&send->rts,          ///rts_info_t struct metadata
														sizeof(rts_info_t),          ///sizeof rts_info_t metadata
														msginfo,                     ///Message info
														mbytes);                     ///Number of bytes
					
									    //End Sender side, 0 bytes
										if (remote_fn)
											remote_fn (_context, cookie, XMI_SUCCESS);
					
										if (remote_fn == NULL)
											{
											freeSendState (send);
											} 
						
						
									}
				            }else{ //sizeof(rts_info_t) > T_Model::packet_model_metadata_bytes
							
							    TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. zero-byte data special case, protocol rts_info_t does not fit in the packet metadata\n"));

				 
								  if (mbytes > (T_Model::packet_model_payload_bytes - sizeof(rts_info_t)))
									 {
					                     TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. zero-byte data special case, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload\n"));

							
										if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{
											 
											 TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. zero-byte data special case, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does fit in the message metadata\n"));
                                               
																					
                                                _rts_model.postPacket (send->pkt,         ///T_Message to send
																NULL,                     ///Callback to execute when done 
																(void *) send,            /// Cookie
																peer,                     ///Target Rank
																(void *)NULL,             ///Metadata 
																0,                        ///Metadata size
																(void *)&send->rts,       ///rts_info_t struct
																sizeof(rts_info_t),       ///sizeof rts_info_t
																(void *)NULL,             ///Message info
																0);  		              ///Number of bytes
																							  
											
											
											
										}else{
										     TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. zero-byte data special case, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does not fit in the message metadata\n"));
											XMI_abort();
										}
					                 }
									 else{
						                  TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. zero-byte data special case, protocol rts_info_t does not fit in the packet metadata, application metadata fits in a single packet payload\n"));
    
											  //OK CASE
						                     _rts_model.postPacket (send->pkt,       ///T_Message to send
														local_fn,                    ///Callback to execute when done 
														(void *) cookie,             /// Cookie
														peer,                        ///Target Rank
														(void *)NULL,                ///Metadata 
														0,                           ///Metadata size
														(void *)&send->rts,          ///rts_info_t struct
														sizeof(rts_info_t),          ///sizeof rts_info_t
														msginfo,                     ///Message info
														mbytes);  		             ///Number of bytes
					
					                    //End Sender side, 0 bytes
									    if (remote_fn)
											remote_fn (_context, cookie, XMI_SUCCESS);
					
										if (remote_fn == NULL)
											{
											freeSendState (send);
											} 
						 
									}
							}
			    }else{
				
				        if (sizeof(rts_info_t) <= T_Model::packet_model_metadata_bytes)
                            {		
                                 TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t fits in the packet metadata\n"));
							
						   
				               if (mbytes > T_Model::packet_model_payload_bytes)
                                  {
								  
								  TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload\n"));
							  
				                     if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
                                        {
										
										TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does fit in the message metadata\n"));
  	    
										        _rts_model.postPacket (send->pkt,           ///T_Message to send
																NULL,                       ///Callback to execute when done 
																(void *) send,              /// Cookie
																peer,                       ///Target Rank
																(void *)&send->rts,         ///rts_info_t struct metadata
																sizeof(rts_info_t),         ///sizeof rts_info_t metadata
																(void *)NULL,               ///Message info
																0);  				        ///Number of bytes
										
										
										
						             }else{
									     TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does not fit in the message metadata\n"));
           				                 XMI_abort();
						                  }
					              }else
					                  {
									   TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t fits in the packet metadata, application metadata fit in a single packet payload\n"));

									   //Everything OK
									    _rts_model.postPacket (send->pkt,
																NULL,
																(void *) cookie ,
																peer,
																(void *)&send->rts,
																sizeof(rts_info_t),
																msginfo,
																mbytes);
									  
					   
					              }
					        }else
					            {
								
								TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t does not fit in the packet metadata\n"));

					            if (mbytes > (T_Model::packet_model_payload_bytes - sizeof(rts_info_t)))
                                     {
									     			
                                       TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload\n"));
  				 
                    				 if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
                                             {
											 TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does fit in the message metadata\n"));
              
										        _rts_model.postPacket (send->pkt,         ///T_Message to send
																NULL,                     ///Callback to execute when done 
																(void *) send,            ///Cookie
																peer,                     ///Target Rank
																(void *)NULL,             ///Metadata 
																0,                        ///Metadata size
																(void *)&send->rts,       ///rts_info_t struct
																sizeof(rts_info_t)        ///sizeof rts_info_t
																);  	
									 
										
											 
											 
											 
								         }else{
										        TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does not fit in the message metadata\n"));
		 					                  XMI_abort();
									           }
								     }else
								          {
										  
										  TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t does not fit in the packet metadata, application metadata fits in a single packet payload\n"));
    	
										     _rts_model.postPacket (send->pkt,
																NULL,
																(void *) cookie ,
																peer,
																(void *)NULL,
																0,
																(void *)&send->rts,
																sizeof(rts_info_t),
																msginfo,
																mbytes);
								   
								          }
								}
								   
						  
					}
            }					
						
                   				
		
		
		      //Save in queue object to send
			  if (bytes!=0)
		          _queue[_fromRank].pushTail(send);  
			
			
			
                    
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
            TRACE_ERR((stderr, ">> AdaptiveSimple::setConnection(%zd, %p) .. _connection[%zd] = %p\n", task, arg, peer, _connection[peer]));
            XMI_assert(_connection[peer] == NULL);
            _connection[peer] = arg;
            TRACE_ERR((stderr, "<< AdaptiveSimple::setConnection(%zd, %p)\n", task, arg));
          }

          inline void * getConnection (xmi_task_t task)
          {
            size_t peer = _device.task2peer (task);
            TRACE_ERR((stderr, ">> AdaptiveSimple::getConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
            XMI_assert(_connection[peer] != NULL);
            TRACE_ERR((stderr, "<< AdaptiveSimple::getConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
            return _connection[peer];
          }

          inline void clearConnection (xmi_task_t task)
          {
            size_t peer = _device.task2peer (task);
            TRACE_ERR((stderr, ">> AdaptiveSimple::clearConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
            _connection[peer] = NULL;
            TRACE_ERR((stderr, "<< AdaptiveSimple::clearConnection(%zd) .. _connection[%zd] = %p\n", task, peer, _connection[peer]));
          }


          MemoryAllocator < sizeof(send_state_t), 16 > _send_allocator;
          MemoryAllocator < sizeof(recv_state_t), 16 > _recv_allocator;

          T_Model         _rts_model;
		  T_Model         _rts_ack_model;
          T_Model         _rts_data_model;
		  T_Model         _data_model;
          T_Model         _cts_model;
          T_Device      & _device;
          xmi_task_t      _fromRank;
          xmi_context_t   _context;
          xmi_dispatch_callback_fn   _dispatch_fn;
          void                     * _cookie;
          void ** _connection;
          // Support up to 100 unique contexts.
          //static adaptive_connection_t _adaptive_connection[];
          AdaptiveConnection<T_Device> _connection_manager;
          size_t      _contextid;
		  static Queue *    _queue;
		  
		  		  
		  ///
          /// \brief Direct send rts packet dispatch.
          ///
		  
		   static int dispatch_rts_direct (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
		   
			TRACE_ERR((stderr, ">> AdaptiveSimple::process_rts()\n"));
		   
			void * msginfo;             
			rts_info_t * send;          
			size_t total_payload =0;
			  
			AdaptiveSimple<T_Model, T_Device, T_LongHeader> * adaptive =
              (AdaptiveSimple<T_Model, T_Device, T_LongHeader> *) recv_func_parm;
			
           // Allocate memory to maintain the state of the rcv.			
		    recv_state_t * rcv = adaptive->allocateRecvState ();
			      		
			//save pointer to protocol 
			rcv->adaptive = adaptive;             
		  
		
            			
			 
			    if (sizeof(rts_info_t) <= T_Model::packet_model_metadata_bytes)
                     {
					     TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() .. rts_info_t coming inside metadata\n"));
				         send = (rts_info_t *) metadata;
						 total_payload = T_Model::packet_model_payload_bytes;
						 msginfo = (void *) payload;
			           		 
			    }else{
				        TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() .. rts_info_t coming inside payload\n"));
			            send = (rts_info_t *) payload;         					   
			            total_payload = T_Model::packet_model_payload_bytes - sizeof(rts_info_t);
						msginfo = (void *) (send+1);
				}
			
			 
						    
		      
			   
			   //Save data in recv_state_t			  
			  rcv->cts.va_send  = send->va_send;             ///Virtual address sender
			  rcv->cts.destRank = send->destRank;            ///Target Rank
			  rcv->cts.va_recv = rcv;                        ///Virual Address Receiver
			  rcv->cts.bytes=send->bytes;                    ///Total Bytes to send
			  rcv->fromRank  = send->fromRank;              ///Origin Rank
			  rcv->count  = send->count;                    ///Metadata application bytes 
              
			  rcv->msgbuff = (char *) malloc (sizeof (char*)*send->count);  ///Allocate buffer for Metadata
			  rcv->msgbytes = 0;                                            ///Initalized received bytes 
			 
			
			 			
		      if (send->count <= total_payload){	
			  
                           TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() .. Application Metadata sent in only one package\n"));
			            
	  		            			  
							
							// Invoke the registered dispatch function.
							adaptive->_dispatch_fn.p2p (adaptive->_context,  // Communication context
                                     adaptive->_contextid,                //context id
                                     adaptive->_cookie,                   // Dispatch cookie
                                     send->fromRank,                   // Origin (sender) rank
                                     msginfo,                          // Application metadata
                                     send->count,                      // Metadata bytes
                                     NULL,                             // No payload data
                                     send->bytes,                      // Number of msg bytes
                                     (xmi_recv_t *) &(rcv->info));     //Recv_state struct
			  
			             

							// Only handle simple receives .. until the non-contiguous support
							// is available
							XMI_assert(rcv->info.kind == XMI_AM_KIND_SIMPLE);

							if (send->bytes == 0) // Move this special case to another dispatch funtion to improve latency in the common case.
									{
									
									TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() .. Application Metadata send in only one package, 0 bytes received\n"));
			            
									// No data packets will follow this envelope packet. Invoke the
									// recv done callback and, if an acknowledgement packet was
									// requested send the acknowledgement. Otherwise return the recv
									// state memory which was allocated above.
								 
									if (rcv->info.local_fn)
									  rcv->info.local_fn (adaptive->_context,
															rcv->info.cookie,
															XMI_SUCCESS);
									
															
									rcv->adaptive->freeRecvState (rcv);
								  return 0;
								  }
			  
			

			                      								  
								      
								  
										//Post CTS package info
										if (sizeof(cts_info_t) <= T_Model::packet_model_metadata_bytes)
														{
														TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() .. protocol cts_info_t fits in the packet metadata\n"));

																   if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
																		{	
																		
																			TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() ..  protocol cts_info_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));
              										
																						 
																			//Post cts package  
																	        adaptive->_cts_model.postPacket (rcv->pkt,
																							NULL,
																							rcv->info.cookie,
																							send->fromRank,
																							&rcv->cts,             ///rts_info_t struct
																							sizeof (cts_info_t),   ///rts_info_t size
																							(void * )NULL,
																							0
																							);
																	 
																		
																		
																	}else
																		{
																		    TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() ..  protocol cts_info_t fits in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));
																			XMI_abort();
																		}					
														

											
											}else{
											          TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() .. protocol cts_info_t does not fit in the packet metadata\n"));


															if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
																		{	
																		
																	   TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() ..  protocol cts_info_t does not fit in the packet metadata, xmi_task_t  fits in the message metadata\n"));
              																						 			 
																					//Post cts package  
																	   adaptive->_cts_model.postPacket (rcv->pkt,
																							NULL,
																							rcv->info.cookie,
																							send->fromRank,
																							NULL,
																							0,
																							&rcv->cts,
																							sizeof (cts_info_t)
																							);
																		
																		
																	}else
																		{
																		TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() ..  protocol cts_info_t does not fit in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));		
																	    XMI_abort();
																		}					
														
											}				
										
			
			
			
			
												  
												  
												  
												  
                 
                      return 0;
			
			}else{
			   
			   TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() .. Application Metadata will be send on multiple packages\n"));
			            
	  		         			
			   //Initialize rts_ack structure
			   header_rts_ack_t header;
			   header.va_recv = rcv;              //Receiver Virtual Address 
			   header.va_send = send->va_send;    //Sender Virtual Address  
			   header.destRank = send->destRank;  // Target Rank
			 
			   
			                                if (sizeof(header_rts_ack_t) <= T_Model::packet_model_metadata_bytes)
														{
														TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() .. header_rts_ack_t fits in the packet metadata\n"));

														
																   if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
																		{	
																		TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() ..  protocol header_rts_ack_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));
 
																												
																						 
																			//Post rts_ack package  
																	adaptive->_rts_ack_model.postPacket (rcv->pkt,
																							NULL,
																							rcv->info.cookie,
																							send->fromRank,
																							(void *)&header,
																							sizeof (header_rts_ack_t),
																							(void * )NULL,
																							0
																							);
																	 
																		
																		
																	}else
																		{
																		TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() ..  protocol header_rts_ack_t fits in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));
																		XMI_abort();
																		}					
														

											
											}
											
											else{
											        TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() .. protocol header_rts_ack_t does not fit in the packet metadata\n"));


															if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
																		{	
																		
																		TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() ..  protocol header_rts_ack_t does not fit in the packet metadata, xmi_task_t  fits in the message metadata\n"));
    										 			 
																			//Post rts_ack package  
																	        adaptive->_rts_ack_model.postPacket (rcv->pkt,
																							NULL,
																							rcv->info.cookie,
																							send->fromRank,
																							NULL,
																							0,
																							(void *)&header,
																							sizeof (header_rts_ack_t)
																							);
																		
																		
																	}else
																		{
																		TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() ..  protocol header_rts_ack_t does not fit in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));		
																	    XMI_abort();
																		}					
														
											}			  
			   
			   
			}   
			
			
			TRACE_ERR((stderr, ">> AdaptiveSimple::process_rts() Done\n"));
			
			  return 0;
			  
          };
		  
		  
		  
		  ///
          /// \brief Direct send rts_ack   packet dispatch.
          ///
		  
		  static int dispatch_rts_ack_direct (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
		  
		   TRACE_ERR((stderr, ">> AdaptiveSimple::process_rts_ack()\n"));
		  
		  			 
           header_rts_ack_t * ack;
			
			if (sizeof(header_rts_ack_t) <= T_Model::packet_model_metadata_bytes)
                {
				    TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_ack() .. header_rts_ack_t coming inside metadata\n"));    
				    //Pointer to cts_info_t
			        ack= (header_rts_ack_t *) metadata;
					
					//Calculate rts first package bytes
					if((ack->va_send->rts.count % T_Model::packet_model_payload_bytes) == 0)             
				                ack->va_send->header.bsend = T_Model::packet_model_payload_bytes;
				         else
					            ack->va_send->header.bsend = (ack->va_send->rts.count % T_Model::packet_model_payload_bytes); 
						 
							
			}else{
			       TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_ack() .. header_rts_ack_t coming inside payload\n"));   
			       //Pointer to cts_info_t
			        ack= (header_rts_ack_t *) payload; 
					
					
					//Calculate rts first package bytes
					if((ack->va_send->rts.count % (T_Model::packet_model_payload_bytes- sizeof(header_rts_ack_t))) == 0)             
				                ack->va_send->header.bsend = T_Model::packet_model_payload_bytes - sizeof(header_rts_ack_t);
				         else
					            ack->va_send->header.bsend = (ack->va_send->rts.count % (T_Model::packet_model_payload_bytes - sizeof(header_rts_ack_t))); 
						 
						  
			      }
		  
		  
		    //Save receiver VA
			ack->va_send->header.va_recv = ack->va_recv;
			
            //Pointer to Protocol object
            AdaptiveSimple<T_Model, T_Device, T_LongHeader> * adaptive=
              (AdaptiveSimple<T_Model, T_Device, T_LongHeader> *) recv_func_parm;
				
   	
							 		
						
			 TRACE_ERR((stderr,"   AdaptiveSimple::process_rts_ack() .. Application Metadata bytes= %d  , send->next_offset = %d , offset=%d , bsend=%d \n", ack->va_send->rts.count, ack->va_send->next_offset, ack->va_send->header.offset ,ack->va_send->header.bsend));
				  
				
		     //Update next offset
			 ack->va_send->next_offset += ack->va_send->header.bsend;   
				

             //TRACE_ERR((stderr,"   AdaptiveSimple::process_rts_ack(), cookie =%p , destination rank =%d, header=%p , offset = %d, msginfo =%p , bytes to send =%d \n",cookie , send->rts.destRank, &send->header, send->header.offset, send->msginfo, send->header.bsend));
           
		     if (sizeof(header_metadata_t) <= T_Model::packet_model_metadata_bytes)
                            {
							TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_ack() .. protocol header_metadata_t fits in the packet metadata\n"));

							    
							           if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_ack() ..  protocol header_metadata_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));
 
																					
											    //Post rts_data package 			 
												adaptive->_rts_data_model.postPacket (ack->va_send->pkt,
															ack->va_send->cb_rts,
															(void *) ack->va_send,
															ack->va_send->rts.destRank,      
															(void *) &ack->va_send->header,   //header_metadata
															sizeof (header_metadata_t),       //bytes
															(void *)((char *)ack->va_send->msginfo + ack->va_send->header.offset), //Pointer to metadata receiver buffer
															(size_t)ack->va_send->header.bsend);                                   //Send bytes
										 
											
											
										}else
											{
											 TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_ack() ..  protocol header_metadata_t fits in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));
												XMI_abort();
											}					
							

				
			}else{
			          TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_ack() .. protocol header_metadata_t does not fit in the packet metadata\n"));


                      			    if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											
											TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_ack() ..  protocol header_metadata_t does not fit in the packet metadata, xmi_task_t  fits in the message metadata\n"));
     										
											    //Post rts_data package 			 
												adaptive->_rts_data_model.postPacket (ack->va_send->pkt,
															ack->va_send->cb_rts,
															(void *) ack->va_send ,
															ack->va_send->rts.destRank,      // target rank
															(void *) NULL,
															0,
															(void *) &ack->va_send->header,   //header_metadata
															sizeof (header_metadata_t),       //bytes
															(void *)((char *)ack->va_send->msginfo + ack->va_send->header.offset), //Pointer to metadata receiver buffer
															(size_t)ack->va_send->header.bsend);                                   //Send bytes
										 
											
											
										}else
											{
											TRACE_ERR((stderr, "   AdaptiveSimple::process_rts() ..  protocol header_metadata_t does not fit in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));		
										    XMI_abort();
											}					
							
				}						 
										 
										 
										 
										 
										 
				  	
				
              TRACE_ERR((stderr, ">> AdaptiveSimple::process_rts_ack() Done\n"));
            

		  return 0;
		  }
		  
		  
		  
		  ///
          /// \brief Direct send rts_data   packet dispatch.
          ///
		  
		  static int dispatch_rts_data_direct (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
		  	  
		  
		   TRACE_ERR((stderr, ">> AdaptiveSimple::process_rts_data()\n"));
		  
           header_metadata_t * header;
		   void * msginfo;
		   
		   //Pointer to protocol			 
              AdaptiveSimple<T_Model, T_Device, T_LongHeader> * adaptive =
                (AdaptiveSimple<T_Model, T_Device, T_LongHeader> *) recv_func_parm; 
				
				
			
			 if (sizeof(header_metadata_t) <= T_Model::packet_model_metadata_bytes)
                  {
				
				   TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_data() .. header_metadata_t coming inside metadata\n"));
				         
						//Pointer to metadata
						header = (header_metadata_t *) metadata;
				  
				  
				        //copy to buffer           			 
                        memcpy ((char *)(header->va_recv->msgbuff) + header->offset, payload, header->bsend);           
   
                        
				  
				  
				   }  
				 	 
			   else{
			   
			       TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_data() .. header_metadata_t coming inside payload\n"));   
			           //Pointer to metadata
				       header = (header_metadata_t *) payload;
				  
				       //copy to buffer           			 
                       memcpy ((char *)(header->va_recv->msgbuff) + header->offset, (header+1), header->bsend);           
   
                              		

                    }			

					
					
			            //Update total of bytes received
                        header->va_recv->msgbytes += header->bsend;
			 
			             //TRACE_ERR((stderr,"   AdaptiveSimple::process_rts_data() .. received bytes =%d, offset = %d, bsend = %d\n", header->va_recv->msgbytes , header->offset, header->bsend));
			 
				           //Terminate after receiving all the bytes		 			  
			             if (header->va_recv->msgbytes != header->va_recv->count)		  
			                {
				           
				            TRACE_ERR((stderr,"   AdaptiveSimple::process_rts_data() .. received packet\n"));
			 
						   return 0;
						   
						   }else{
						       TRACE_ERR((stderr,"   AdaptiveSimple::process_rts_data() .. all metadadata received\n"));
						        msginfo = header->va_recv->msgbuff;
							   
							   }
			
		   
		   
		   
					
			 //TRACE_ERR((stderr,"   AdaptiveSimple::process_rts_data() ...  header->va_recv->msgbuff  = %p , header->offest = %d , payload = %p , header->bsend = %d \n",header->va_recv->msgbuff, header->offset, payload, header->bsend)); 
			
			 		
					 
			TRACE_ERR((stderr,"   AdaptiveSimple::process_rts_data() ..before  _dispatch_fn.p2p  adaptive->_context = %p , adaptive->_contextid =%d , adaptive->_cookie =%p , header->va_recv->fromRank =%d,   msginfo =%p , header->va_recv->count = %d, header->va_recv->cts.bytes = %d , header->va_recv->info =%p ,adaptive = %p ,_dispatch_fn.p2p = %p \n",adaptive->_context,adaptive->_contextid,adaptive->_cookie,header->va_recv->fromRank,msginfo, header->va_recv->count,  header->va_recv->cts.bytes,  (xmi_recv_t *) &(header->va_recv->info), adaptive ,  adaptive->_dispatch_fn.p2p));
									 
			   // Invoke the registered dispatch function.
              adaptive->_dispatch_fn.p2p (adaptive->_context,              // Communication context
                                     adaptive->_contextid,              // contextid
                                     adaptive->_cookie,                 // Dispatch cookie
                                     header->va_recv->fromRank,      // Origin (sender) rank
                                     msginfo,                        // Application metadata
                                     header->va_recv->count,         // Metadata bytes
                                     NULL,                           // No payload data
                                     header->va_recv->cts.bytes,     // Number of msg bytes
                                     (xmi_recv_t *) &(header->va_recv->info));   //recv_struct
			  
			//free received buffer
			free(header->va_recv->msgbuff); 
			  
			TRACE_ERR((stderr,"   AdaptiveSimple::process_rts_data().. after dispatch_rts_data after p2p rcv->info.data.simple.addr  = %p , header->offest = %d , header->bsend = %d , rcv->info.data.simple.bytes =%d\n",header->va_recv->info.data.simple.addr, header->offset, header->bsend, header->va_recv->info.data.simple.bytes)); 
			  
			          

            // Only handle simple receives .. until the non-contiguous support
            // is available
            XMI_assert(header->va_recv->info.kind == XMI_AM_KIND_SIMPLE);

            if (header->va_recv->cts.bytes == 0) // Move this special case to another dispatch funtion to improve latency in the common case.
              {
                // No data packets will follow this envelope packet. Invoke the
                // recv done callback and, if an acknowledgement packet was
                // requested send the acknowledgement. Otherwise return the recv
                // state memory which was allocated above.
             
			     TRACE_ERR((stderr,"   AdaptiveSimple::process_rts_data().. Done, 0 bytes received\n"));
				    
                 if (header->va_recv->info.local_fn){
                    header->va_recv->info.local_fn (adaptive->_context,
                                        header->va_recv->info.cookie,
                                        XMI_SUCCESS);
				
				//free receiver state
				header->va_recv->adaptive->freeRecvState (header->va_recv);
				}
              return 0;
			  }
			  
			  
			
			if (sizeof(cts_info_t) <= T_Model::packet_model_metadata_bytes)
                            {
							TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_data() .. protocol cts_info_t fits in the packet metadata\n"));

							
							           if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_data() ..  protocol cts_info_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));
 
																					
											    			 
									        //Post cts package  
										    adaptive->_cts_model.postPacket (header->va_recv->pkt,
																NULL,
																header->va_recv->info.cookie,
																header->va_recv->fromRank,
																&header->va_recv->cts,               //cts structure
																sizeof (cts_info_t),           
																(void * )NULL,
																0
																);
										 
											
											
										}else
											{
											 TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_data() ..  protocol cts_info_t fits in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));
											  XMI_abort();
											}					
							

				
				}else{
                                TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_data() .. protocol cts_info_t does not fit in the packet metadata\n"));

                      			if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_data() ..  protocol cts_info_t does not fit in the packet metadata, xmi_task_t  fits in the message metadata\n"));
    										
											    			 			 
											 //Post cts package  
										     adaptive->_cts_model.postPacket (header->va_recv->pkt,
																NULL,
																header->va_recv->info.cookie,
																header->va_recv->fromRank,
																NULL,
																0,
																&header->va_recv->cts,        //cts structure
																sizeof (cts_info_t)
																);
											
											
										}else
											{
											TRACE_ERR((stderr, "   AdaptiveSimple::process_rts_data() ..  protocol cts_info_t does not fit in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));		
											XMI_abort();
											}					
							
				}				
			
			
												  
			TRACE_ERR((stderr, ">> AdaptiveSimple::process_rts_data() Done\n"));									  
			 								  
                 
            return 0;
			
		 }
		  
		  
		   ///
          /// \brief Direct send cts_data   packet dispatch.
          ///

          static int dispatch_cts_direct (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
             TRACE_ERR((stderr, ">> AdaptiveSimple::process_cts()\n"));
			 
			 cts_info_t * rcv;
			 int to_send;
			
			if (sizeof(cts_info_t) <= T_Model::packet_model_metadata_bytes)
                {
				    TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. cts_info_t coming inside metadata\n"));
				    //Pointer to cts_info_t
			        rcv= (cts_info_t *) metadata;
							
			}else{
			       TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. cts_info_t coming inside payload\n"));
			       //Pointer to cts_info_t
			        rcv= (cts_info_t *) payload; 
						  
			      }
				  
				  
			//Determine short package
				   if((rcv->bytes % T_Model::packet_model_payload_bytes) == 0)             
				       to_send = T_Model::packet_model_payload_bytes;
				   else
					    to_send = (rcv->bytes % T_Model::packet_model_payload_bytes); 
					
			
			//Pointer to protocol			 
              AdaptiveSimple<T_Model, T_Device, T_LongHeader> * adaptive =
                (AdaptiveSimple<T_Model, T_Device, T_LongHeader> *) recv_func_parm; 
				  
						
			TRACE_ERR((stderr,"   AdaptiveSimple::process_cts() .. va_recv = %p , to_send = %d , payload_bytes = %d, next_offset= %d , total of bytes=%d\n",rcv->va_recv, to_send, T_Model::packet_model_payload_bytes, rcv->va_send->next_offset,rcv->va_send->sendlen));
				
								
				rcv->va_send->header.va_recv  = rcv->va_recv;    ///Virtual address reciver
				rcv->va_send->header.offset  = 0;                ///Initialize offset to zero
				rcv->va_send->header.bsend  = to_send;           ///Bytes to send in the package 
				rcv->va_send->next_offset = to_send;             ///Next offset value
				rcv->va_send->pf = recv_func_parm;               ///Save pointer to recv_func_param
				
			
				
			if (sizeof(header_metadata_t) <= T_Model::packet_model_metadata_bytes)
                            {
							TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. protocol header_metadata_t fits in the packet metadata\n"));
	
							           if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											
                                             TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() ..  protocol header_metadata_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));
  
											 //Post First Data package   			 
											adaptive->_data_model.postPacket   ( rcv->va_send->pkt,          ///T_message to send
																rcv->va_send->cb_data,         ///Callback to execute when done
																(void *)rcv->va_send,
																rcv->destRank,                 ///target Rank
																&rcv->va_send->header,         ///metadata
																sizeof (header_metadata_t),    ///metadata size   
																&rcv->va_send->send_buffer,    ///data to send
																to_send);                      ///Bytes to send
										 
											
											
										}else
											{
											TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() ..  protocol header_metadata_t fits in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));
											XMI_abort();
											}					
							

				
				}else{
                             TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. protocol header_metadata_t does not fit in the packet metadata\n"));

                      			if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() ..  protocol header_metadata_t does not fit in the packet metadata, xmi_task_t  fits in the message metadata\n"));
    									
											//Post Fisrt data package 			 			 			 
											adaptive->_data_model.postPacket   ( rcv->va_send->pkt,          ///T_message to send
																rcv->va_send->cb_data,         ///Callback to execute when done
																(void *)rcv->va_send,
																rcv->destRank,                 ///target Rank
																(void *)NULL,
																0,
																&rcv->va_send->header,         ///metadata
																sizeof (header_metadata_t),    ///metadata size   
																&rcv->va_send->send_buffer,    ///data to send
																to_send);     
											
											
										}else
											{
											TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() ..  protocol header_metadata_t does not fit in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));		
											XMI_abort();
											}					
							
				}		
				
				
				
						
				
				
				//Remove Head from queue
		        adaptive->_queue[adaptive->_fromRank].popHead();
				
		        //Check queue is not empty
		       	if(!adaptive->_queue[adaptive->_fromRank].isEmpty()){	
                    
			
				 //recover next object to send			
			     send_state_t * send = (send_state_t *) (adaptive->_queue[adaptive->_fromRank].peekHead()); 
			
						     				 
				 
				if (send->sendlen == 0){
				 
				 
				      // This branch should be resolved at compile time and optimized out.
                      if (sizeof(rts_info_t) <= T_Model::packet_model_metadata_bytes)
                            {
							 TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. zero-byte data special case, protocol rts_info_t fits in the packet metadata\n"));

				 
								if (send->rts.count > T_Model::packet_model_payload_bytes)
								    {
									
									TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. zero-byte data special case, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload\n"));

						                 
										if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											
											  TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. zero-byte data special case, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does fit in the message metadata\n"));
                                                                  											   
                                                //send RTS info without message info
										        adaptive->_rts_model.postPacket (send->pkt,         ///T_Message to send
																NULL,                     ///Callback to execute when done 
																(void *)rcv->va_send,   /// Cookie
																send->rts.destRank,       ///Target Rank
																(void *)&send->rts,       ///rts_info_t struct  metadata
																sizeof(rts_info_t),       ///sizeof rts_info_t  metadata
																(void *)NULL,             ///Message info
																0);  				      ///Number of bytes
											
											   
										}else
											{
											TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. zero-byte data special case, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does not fit in the message metadata\n"));
                     						XMI_abort();
											}								
									}				
									else
									{
									
									TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. zero-byte data special case, protocol rts_info_t fits in the packet metadata, application metadata fit in a single packet payload\n"));

						                
										//Everything OK                               ///Post rts package
								        adaptive->_rts_model.postPacket (send->pkt,             ///T_Message to send
														send->local_fn,               ///Callback to execute when done 
														(void *)send->cookie,       /// Cookie
														send->rts.destRank,          ///Target Rank
														(void *)&send->rts,          ///rts_info_t struct metadata
														sizeof(rts_info_t),          ///sizeof rts_info_t metadata
														send->msginfo,               ///Message info
														send->rts.count);            ///Number of bytes
					
									    //End Sender side, 0 bytes
										if (send->remote_fn)
											send->remote_fn (adaptive->_context, send->cookie, XMI_SUCCESS);
					
										if (send->remote_fn == NULL)
											{
											adaptive->freeSendState (send);
											} 
						
						
									}
				            }else{ //sizeof(rts_info_t) > T_Model::packet_model_metadata_bytes
							
							    TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. zero-byte data special case, protocol rts_info_t does not fit in the packet metadata\n"));

				 
								  if (send->rts.count > (T_Model::packet_model_payload_bytes - sizeof(rts_info_t)))
									 {
					                     TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. zero-byte data special case, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload\n"));

							
										if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{
											 
											 TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. zero-byte data special case, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does fit in the message metadata\n"));
                                               
																					
                                                adaptive->_rts_model.postPacket (send->pkt,         ///T_Message to send
																NULL,                     ///Callback to execute when done 
																(void *)rcv->va_send,   /// Cookie
																send->rts.destRank,       ///Target Rank
																(void *)NULL,             ///Metadata 
																0,                        ///Metadata size
																(void *)&send->rts,       ///rts_info_t struct
																sizeof(rts_info_t),       ///sizeof rts_info_t
																(void *)NULL,             ///Message info
																0);  		              ///Number of bytes
																							  
											
											
											
										}else{
										     TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. zero-byte data special case, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does not fit in the message metadata\n"));
											XMI_abort();
										}
					                 }
									 else{
						                  TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. zero-byte data special case, protocol rts_info_t does not fit in the packet metadata, application metadata fits in a single packet payload\n"));
    
											  //OK CASE
						                     adaptive->_rts_model.postPacket (send->pkt,       ///T_Message to send
														send->local_fn,              ///Callback to execute when done 
														(void *)send->cookie,       /// Cookie
														 send->rts.destRank,         ///Target Rank
														(void *)NULL,                ///Metadata 
														0,                           ///Metadata size
														(void *)&send->rts,          ///rts_info_t struct
														sizeof(rts_info_t),          ///sizeof rts_info_t
														send->msginfo,               ///Message info
														send->rts.count);  	         ///Number of bytes
					
					                    //End Sender side, 0 bytes
									    if (send->remote_fn)
											send->remote_fn (adaptive->_context, send->cookie, XMI_SUCCESS);
					
										if (send->remote_fn == NULL)
											{
											adaptive->freeSendState (send);
											} 
						 
									}
							}
			    }else{
				
				        if (sizeof(rts_info_t) <= T_Model::packet_model_metadata_bytes)
                            {		
                                 TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. with data, protocol rts_info_t fits in the packet metadata\n"));
							
						   
				               if (send->rts.count > T_Model::packet_model_payload_bytes)
                                  {
								  
								  TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. with data, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload\n"));
							  
				                     if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
                                        {
										
										TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does fit in the message metadata\n"));
  	    
										        adaptive->_rts_model.postPacket (send->pkt,           ///T_Message to send
																NULL,                       ///Callback to execute when done 
																(void *) rcv->va_send,      /// Cookie
																send->rts.destRank,         ///Target Rank
																(void *)&send->rts,         ///rts_info_t struct metadata
																sizeof(rts_info_t),         ///sizeof rts_info_t metadata
																(void *)NULL,               ///Message info
																0);  				        ///Number of bytes
										
										
										
						             }else{
									     TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. with data, protocol rts_info_t fits in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does not fit in the message metadata\n"));
           				                 XMI_abort();
						                  }
					              }else
					                  {
									   TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. zero-byte data special case, protocol rts_info_t fits in the packet metadata, application metadata fit in a single packet payload\n"));

									   //Everything OK
									    adaptive->_rts_model.postPacket (send->pkt,
																NULL,
																(void *) send->cookie ,
																send->rts.destRank,
																(void *)&send->rts,
																sizeof(rts_info_t),
																send->msginfo,
																send->rts.count);
									  
					   
					              }
					        }else
					            {
								
								TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. with data, protocol rts_info_t does not fit in the packet metadata\n"));

					            if (send->rts.count > (T_Model::packet_model_payload_bytes - sizeof(rts_info_t)))
                                     {
									     			
                                       TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. with data, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload\n"));
  				 
                    				 if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
                                             {
											 TRACE_ERR((stderr, "AdaptiveSimple::simple_impl() .. with data, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does fit in the message metadata\n"));
              
										        adaptive->_rts_model.postPacket (send->pkt,         ///T_Message to send
																NULL,                     ///Callback to execute when done 
																(void *) rcv->va_send,    ///Cookie
																send->rts.destRank,       ///Target Rank
																(void *)NULL,             ///Metadata 
																0,                        ///Metadata size
																(void *)&send->rts,       ///rts_info_t struct
																sizeof(rts_info_t)        ///sizeof rts_info_t
																);  	
									 
										
											 
											 
											 
								         }else{
										        TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. with data, protocol rts_info_t does not fit in the packet metadata, application metadata does not fit in a single packet payload, xmi_task_t does not fit in the message metadata\n"));
		 					                  XMI_abort();
									           }
								     }else
								          {
										  
										  TRACE_ERR((stderr, "   AdaptiveSimple::process_cts() .. with data, protocol rts_info_t does not fit in the packet metadata, application metadata fits in a single packet payload\n"));
    	
										     adaptive->_rts_model.postPacket (send->pkt,
																NULL,
																(void *) send->cookie ,
																send->rts.destRank,
																(void *)NULL,
																0,
																(void *)&send->rts,
																sizeof(rts_info_t),
																send->msginfo,
																send->rts.count);
								   
								          }
								}
								   
						  
					}
            }					
			
			 TRACE_ERR((stderr, ">> AdaptiveSimple::process_cts()\n"));
	
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
			TRACE_ERR((stderr, ">> AdaptiveSimple::process_data()\n"));
           
		   header_metadata_t * header;
		   void * msginfo;
			
			 if (sizeof(header_metadata_t) <= T_Model::packet_model_metadata_bytes)
                  {
				TRACE_ERR((stderr, "   AdaptiveSimple::process_data() .. header_metadata_t coming inside metadata\n"));
				    
				 //Pointer to metadata
				  header = (header_metadata_t *) metadata;
				  
				  //copy data to buffer
				  memcpy ((char *)(header->va_recv->info.data.simple.addr) + header->offset, payload, header->bsend);      
			   
		        }  
			   else{
			   TRACE_ERR((stderr, "   AdaptiveSimple::process_data() .. header_metadata_t coming inside payload\n"));
				  
			      //Pointer to metadata
				  header = (header_metadata_t *) payload;
				  
				  //copy data to buffer
				  memcpy ((char *)(header->va_recv->info.data.simple.addr) + header->offset, (header+1), header->bsend);      				 
			  		

                    }			

				    
   
                //Update total of bytes received
                header->va_recv->info.data.simple.bytes -= header->bsend;
			 
			    TRACE_ERR((stderr,"   AdaptiveSimple::process_data() .. buffer address =%p ,  bytes for receiving =%d, offset = %d, bsend = %d\n", header->va_recv->info.data.simple.addr, header->va_recv->info.data.simple.bytes , header->offset, header->bsend));
			 
				//Terminate after receiving all the bytes		 			  
			    if (header->va_recv->info.data.simple.bytes == 0)		  
			      {
				     TRACE_ERR((stderr,"   AdaptiveSimple::process_data() ..  all data Received \n")); 
				   if (header->va_recv->info.local_fn) 
				        header->va_recv->info.local_fn (header->va_recv->adaptive->_context,header->va_recv->info.cookie, XMI_SUCCESS);   	     
				
			  			         
                      return 0;
                  }
				  
		         TRACE_ERR((stderr, ">>   AdaptiveSimple::process_data() ..  data package Received\n"));
				  
	      	 return 0;
          };

		  
		  ///
          /// \brief Callback invoked after send a rts_data packet.
          ///
		  
		  static void cb_rts_send (xmi_context_t   context,
                                     void          * cookie,
                                     xmi_result_t    result)
          {
		  
		   TRACE_ERR((stderr, ">> AdaptiveSimple::Callback_rts_send()\n"));
		   
		 //Fix Bug Share Memory Device
		 //usleep(200); 
		   
            //Pointer to send state
			send_state_t * send = (send_state_t *) cookie;
			
			
            //Pointer to Protocol object
            AdaptiveSimple<T_Model, T_Device, T_LongHeader> * adaptive=
              (AdaptiveSimple<T_Model, T_Device, T_LongHeader> *) send->adaptive;
				

            	 				  
				//Define bytes to send
				if(sizeof(header_metadata_t) <= T_Model::packet_model_metadata_bytes){	
				
					     send->header.bsend = T_Model::packet_model_payload_bytes;				 
				}else{
					     send->header.bsend = (T_Model::packet_model_payload_bytes - sizeof(header_metadata_t));
				    				
				}				
				
				 	
				
				 //Check if data to send
                 if (send->rts.count <= send->next_offset )
                    {
					 
					  TRACE_ERR((stderr, ">>   AdaptiveSimple::Callback_rts_send() .. all rts_data was sent\n"));
							
					if(send->sendlen ==0){
					   
					          TRACE_ERR((stderr, ">>   AdaptiveSimple::Callback_rts_send() .. Done, 0 bytes to send\n"));
							
						  	
					           if (send->local_fn != NULL)
                                 {
                                 send->local_fn (adaptive->_context, send->cookie, XMI_SUCCESS);
                                 }
							
						      if (send->remote_fn)
											send->remote_fn (adaptive->_context, send->cookie, XMI_SUCCESS);	

                              if (send->remote_fn == NULL)
                                 {
                                  adaptive->freeSendState (send);
                                 }
					
					     }
						 

					 return ;
					
				  }
				  	       
                
                 TRACE_ERR((stderr,"   AdaptiveSimple::Callback_rts_send() .. Metadata bytes= %d  , send->next_offset = %d , offset=%d , bsend=%d \n", send->rts.count, send->next_offset, send->header.offset ,send->header.bsend));
				  			 				
				  send->header.offset= send->next_offset; 
			      send->next_offset += send->header.bsend; 
				

                  //TRACE_ERR((stderr,"   AdaptiveSimple::Callback_rts_send() .. cookie =%p , destination rank =%d, header=%p , offset = %d, msginfo =%p , send =%d \n",cookie , send->rts.destRank, &send->header, send->header.offset, send->msginfo, send->header.bsend));
                  
				  if (sizeof(header_metadata_t) <= T_Model::packet_model_metadata_bytes)
                            {
							TRACE_ERR((stderr, "   AdaptiveSimple::Callback_rts_send() .. protocol header_metadata_t fits in the packet metadata\n"));

							           if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											TRACE_ERR((stderr, "   AdaptiveSimple::Callback_rts_send() ..  protocol header_metadata_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));

																					
											    //Post rts_data package 			 
												adaptive->_rts_data_model.postPacket (send->pkt,
															send->cb_rts,
															(void *) send,
															send->rts.destRank,      
															(void *) &send->header,
															sizeof (header_metadata_t),
															(void *)((char *)send->msginfo + send->header.offset),
															(size_t)send->header.bsend);
										 
											
											
										}else
											{
											 TRACE_ERR((stderr, "   AdaptiveSimple::Callback_rts_send() ..  protocol header_metadata_t fits in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));
   										  	 XMI_abort();
											}					
							

				
			        }else{
                                TRACE_ERR((stderr, "   AdaptiveSimple::Callback_rts_send .. protocol header_metadata_t does not fit in the packet metadata\n"));
	
                      			if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											TRACE_ERR((stderr, "   AdaptiveSimple::Callback_rts_send ..  protocol header_metadata_t does not fit in the packet metadata, xmi_task_t  fits in the message metadata\n"));
     										
											    //Post rts_data package 			 
												adaptive->_rts_data_model.postPacket (send->pkt,
															send->cb_rts,
															(void *) send ,
															send->rts.destRank,      
															(void *) NULL,
															0,
															(void *) &send->header,
															sizeof (header_metadata_t),
															(void *)((char *)send->msginfo + send->header.offset),
															(size_t)send->header.bsend);
										 
											
											
										}else
											{
											TRACE_ERR((stderr, "   AdaptiveSimple::Callback_rts_send ..  protocol header_metadata_t does not fit in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));		
											XMI_abort();
											}					
							
				}						 
										 
										 
			TRACE_ERR((stderr, ">>   AdaptiveSimple::Callback_rts_send() .. posted rts_data packet\n"));
							
            return;
          }

		  
		  
		  
		  
		  
		  ///
          /// \brief Callback invoked after send a data_send packet.
          ///

          static void cb_data_send (xmi_context_t   context,
                                     void          * cookie,
                                     xmi_result_t    result)
          {
		  
		   TRACE_ERR((stderr, ">> AdaptiveSimple::Callback_data_send()\n"));
		   
		    
		   
		  			   
		    //Pointer to send state
            send_state_t * send = (send_state_t *) cookie;
			
			
            //Pointer to Protocol 
            AdaptiveSimple<T_Model, T_Device, T_LongHeader> * adaptive=
              (AdaptiveSimple<T_Model, T_Device, T_LongHeader> *) send->pf;
			  
			

            //Define bytes to send and next_offset
		  	if (sizeof(header_metadata_t) <= T_Model::packet_model_metadata_bytes){			
				 send->next_offset += T_Model::packet_model_payload_bytes;                 ///Update next Offset
				 send->header.bsend= T_Model::packet_model_payload_bytes;                 ///Update bytes to send
				 
			}else{
			     send->next_offset += (T_Model::packet_model_payload_bytes-sizeof(header_metadata_t));                 ///Update next Offset
				 send->header.bsend= (T_Model::packet_model_payload_bytes-sizeof(header_metadata_t));                 ///Update bytes to send
			 }  
				
				
				
			TRACE_ERR((stderr,"   AdaptiveSimple::Callback_data_send() .. data total bytes= %d  , send->next_offset = %d , offset=%d , bsend=%d \n", send->sendlen, send->next_offset,send->header.offset,send->header.bsend));
			//TRACE_ERR((stderr,"   AdaptiveSimple::Callback_data_send() .. cookie= %p , send->cts->va_send = %p ,send->cb_data = %p\n" ,cookie,send->rts.va_send, send->cb_data));
				
				 //Check if data to send
                 if (send->sendlen < send->next_offset)
                    {
					
					TRACE_ERR((stderr, ">>   AdaptiveSimple::Callback_data_send() .. all data was sent\n"));
						
					if (send->local_fn != NULL)
                          {
                             send->local_fn (adaptive->_context, send->cookie, XMI_SUCCESS);
                            }
							
					if (send->remote_fn != NULL)
                          {
                             send->remote_fn (adaptive->_context, send->cookie, XMI_SUCCESS);
                            }		

                   if (send->remote_fn == NULL)
                      {
                          adaptive->freeSendState (send);
                        }
					
	
					 return ;
				    }
				  
				  
                 send->header.offset= send->next_offset;                                   ///Update offset in metadata
				
                
				 if (sizeof(header_metadata_t) <= T_Model::packet_model_metadata_bytes)
                            {
							TRACE_ERR((stderr, "   AdaptiveSimple::Callback_data_send() .. protocol header_metadata_t fits in the packet metadata\n"));

							           if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											
											TRACE_ERR((stderr, "   AdaptiveSimple::Callback_data_send() ..  protocol header_metadata_t fits in the packet metadata, xmi_task_t  fits in the message metadata\n"));
										    			 
												//Post data package 	
											
												adaptive->_data_model.postPacket (send->pkt,
																			send->cb_data,
																			(void *)send,
																			send->rts.destRank,      
																			(void *) &send->header,
																			sizeof (header_metadata_t),
																			(void *)(send->send_buffer + send->header.offset),
																			(size_t)send->header.bsend);
										 
											
											
										}else
											{
											TRACE_ERR((stderr, "   AdaptiveSimple::Callback_data_send() ..  protocol header_metadata_t fits in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));
   										  	XMI_abort();
											}					
							

				
				}else{
				       TRACE_ERR((stderr, "   AdaptiveSimple::Callback_data_send .. protocol header_metadata_t does not fit in the packet metadata\n"));
	
                      			if (sizeof(xmi_task_t) <= T_Model::message_model_metadata_bytes)
											{	
											TRACE_ERR((stderr, "   AdaptiveSimple::Callback_data_send ..  protocol header_metadata_t does not fit in the packet metadata, xmi_task_t  fits in the message metadata\n"));
     									
											    //Post data package 			 			 
												adaptive->_data_model.postPacket (send->pkt,
																			send->cb_data,
																			(void *)send,
																			send->rts.destRank,      // target rank
																			(void *)NULL,
																			 0,
																			(void *) &send->header,
																			sizeof (header_metadata_t),
																			(void *)(send->send_buffer + send->header.offset),
																			(size_t)send->header.bsend);
										 
											
											
										}else
											{
											TRACE_ERR((stderr, "   AdaptiveSimple::Callback_data_send ..  protocol header_metadata_t does not fit in the packet metadata, xmi_task_t  does not fit in the message metadata\n"));		
											XMI_abort();
											}					
							
				}						 
					 				

			TRACE_ERR((stderr, ">>   AdaptiveSimple::Callback_data_send() .. posted data packet\n"));		
      
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
            TRACE_ERR((stderr, "(%zd) AdaptiveFactory::dispatch_rts_read() .. \n"));

            AdaptiveFactory<T_Model, T_Device, T_LongHeader> * pf =
              (AdaptiveFactory<T_Model, T_Device, T_LongHeader> *) recv_func_parm;

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
            TRACE_ERR((stderr, "(%zd) AdaptiveFactory::dispatch_rts_ack_read() .. \n"));

            AdaptiveFactory<T_Model, T_Device, T_LongHeader> * pf =
              (AdaptiveFactory<T_Model, T_Device, T_LongHeader> *) recv_func_parm;

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
            TRACE_ERR((stderr, "(%zd) AdaptiveFactory::dispatch_rts_data_read() .. \n"));

            AdaptiveFactory<T_Model, T_Device, T_LongHeader> * pf =
              (AdaptiveFactory<T_Model, T_Device, T_LongHeader> *) recv_func_parm;

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
            TRACE_ERR((stderr, "AdaptiveFactory::dispatch_data_read() .. \n"));

            AdaptiveFactory<T_Model, T_Device, T_LongHeader> * pf =
              (AdaptiveFactory<T_Model, T_Device, T_LongHeader> *) recv_func_parm;

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
          /// \brief Read-access  send cts packet callback.
          ///
		  		  
		   static int dispatch_cts_read (void         * metadata,
                                          void         * payload,
                                          size_t         bytes,
                                          void         * recv_func_parm,
										  void         * cookie)
          {
#if 0
           TRACE_ERR((stderr, "AdaptiveFactory::dispatch_cts_read() .. \n"));
            send_state_t * state = (send_state_t *) metadata;

            xmi_event_function * local_fn  = state->local_fn;
            xmi_event_function * remote_fn = state->remote_fn;
            void               * cookie    = state->cookie;

            Adaptive<T_Model, T_Device, T_LongHeader> * adaptive =
              (Adaptive<T_Model, T_Device, T_LongHeader> *) recv_func_parm;
            adaptive->freeSendState (state);

            if (local_fn)  local_fn  (0, cookie, XMI_SUCCESS);

            if (remote_fn) remote_fn (0, cookie, XMI_SUCCESS);

#endif
            return 0;
          }
		  
	
      };
	  template <class T_Model, class T_Device, bool T_LongHeader >
	  Queue * AdaptiveSimple< T_Model,  T_Device, T_LongHeader>::_queue = NULL;
    };
  };
};
#undef TRACE_ERR
#endif // __xmi_p2p_protocol_send_adaptive_adaptivesimple_h__

//
// astyle info    http://astyle.sourceforge.net
//
// astyle options --style=gnu --indent=spaces=2 --indent-classes
// astyle options --indent-switches --indent-namespaces --break-blocks
// astyle options --pad-oper --keep-one-line-blocks --max-instatement-indent=79
//
