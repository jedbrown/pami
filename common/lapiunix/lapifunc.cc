///
/// \file common/lapiunix/lapifunc.cc
/// \brief Encapsulation for LAPI function pointers.
///
/****************************************************************************
 Classes: LapiFunc

 Description: Encapsulation for LAPI function pointers.

 Author: Nicole Nie

 History:
   Date     Who ID    Description
   -------- --- ---   -----------
   07/14/09 nieyy     Initial code

****************************************************************************/

#include "lapifunc.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>
#include "util/common.h"
#include "lapierror.h"

const char *lapi_cmd_list[] = {
    "LAPI_Addr_get",
    "LAPI_Addr_set",
    "LAPI_Address",
    "LAPI_Address_init",
    "LAPI_Address_init64",
    "LAPI_Amsend",
    "LAPI_Amsendv",
    "LAPI_Fence",
    "LAPI_Get",
    "LAPI_Getcntr",
    "LAPI_Getv",
    "LAPI_Gfence",
    "LAPI_Init",
    "LAPI_Msg_string",
    "LAPI_Msgpoll",
    "LAPI_Probe",
    "LAPI_Put",
    "LAPI_Putv",
    "LAPI_Qenv",
    "LAPI_Rmw",
    "LAPI_Rmw64",
    "LAPI_Senv",
    "LAPI_Setcntr",
    "LAPI_Term",
    "LAPI_Util",
    "LAPI_Waitcntr",
    "LAPI_Xfer",
    "LAPI_Nopoll_wait",
    "LAPI_Purge_totask",
    "LAPI_Resume_totask",
    "LAPI_Setcntr_wstatus",
    "LAPI_Cau_group_create",
    "LAPI_Cau_group_destroy",
    "LAPI_Cau_multicast",
    "LAPI_Cau_reduce"
};

enum {
    I_Addr_get = 0,
    I_Addr_set,
    I_Address,
    I_Address_init,
    I_Address_init64,
    I_Amsend,
    I_Amsendv,
    I_Fence,
    I_Get,
    I_Getcntr,
    I_Getv,
    I_Gfence,
    I_Init,
    I_Msg_string,
    I_Msgpoll,
    I_Probe,
    I_Put,
    I_Putv,
    I_Qenv,
    I_Rmw,
    I_Rmw64,
    I_Senv,
    I_Setcntr,
    I_Term,
    I_Util,
    I_Waitcntr,
    I_Xfer,
    I_Nopoll_wait,
    I_Purge_totask,
    I_Resume_totask,
    I_Setcntr_wstatus,
    I_Cau_group_create,
    I_Cau_group_destroy,
    I_Cau_multicast,
    I_Cau_reduce
};

LapiFunc *LapiFunc::instance = NULL;

LapiFunc * LapiFunc::getInstance()
{
    if (instance == NULL) {
        instance = new LapiFunc();
        int rc = instance->load();
        // FixMe: error handling
        PAMI_assert(rc == 0);
    }
    return instance;
}

LapiFunc::LapiFunc()
{
    dlopen_file = NULL;

    addr_get_hndlr = NULL;
    addr_set_hndlr = NULL;
    address_hndlr = NULL;
    address_init_hndlr = NULL;
    address_init64_hndlr = NULL;

    amsend_hndlr = NULL;
    amsendv_hndlr = NULL;
    fence_hndlr = NULL;
    get_hndlr = NULL;
    getcntr_hndlr = NULL;
    getv_hndlr = NULL;
    gfence_hndlr = NULL;
    init_hndlr = NULL;
    msg_string_hndlr = NULL;
    msgpoll_hndlr = NULL;
    probe_hndlr = NULL;

    put_hndlr = NULL;
    putv_hndlr = NULL;
    qenv_hndlr = NULL;
    rmw_hndlr = NULL;
    rmw64_hndlr = NULL;
    senv_hndlr = NULL;
    setcntr_hndlr = NULL;
    term_hndlr = NULL;

    util_hndlr = NULL;
    waitcntr_hndlr = NULL;
    xfer_hndlr = NULL;

    nopoll_wait_hndlr = NULL;
    purge_totask_hndlr = NULL;
    resume_totask_hndlr = NULL;
    setcntr_wstatus_hndlr = NULL;

    cau_group_create_hndlr = NULL;
    cau_group_destroy_hndlr = NULL;
    cau_multicast_hndlr = NULL;
    cau_reduce_hndlr = NULL;
}

LapiFunc::~LapiFunc()
{
    if (dlopen_file) {
        ::dlclose(dlopen_file);
    }
}

int LapiFunc::load()
{
    char *filename = NULL;
#ifdef _LAPI_LINUX // Linux
    filename = "libpami.so";
#else
#ifdef __64BIT__
    filename = "libpami_r.a(liblapi64_r.o)";
#else  // 32-bit
    filename = "libpami_r.a(liblapi_r.o)";
#endif
#endif

#ifdef _LAPI_LINUX // Linux
    dlopen_file = ::dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
#else // aix
    dlopen_file = ::dlopen(filename, RTLD_NOW | RTLD_GLOBAL | RTLD_MEMBER);
#endif
    if (NULL == dlopen_file) {
        ::fprintf(stderr, "Error when opening %s: %s\n", filename, ::dlerror());
        return -1;
    }

    // FixMe: error handling
    addr_get_hndlr = (lapi_addr_get_hndlr *) import(lapi_cmd_list[I_Addr_get]);
    PAMI_assert(addr_get_hndlr);
    addr_set_hndlr = (lapi_addr_set_hndlr *) import(lapi_cmd_list[I_Addr_set]);
    PAMI_assert(addr_set_hndlr);
    address_hndlr = (lapi_address_hndlr *) import(lapi_cmd_list[I_Address]);
    PAMI_assert(address_hndlr);
    address_init_hndlr = (lapi_address_init_hndlr *) import(lapi_cmd_list[I_Address_init]);
    PAMI_assert(address_init_hndlr);
    address_init64_hndlr = (lapi_address_init64_hndlr *) import(lapi_cmd_list[I_Address_init64]);
    PAMI_assert(address_init64_hndlr);

    amsend_hndlr = (lapi_amsend_hndlr *) import(lapi_cmd_list[I_Amsend]);
    PAMI_assert(amsend_hndlr);
    amsendv_hndlr = (lapi_amsendv_hndlr *) import(lapi_cmd_list[I_Amsendv]);
    PAMI_assert(amsendv_hndlr);
    fence_hndlr = (lapi_fence_hndlr *) import(lapi_cmd_list[I_Fence]);
    PAMI_assert(fence_hndlr);
    get_hndlr = (lapi_get_hndlr *) import(lapi_cmd_list[I_Get]);
    PAMI_assert(get_hndlr);
    getcntr_hndlr = (lapi_getcntr_hndlr*) import(lapi_cmd_list[I_Getcntr]);
    PAMI_assert(getcntr_hndlr);
    getv_hndlr = (lapi_getv_hndlr *) import(lapi_cmd_list[I_Getv]);
    PAMI_assert(getv_hndlr);
    gfence_hndlr = (lapi_gfence_hndlr *) import(lapi_cmd_list[I_Gfence]);
    PAMI_assert(gfence_hndlr);
    init_hndlr = (lapi_init_hndlr *) import(lapi_cmd_list[I_Init]);
    PAMI_assert(init_hndlr);
    msg_string_hndlr = (lapi_msg_string_hndlr *) import(lapi_cmd_list[I_Msg_string]);
    PAMI_assert(msg_string_hndlr);
    msgpoll_hndlr = (lapi_msgpoll_hndlr *) import(lapi_cmd_list[I_Msgpoll]);
    PAMI_assert(msgpoll_hndlr);
    probe_hndlr = (lapi_probe_hndlr*) import(lapi_cmd_list[I_Probe]);
    PAMI_assert(probe_hndlr);

    put_hndlr = (lapi_put_hndlr *) import(lapi_cmd_list[I_Put]);
    PAMI_assert(put_hndlr);
    putv_hndlr = (lapi_putv_hndlr *) import(lapi_cmd_list[I_Putv]);
    PAMI_assert(putv_hndlr);
    qenv_hndlr = (lapi_qenv_hndlr *) import(lapi_cmd_list[I_Qenv]);
    PAMI_assert(qenv_hndlr);
    rmw_hndlr = (lapi_rmw_hndlr *) import(lapi_cmd_list[I_Rmw]);
    PAMI_assert(rmw_hndlr);
    rmw64_hndlr = (lapi_rmw64_hndlr *) import(lapi_cmd_list[I_Rmw64]);
    PAMI_assert(rmw64_hndlr);
    senv_hndlr = (lapi_senv_hndlr *) import(lapi_cmd_list[I_Senv]);
    PAMI_assert(senv_hndlr);
    setcntr_hndlr = (lapi_setcntr_hndlr *) import(lapi_cmd_list[I_Setcntr]);
    PAMI_assert(setcntr_hndlr);
    term_hndlr = (lapi_term_hndlr *) import(lapi_cmd_list[I_Term]);
    PAMI_assert(term_hndlr);

    util_hndlr = (lapi_util_hndlr *) import(lapi_cmd_list[I_Util]);
    PAMI_assert(util_hndlr);
    waitcntr_hndlr = (lapi_waitcntr_hndlr *) import(lapi_cmd_list[I_Waitcntr]);
    PAMI_assert(waitcntr_hndlr);
    xfer_hndlr = (lapi_xfer_hndlr *) import(lapi_cmd_list[I_Xfer]);
    PAMI_assert(xfer_hndlr);

    nopoll_wait_hndlr = (lapi_nopoll_wait_hndlr *) import(lapi_cmd_list[I_Nopoll_wait]);
    PAMI_assert(nopoll_wait_hndlr);
    purge_totask_hndlr = (lapi_purge_totask_hndlr *) import(lapi_cmd_list[I_Purge_totask]);
    PAMI_assert(purge_totask_hndlr);
    resume_totask_hndlr = (lapi_resume_totask_hndlr *) import(lapi_cmd_list[I_Resume_totask]);
    PAMI_assert(resume_totask_hndlr);
    setcntr_wstatus_hndlr = (lapi_setcntr_wstatus_hndlr *) import(lapi_cmd_list[I_Setcntr_wstatus]);
    PAMI_assert(setcntr_wstatus_hndlr);

    cau_group_create_hndlr = (lapi_cau_group_create_hndlr *) import(lapi_cmd_list[I_Cau_group_create]);
    PAMI_assert(cau_group_create_hndlr);
    cau_group_destroy_hndlr = (lapi_cau_group_destroy_hndlr *) import(lapi_cmd_list[I_Cau_group_destroy]);
    PAMI_assert(cau_group_destroy_hndlr);
    cau_multicast_hndlr = (lapi_cau_multicast_hndlr *) import(lapi_cmd_list[I_Cau_multicast]);
    PAMI_assert(cau_multicast_hndlr);
    cau_reduce_hndlr = (lapi_cau_reduce_hndlr *) import(lapi_cmd_list[I_Cau_reduce]);
    PAMI_assert(cau_reduce_hndlr);
    return 0;
}

int LapiFunc::Addr_get(lapi_handle_t hndl, void **addr, int addr_hndl)
{
    return(addr_get_hndlr(hndl, addr, addr_hndl));
}

int LapiFunc::Addr_set(lapi_handle_t hndl, void *addr, int addr_hndl)
{
    return(addr_set_hndlr(hndl, addr, addr_hndl));
}

int LapiFunc::Address(void *my_addr, ulong *ret_addr)
{
    return(address_hndlr(my_addr, ret_addr));
}

int LapiFunc::Address_init(lapi_handle_t hndl, void *my_tab, void *add_tab[])
{
    return(address_init_hndlr(hndl, my_tab, add_tab));
}

int LapiFunc::Address_init64(lapi_handle_t hndl, lapi_long_t my_tab, lapi_long_t *add_tab)
{
    return(address_init64_hndlr(hndl, my_tab, add_tab));
}

int LapiFunc::Amsend(lapi_handle_t hndl, uint tgt, void *hdr_hdl, void *uhdr, uint uhdr_len,
        void *udata, ulong udata_len, lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr)
{
    return(amsend_hndlr(hndl, tgt, hdr_hdl, uhdr, uhdr_len, udata, udata_len, tgt_cntr, org_cntr, cmpl_cntr));
}

int LapiFunc::Amsendv(lapi_handle_t hndl, uint tgt, void *hdr_hdl, void *uhdr,uint uhdr_len,
        lapi_vec_t *org_vec, lapi_cntr_t  *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr)
{
    return(amsendv_hndlr(hndl, tgt, hdr_hdl, uhdr, uhdr_len, org_vec, tgt_cntr, org_cntr, cmpl_cntr));
}

int LapiFunc::Fence(lapi_handle_t hndl)
{
    return(fence_hndlr(hndl));
}

int LapiFunc::Get(lapi_handle_t hndl, uint tgt, ulong len, void *tgt_addr, void *org_addr,
        lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr)
{
    return(get_hndlr(hndl, tgt, len, tgt_addr, org_addr, tgt_cntr, org_cntr));
}

int LapiFunc::Getcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int *val)
{
    return(getcntr_hndlr(hndl, cntr, val));
}

int LapiFunc::Getv(lapi_handle_t hndl, uint tgt, lapi_vec_t *tgt_vec, lapi_vec_t *org_vec,
        lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr)
{
    return(getv_hndlr(hndl, tgt, tgt_vec, org_vec, tgt_cntr, org_cntr));
}

int LapiFunc::Gfence(lapi_handle_t hndl)
{
    return(gfence_hndlr(hndl));
}

int LapiFunc::Init(lapi_handle_t *hndl, lapi_info_t *lapi_info)
{
    return (init_hndlr(hndl, lapi_info));
}

int LapiFunc::Msg_string(int error_code, void *buf)
{
    return(msg_string_hndlr(error_code, buf));
}

int LapiFunc::Msgpoll(lapi_handle_t hndl, uint cnt, lapi_msg_info_t *info)
{
    return(msgpoll_hndlr(hndl, cnt, info));
}

int LapiFunc::Probe(lapi_handle_t hndl)
{
    return(probe_hndlr(hndl));
}

int LapiFunc::Put(lapi_handle_t hndl, uint tgt, ulong len, void *tgt_addr, void *org_addr,
        lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr)
{
    return(put_hndlr(hndl, tgt, len, tgt_addr, org_addr, tgt_cntr, org_cntr, cmpl_cntr));
}

int LapiFunc::Putv(lapi_handle_t hndl, uint tgt, lapi_vec_t *tgt_vec, lapi_vec_t *org_addr,
        lapi_cntr_t *tgt_cntr, lapi_cntr_t *org_cntr, lapi_cntr_t *cmpl_cntr)
{
    return(putv_hndlr(hndl, tgt, tgt_vec, org_addr, tgt_cntr, org_cntr, cmpl_cntr));
}

int LapiFunc::Qenv(lapi_handle_t hndl, lapi_query_t query, int *ret_val)
{
    return(qenv_hndlr(hndl, query, ret_val));
}

int LapiFunc::Rmw(lapi_handle_t hndl, RMW_ops_t op, uint tgt, int *tgt_var, int *in_val,
        int *prev_tgt_val, lapi_cntr_t *org_cntr)
{
    return(rmw_hndlr(hndl, op, tgt, tgt_var, in_val, prev_tgt_val, org_cntr));
}

int LapiFunc::Rmw64(lapi_handle_t hndl, RMW_ops_t op, uint tgt, long long *tgt_var,
        long long *in_val, long long *prev_tgt_val, lapi_cntr_t *org_cntr)
{
    return(rmw64_hndlr(hndl, op, tgt, tgt_var, in_val, prev_tgt_val, org_cntr));
}

int LapiFunc::Senv(lapi_handle_t hndl, lapi_query_t query, int set_val)
{
    return(senv_hndlr(hndl, query, set_val));
}

int LapiFunc::Setcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int val)
{
    return(setcntr_hndlr(hndl, cntr, val));
}

int LapiFunc::Term(lapi_handle_t hndl)
{
    return(term_hndlr(hndl));
}

int LapiFunc::Util(lapi_handle_t hndl, lapi_util_t *util_p)
{
    return(util_hndlr(hndl, util_p));
}

int LapiFunc::Waitcntr(lapi_handle_t hndl, lapi_cntr_t *cntr, int val, int *cur_cntr_val)
{
    return(waitcntr_hndlr(hndl, cntr, val, cur_cntr_val));
}

int LapiFunc::Xfer(lapi_handle_t hndl, lapi_xfer_t *xfer_cmd)
{
    return(xfer_hndlr(hndl, xfer_cmd));
}

int LapiFunc::Nopoll_wait(lapi_handle_t hndl, lapi_cntr_t *cntr_ptr, int val, int *cur_cntr_val)
{
    return(nopoll_wait_hndlr(hndl, cntr_ptr, val, cur_cntr_val));
}

int LapiFunc::Purge_totask(lapi_handle_t hndl, css_task_t dest)
{
    return(purge_totask_hndlr(hndl, dest));
}

int LapiFunc::Resume_totask(lapi_handle_t hndl, css_task_t dest)
{
    return(resume_totask_hndlr(hndl, dest));
}

int LapiFunc::Setcntr_wstatus(lapi_handle_t hndl, lapi_cntr_t *cntr, int val, uint *dest, int *dest_status)
{
    return(setcntr_wstatus_hndlr(hndl, cntr, val, dest, dest_status));
}


int LapiFunc::Cau_group_create(lapi_handle_t hndl, uint group_id,uint num_tasks, uint *task_list)
{
    return(cau_group_create_hndlr(hndl,group_id,num_tasks,task_list));
}

int LapiFunc::Cau_group_destroy(lapi_handle_t hndl, uint group_id)
{
    return(cau_group_destroy_hndlr(hndl,group_id));
}

int LapiFunc::Cau_multicast(lapi_handle_t hndl, uint group,
                            int hdr_hdl, void *hdr, uint hdr_len, void *data, ulong data_len,
                            compl_hndlr_t done, void *cookie)
{
    return(cau_multicast_hndlr(hndl,group,hdr_hdl,hdr,hdr_len,data,data_len,done,cookie));
}

int LapiFunc::Cau_reduce(lapi_handle_t hndl, uint group,
                         int hdr_hdl, void *hdr, uint hdr_len, void *data, ulong data_len,
                         cau_reduce_op_t op, compl_hndlr_t done, void *cookie)
{
    return(cau_reduce_hndlr(hndl,group,hdr_hdl,hdr,hdr_len,data,data_len,op,done,cookie));
}





void * LapiFunc::import(const char * funcname)
{
    void *handle = NULL;
    char *error = NULL;

    ::dlerror();    /* Clear any existing error */

    handle = ::dlsym(dlopen_file, funcname);
    if ((error = ::dlerror()) != NULL)  {
        ::fprintf(stderr, "Error when taking the handle of %s: %s\n", funcname, error);
        handle = NULL;
    }

    return handle;
}
