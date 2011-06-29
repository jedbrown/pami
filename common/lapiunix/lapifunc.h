/**
 * \file common/lapiunix/lapifunc.h
 * \brief Encapsulation for LAPI function pointers.
 */
/****************************************************************************
 Description: Encapsulation for LAPI function pointers.

 Author: Nicole Nie

 History:
   Date     Who ID    Description
   -------- --- ---   -----------
   07/14/09 nieyy     Initial code
****************************************************************************/

#ifndef __common_lapiunix_lapifunc_h__
#define __common_lapiunix_lapifunc_h__

#include <stdlib.h>
#include "lapi.h"
#include "lapi_hfi.h"
#include "common/lapiunix/lapierror.h"

// Define this to turn on dlopened lapi functions
// #ifdef LAPI_DLOPEN


class LapiFunc
{
    public:
        typedef int (lapi_addr_get_hndlr)(lapi_handle_t hndl, void **addr, int addr_hndl);
        typedef int (lapi_addr_set_hndlr)(lapi_handle_t hndl, void *addr, int addr_hndl);
        typedef int (lapi_address_hndlr)(void *my_addr, ulong *ret_addr);
        typedef int (lapi_address_init_hndlr)(lapi_handle_t hndl, void *my_tab, void *add_tab[]);
        typedef int (lapi_address_init64_hndlr)(lapi_handle_t hndl, lapi_long_t my_tab, lapi_long_t *add_tab);

        typedef int (lapi_amsend_hndlr)(lapi_handle_t hndl, uint tgt, void *hdr_hdl, void *uhdr, uint uhdr_len,
                void *udata, ulong udata_len, lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr);
        typedef int (lapi_amsendv_hndlr)(lapi_handle_t hndl, uint tgt, void *hdr_hdl, void *uhdr,uint uhdr_len,
                lapi_vec_t *org_vec, lapi_cntr_t  *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr);
        typedef int (lapi_fence_hndlr)(lapi_handle_t hndl);
        typedef int (lapi_get_hndlr)(lapi_handle_t hndl, uint tgt, ulong len, void *tgt_addr, void *org_addr,
                lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr);
        typedef int (lapi_getcntr_hndlr)(lapi_handle_t hndl, lapi_cntr_t *cntr, int *val);
        typedef int (lapi_getv_hndlr)(lapi_handle_t hndl, uint tgt, lapi_vec_t *tgt_vec, lapi_vec_t *org_vec,
                lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr);
        typedef int (lapi_gfence_hndlr)(lapi_handle_t hndl);
        typedef int (lapi_init_hndlr)(lapi_handle_t *hndl, lapi_info_t *lapi_info);
        typedef int (lapi_msg_string_hndlr)(int error_code, void *buf);
        typedef int (lapi_msgpoll_hndlr)(lapi_handle_t hndl, uint cnt, lapi_msg_info_t *info);
        typedef int (lapi_probe_hndlr)(lapi_handle_t hndl);

        typedef int (lapi_put_hndlr)(lapi_handle_t hndl, uint tgt, ulong len, void *tgt_addr, void *org_addr,
                lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr);
        typedef int (lapi_putv_hndlr)(lapi_handle_t hndl, uint tgt, lapi_vec_t *tgt_vec, lapi_vec_t *org_addr,
                lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr);
        typedef int (lapi_qenv_hndlr)(lapi_handle_t hndl, lapi_query_t query, int *ret_val);
        typedef int (lapi_rmw_hndlr)(lapi_handle_t hndl, RMW_ops_t op, uint tgt, int *tgt_var, int *in_val,
                int *prev_tgt_val, lapi_cntr_t *org_cntr);
        typedef int (lapi_rmw64_hndlr)(lapi_handle_t hndl, RMW_ops_t op, uint tgt, long long *tgt_var,
                long long *in_val, long long *prev_tgt_val, lapi_cntr_t *org_cntr);
        typedef int (lapi_senv_hndlr)(lapi_handle_t hndl, lapi_query_t query, int set_val);
        typedef int (lapi_setcntr_hndlr)(lapi_handle_t hndl, lapi_cntr_t *cntr, int val);
        typedef int (lapi_term_hndlr)(lapi_handle_t hndl);

        typedef int (lapi_util_hndlr)(lapi_handle_t hndl, lapi_util_t *util_p);
        typedef int (lapi_waitcntr_hndlr)(lapi_handle_t hndl, lapi_cntr_t *cntr, int val, int *cur_cntr_val);
        typedef int (lapi_xfer_hndlr)(lapi_handle_t hndl, lapi_xfer_t *xfer_cmd);

        typedef int (lapi_nopoll_wait_hndlr)(lapi_handle_t hndl, lapi_cntr_t *cntr_ptr, int val, int *cur_cntr_val);
        typedef int (lapi_purge_totask_hndlr)(lapi_handle_t hndl, css_task_t dest);
        typedef int (lapi_resume_totask_hndlr)(lapi_handle_t hndl, css_task_t dest);
        typedef int (lapi_setcntr_wstatus_hndlr)(lapi_handle_t hndl, lapi_cntr_t *cntr, int val, uint *dest, int *dest_status);

        typedef int (lapi_cau_group_create_hndlr)(lapi_handle_t hndl, uint group_id,uint num_tasks, uint *task_list);
        typedef int (lapi_cau_group_destroy_hndlr)(lapi_handle_t hndl, uint group_id);
        typedef int (lapi_cau_multicast_hndlr)(lapi_handle_t hndl, uint group,
                                               int hdr_hdl, void *hdr, uint hdr_len, void *data, ulong data_len,
                                               compl_hndlr_t done, void *cookie);
        typedef int (lapi_cau_reduce_hndlr)(lapi_handle_t hndl, uint group,
                                            int hdr_hdl, void *hdr, uint hdr_len, void *data, ulong data_len,
                                            cau_reduce_op_t op, compl_hndlr_t done, void *cookie);
      private:
        void *dlopen_file;

        lapi_addr_get_hndlr *addr_get_hndlr;
        lapi_addr_set_hndlr *addr_set_hndlr;
        lapi_address_hndlr *address_hndlr;
        lapi_address_init_hndlr *address_init_hndlr;
        lapi_address_init64_hndlr *address_init64_hndlr;

        lapi_amsend_hndlr *amsend_hndlr;
        lapi_amsendv_hndlr *amsendv_hndlr;
        lapi_fence_hndlr *fence_hndlr;
        lapi_get_hndlr *get_hndlr;
        lapi_getcntr_hndlr *getcntr_hndlr;
        lapi_getv_hndlr *getv_hndlr;
        lapi_gfence_hndlr *gfence_hndlr;
        lapi_init_hndlr *init_hndlr;
        lapi_msg_string_hndlr *msg_string_hndlr;
        lapi_msgpoll_hndlr *msgpoll_hndlr;
        lapi_probe_hndlr *probe_hndlr;

        lapi_put_hndlr *put_hndlr;
        lapi_putv_hndlr *putv_hndlr;
        lapi_qenv_hndlr *qenv_hndlr;
        lapi_rmw_hndlr *rmw_hndlr;
        lapi_rmw64_hndlr *rmw64_hndlr;
        lapi_senv_hndlr *senv_hndlr;
        lapi_setcntr_hndlr *setcntr_hndlr;
        lapi_term_hndlr *term_hndlr;

        lapi_util_hndlr *util_hndlr;
        lapi_waitcntr_hndlr *waitcntr_hndlr;
        lapi_xfer_hndlr *xfer_hndlr;

        lapi_nopoll_wait_hndlr *nopoll_wait_hndlr;
        lapi_purge_totask_hndlr *purge_totask_hndlr;
        lapi_resume_totask_hndlr *resume_totask_hndlr;
        lapi_setcntr_wstatus_hndlr *setcntr_wstatus_hndlr;

        lapi_cau_group_create_hndlr *cau_group_create_hndlr;
        lapi_cau_group_destroy_hndlr *cau_group_destroy_hndlr;
        lapi_cau_multicast_hndlr *cau_multicast_hndlr;
        lapi_cau_reduce_hndlr *cau_reduce_hndlr;

    private:
        LapiFunc();
        static LapiFunc *instance;

    public:
        ~LapiFunc();
        static LapiFunc *getInstance();
        int load();

        int Addr_get(lapi_handle_t hndl, void **addr, int addr_hndl);
        int Addr_set(lapi_handle_t hndl, void *addr, int addr_hndl);
        int Address(void *my_addr, ulong *ret_addr);
        int Address_init(lapi_handle_t hndl, void *my_tab, void *add_tab[]);
        int Address_init64(lapi_handle_t hndl, lapi_long_t my_tab, lapi_long_t *add_tab);
        int Amsend(lapi_handle_t hndl, uint tgt, void *hdr_hdl, void *uhdr, uint uhdr_len,
                void *udata, ulong udata_len, lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr);
        int Amsendv(lapi_handle_t hndl, uint tgt, void *hdr_hdl, void *uhdr,uint uhdr_len,
                lapi_vec_t *org_vec, lapi_cntr_t  *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr);
        int Fence(lapi_handle_t hndl);
        int Get(lapi_handle_t hndl, uint tgt, ulong len, void *tgt_addr, void *org_addr,
                lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr);
        int Getcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int *val);
        int Getv(lapi_handle_t hndl, uint tgt, lapi_vec_t *tgt_vec, lapi_vec_t *org_vec,
                lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr);
        int Gfence(lapi_handle_t hndl);
        int Init(lapi_handle_t *hndl, lapi_info_t *lapi_info);
        int Msg_string(int error_code, void *buf);
        int Msgpoll(lapi_handle_t hndl, uint cnt, lapi_msg_info_t *info);
        int Probe(lapi_handle_t hndl);
        int Put(lapi_handle_t hndl, uint tgt, ulong len, void *tgt_addr, void *org_addr,
                lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr);
        int Putv(lapi_handle_t hndl, uint tgt, lapi_vec_t *tgt_vec, lapi_vec_t *org_addr,
                lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr);
        int Qenv(lapi_handle_t hndl, lapi_query_t query, int *ret_val);
        int Rmw(lapi_handle_t hndl, RMW_ops_t op, uint tgt, int *tgt_var, int *in_val,
                int *prev_tgt_val, lapi_cntr_t *org_cntr);
        int Rmw64(lapi_handle_t hndl, RMW_ops_t op, uint tgt, long long *tgt_var,
                long long *in_val, long long *prev_tgt_val, lapi_cntr_t *org_cntr);
        int Senv(lapi_handle_t hndl, lapi_query_t query, int set_val);
        int Setcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int val);
        int Term(lapi_handle_t hndl);
        int Util(lapi_handle_t hndl, lapi_util_t *util_p);
        int Waitcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int val, int *cur_cntr_val);
        int Xfer(lapi_handle_t hndl, lapi_xfer_t *xfer_cmd);
        int Nopoll_wait(lapi_handle_t hndl, lapi_cntr_t *cntr_ptr, int val, int *cur_cntr_val);
        int Purge_totask(lapi_handle_t hndl, css_task_t dest);
        int Resume_totask(lapi_handle_t hndl, css_task_t dest);
        int Setcntr_wstatus(lapi_handle_t hndl, lapi_cntr_t *cntr, int val, uint *dest, int *dest_status);

        int (Cau_group_create)(lapi_handle_t hndl, uint group_id,uint num_tasks, uint *task_list);
        int (Cau_group_destroy)(lapi_handle_t hndl, uint group_id);
        int (Cau_multicast)(lapi_handle_t hndl, uint group,
                            int hdr_hdl, void *hdr, uint hdr_len, void *data, ulong data_len,
                            compl_hndlr_t done, void *cookie);
        int (Cau_reduce)(lapi_handle_t hndl, uint group,
                         int hdr_hdl, void *hdr, uint hdr_len, void *data, ulong data_len,
                         cau_reduce_op_t op, compl_hndlr_t done, void *cookie);
      private:
        void * import(const char *funcname);
};

#ifdef LAPI_DLOPEN

#define lapi_addr_get LapiFunc::getInstance()->Addr_get
#define lapi_addr_set LapiFunc::getInstance()->Addr_set
#define lapi_address LapiFunc::getInstance()->Address
#define lapi_address_init LapiFunc::getInstance()->Address_init
#define lapi_address_init64 LapiFunc::getInstance()->Address_init64

#define lapi_amsend LapiFunc::getInstance()->Amsend
#define lapi_amsendv LapiFunc::getInstance()->Amsendv
#define lapi_fence LapiFunc::getInstance()->Fence
#define lapi_get LapiFunc::getInstance()->Get
#define lapi_getcntr LapiFunc::getInstance()->Getcntr
#define lapi_getv LapiFunc::getInstance()->Getv
#define lapi_gfence LapiFunc::getInstance()->Gfence
#define lapi_init LapiFunc::getInstance()->Init
#define lapi_msg_string LapiFunc::getInstance()->Msg_string
#define lapi_msgpoll LapiFunc::getInstance()->Msgpoll
#define lapi_probe LapiFunc::getInstance()->Probe

#define lapi_put LapiFunc::getInstance()->Put
#define lapi_putv LapiFunc::getInstance()->Putv
#define lapi_qenv LapiFunc::getInstance()->Qenv
#define lapi_rmw LapiFunc::getInstance()->Rmw
#define lapi_rmw64 LapiFunc::getInstance()->Rmw64
#define lapi_senv LapiFunc::getInstance()->Senv
#define lapi_setcntr LapiFunc::getInstance()->Setcntr
#define lapi_term LapiFunc::getInstance()->Term

#define lapi_util LapiFunc::getInstance()->Util
#define lapi_waitcntr LapiFunc::getInstance()->Waitcntr
#define lapi_xfer LapiFunc::getInstance()->Xfer

#define lapi_nopoll_wait LapiFunc::getInstance()->Nopoll_wait
#define lapi_purge_totask LapiFunc::getInstance()->Purge_totask
#define lapi_resume_totask LapiFunc::getInstance()->Resume_totask
#define lapi_setcntr_wstatus LapiFunc::getInstance()->Setcntr_wstatus

#define lapi_cau_group_create LapiFunc::getInstance()->Cau_group_create;
#define lapi_cau_group_destroy LapiFunc::getInstance()->Cau_group_destroy;
#define lapi_cau_multicast LapiFunc::getInstance()->Cau_multicast;
#define lapi_cau_reduce LapiFunc::getInstance()->Cau_reduce;

#else

#define lapi_addr_get LAPI_Addr_get
#define lapi_addr_set LAPI_Addr_set
#define lapi_address LAPI_Address
#define lapi_address_init LAPI_Address_init
#define lapi_address_init64 LAPI_Address_init64

#define lapi_amsend LAPI_Amsend
#define lapi_amsendv LAPI_Amsendv
#define lapi_fence LAPI_Fence
#define lapi_get LAPI_Get
#define lapi_getcntr LAPI_Getcntr
#define lapi_getv LAPI_Getv
#define lapi_gfence LAPI_Gfence
#define lapi_init LAPI_Init
#define lapi_msg_string LAPI_Msg_string
#define lapi_msgpoll LAPI_Msgpoll
#define lapi_probe LAPI_Probe

#define lapi_put LAPI_Put
#define lapi_putv LAPI_Putv
#define lapi_qenv LAPI_Qenv
#define lapi_rmw LAPI_Rmw
#define lapi_rmw64 LAPI_Rmw64
#define lapi_senv LAPI_Senv
#define lapi_setcntr LAPI_Setcntr
#define lapi_term LAPI_Term

#define lapi_util LAPI_Util
#define lapi_waitcntr LAPI_Waitcntr
#define lapi_xfer LAPI_Xfer

#define lapi_nopoll_wait LAPI_Nopoll_wait
#define lapi_purge_totask LAPI_Purge_totask
#define lapi_resume_totask LAPI_Resume_totask
#define lapi_setcntr_wstatus LAPI_Setcntr_wstatus

#define lapi_cau_group_create LAPI_Cau_group_create
#define lapi_cau_group_destroy LAPI_Cau_group_destroy
#define lapi_cau_multicast LAPI_Cau_multicast
#define lapi_cau_reduce LAPI_Cau_reduce

#endif //LAPI_DLOPEN


#endif
