///
/// \file  fcafunc.cc
/// \brief Encapsulation for FCA function pointers.
///
#include "fcafunc.h"
#include <stdint.h>
const char *FCA_cmd_list[] =
{
  "fca_get_version",
  "fca_get_version_string",
  "fca_init",
  "fca_cleanup",
  "fca_progress",
  "fca_get_rank_info",
  "fca_free_rank_info",
  "fca_comm_new",
  "fca_comm_end",
  "fca_comm_init",
  "fca_comm_destroy",
  "fca_comm_get_caps",
  "fca_do_reduce",
  "fca_do_all_reduce",
  "fca_do_bcast",
  "fca_do_allgather",
  "fca_do_allgatherv",
  "fca_do_barrier",
  "fca_translate_mpi_op",
  "fca_translate_mpi_dtype",
  "fca_get_dtype_size"
};

enum
{
  I_Get_version,
  I_Get_version_string,
  I_Init,
  I_Cleanup,
  I_Progress,
  I_Get_rank_info,
  I_Free_rank_info,
  I_Comm_new,
  I_Comm_end,
  I_Comm_init,
  I_Comm_destroy,
  I_Comm_get_caps,
  I_Do_reduce,
  I_Do_all_reduce,
  I_Do_bcast,
  I_Do_allgather,
  I_Do_allgatherv,
  I_Do_barrier,
  I_Translate_mpi_op,
  I_Translate_mpi_dtype,
  I_Get_dtype_size
};

FCAFunc *FCAFunc::instance = NULL;
FCAFunc * FCAFunc::getInstance()
{
  if (instance == NULL)
    {
      instance = new FCAFunc();
      int rc = instance->Load();
      assert(rc == 0);
    }
  return instance;
}

void * FCAFunc::import(const char * funcname)
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


int FCAFunc::Load()
{
#ifdef _LAPI_LINUX // Linux
  const char *filename = "libfca.so";
#else
#ifdef __64BIT__
  const char *filename = "libfca_r.a(libfca64_r.o)";
#else  // 32-bit
  const char* filename = "libfca_r.a(libfca_r.o)";
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
  get_version_handler         = (fca_get_version_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Get_version]);
  get_version_string_handler  = (fca_get_version_string_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Get_version_string]);
  init_handler                = (fca_init_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Init]);
  cleanup_handler             = (fca_cleanup_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Cleanup]);
  progress_handler            = (fca_progress_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Progress]);
  get_rank_info_handler       = (fca_get_rank_info_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Get_rank_info]);
  free_rank_info_handler      = (fca_free_rank_info_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Free_rank_info]);
  comm_new_handler            = (fca_comm_new_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Comm_new]);
  comm_end_handler            = (fca_comm_end_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Comm_end]);
  comm_init_handler           = (fca_comm_init_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Comm_init]);
  comm_destroy_handler        = (fca_comm_destroy_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Comm_destroy]);
  comm_get_caps_handler       = (fca_comm_get_caps_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Comm_get_caps]);
  do_reduce_handler           = (fca_do_reduce_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Do_reduce]);
  do_all_reduce_handler       = (fca_do_all_reduce_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Do_all_reduce]);
  do_bcast_handler            = (fca_do_bcast_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Do_bcast]);
  do_allgather_handler        = (fca_do_allgather_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Do_allgather]);
  do_allgatherv_handler       = (fca_do_allgatherv_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Do_allgatherv]);
  do_barrier_handler          = (fca_do_barrier_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Do_barrier]);
  translate_mpi_op_handler    = (fca_translate_mpi_op_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Translate_mpi_op]);
  translate_mpi_dtype_handler = (fca_translate_mpi_dtype_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Translate_mpi_dtype]);
  get_dtype_size_handler      = (fca_get_dtype_size_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Get_dtype_size]); 
  assert(get_version_handler);
  assert(get_version_string_handler);
  assert(init_handler);
  assert(cleanup_handler);
  assert(progress_handler);
  assert(get_rank_info_handler);
  assert(free_rank_info_handler);
  assert(comm_new_handler);
  assert(comm_end_handler);
  assert(comm_init_handler);
  assert(comm_destroy_handler);
  assert(comm_get_caps_handler);
  assert(do_reduce_handler);
  assert(do_all_reduce_handler);
  assert(do_bcast_handler);
  assert(do_allgather_handler);
  assert(do_allgatherv_handler);
  assert(do_barrier_handler);
  assert(translate_mpi_op_handler);
  assert(translate_mpi_dtype_handler);
  assert(get_dtype_size_handler);
  return 0;
}
