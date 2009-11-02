/**********************************************************************/
/**  Deprecated Multicast:  To be deleted soon!!! */
/**********************************************************************/
typedef enum
{
  XMI_UNDEFINED_CONSISTENCY = -1,
  XMI_RELAXED_CONSISTENCY,
  XMI_MATCH_CONSISTENCY,
  XMI_WEAK_CONSISTENCY,
  XMI_CONSISTENCY_COUNT
} xmi_consistency_t;

#define  LINE_BCAST_MASK    (XMI_LINE_BCAST_XP|XMI_LINE_BCAST_XM|   \
                             XMI_LINE_BCAST_YP|XMI_LINE_BCAST_YM|   \
                             XMI_LINE_BCAST_ZP|XMI_LINE_BCAST_ZM)
typedef enum
{
  XMI_PT_TO_PT_SUBTASK           =  0,      /**< Send a pt-to-point message */
  XMI_LINE_BCAST_XP              =  0x20,   /**< Bcast along x+ */
  XMI_LINE_BCAST_XM              =  0x10,   /**< Bcast along x- */
  XMI_LINE_BCAST_YP              =  0x08,   /**< Bcast along y+ */
  XMI_LINE_BCAST_YM              =  0x04,   /**< Bcast along y- */
  XMI_LINE_BCAST_ZP              =  0x02,   /**< Bcast along z+ */
  XMI_LINE_BCAST_ZM              =  0x01,   /**< Bcast along z- */
  XMI_COMBINE_SUBTASK            =  0x0100, /**< Combine the incoming message */
  /* with the local state */
  XMI_GI_BARRIER                 =  0x0200,
  XMI_LOCKBOX_BARRIER            =  0x0300,
  XMI_TREE_BARRIER               =  0x0400,
  XMI_TREE_BCAST                 =  0x0500,
  XMI_TREE_ALLREDUCE             =  0x0600,
  XMI_REDUCE_RECV_STORE          =  0x0700,
  XMI_REDUCE_RECV_NOSTORE        =  0x0800,
  XMI_BCAST_RECV_STORE           =  0x0900,
  XMI_BCAST_RECV_NOSTORE         =  0x0a00,
  XMI_LOCALCOPY                  =  0x0b00,
  XMI_UNDEFINED_SUBTASK          =  (~LINE_BCAST_MASK),
} xmi_subtask_t;


typedef xmi_quad_t * (*xmi_olddispatch_multicast_fn) (const xmi_quad_t   * info,
                                                      unsigned             count,
                                                      unsigned             peer,
                                                      unsigned             sndlen,
                                                      unsigned             connection_id,
                                                      void               * arg,
                                                      unsigned           * rcvlen,
                                                      char              ** rcvbuf,
                                                      unsigned           * pipewidth,
                                                      xmi_callback_t     * cb_done);
typedef struct
{
  xmi_quad_t        * request;
  xmi_callback_t      cb_done;
  unsigned            connection_id;
  unsigned            bytes;
  const char        * src;
  unsigned            ntasks;
  unsigned          * tasks;
  xmi_subtask_t     * opcodes;
  const xmi_quad_t  * msginfo;
  unsigned            count;
  unsigned            flags;
  xmi_op              op;
  xmi_dt              dt;
} xmi_oldmulticast_t;

typedef struct
{
  xmi_quad_t        * request;
  xmi_callback_t      cb_done;
  unsigned            connection_id;
  unsigned            bytes;
  char              * rcvbuf;
  unsigned            pipelineWidth;
  xmi_subtask_t       opcode;
  xmi_op              op;
  xmi_dt              dt;
} xmi_oldmulticast_recv_t;


typedef xmi_quad_t * (*xmi_olddispatch_manytomany_fn) (unsigned         connection_id,
                                                       void           * arg,
                                                       char          ** rcvbuf,
                                                       size_t        ** rcvdispls,
                                                       size_t        ** rcvlens,
                                                       size_t        ** rcvcounters,
                                                       size_t         * ntasks,
                                                       xmi_callback_t * cb_done);





