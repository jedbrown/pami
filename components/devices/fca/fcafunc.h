/**
 * \file common/FCAunix/FCAfunc.h
 * \brief Encapsulation for FCA function pointers.
 */
#ifndef __FcaFunc_h__
#define __FcaFunc_h__

#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "components/devices/fca/fca_api.h"
#include "components/devices/fca/core/fca_errno.h"

// Define this to turn on dlopened FCA functions
#define FCA_DLOPEN
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
  "fca_get_dtype_size",
  "fca_strerror",
  "fca_default_config"
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
  I_Get_dtype_size,
  I_Strerror,
  I_Default_config
};

class FCAFunc
{
public:
  typedef       int   (fca_get_version_handler_t)(void);
  typedef const char *(fca_get_version_string_handler_t)(void);
  typedef       int   (fca_init_handler_t)(struct fca_init_spec *fca_spec, fca_t **fca_context);
  typedef       void  (fca_cleanup_handler_t)(fca_t *context);
  typedef       void  (fca_progress_handler_t)(fca_t *context);
  typedef       void *(fca_get_rank_info_handler_t)(fca_t *context, int *buf_len);
  typedef       void  (fca_free_rank_info_handler_t)(void *rank_info);
  typedef       int   (fca_comm_new_handler_t)(fca_t *context, fca_comm_new_spec_t *spec,fca_comm_desc_t *comm_desc);
  typedef       int   (fca_comm_end_handler_t)(fca_t *context, int comm_id);
  typedef       int   (fca_comm_init_handler_t)(fca_t *context, fca_comm_init_spec_t *spec, fca_comm_t **fca_comm);
  typedef       void  (fca_comm_destroy_handler_t)(fca_comm_t *comm);
  typedef       int   (fca_comm_get_caps_handler_t)(fca_comm_t *comm, fca_comm_caps_t *caps);
  typedef       int   (fca_do_reduce_handler_t)(fca_comm_t *comm, fca_reduce_spec_t *spec);
  typedef       int   (fca_do_all_reduce_handler_t)(fca_comm_t *comm, fca_reduce_spec_t *spec);
  typedef       int   (fca_do_bcast_handler_t)(fca_comm_t *comm, fca_bcast_spec_t *spec);
  typedef       int   (fca_do_allgather_handler_t)(fca_comm_t *comm, fca_gather_spec_t *spec);
  typedef       int   (fca_do_allgatherv_handler_t)(fca_comm_t *comm, fca_gatherv_spec_t *spec);
  typedef       int   (fca_do_barrier_handler_t)(fca_comm_t *comm);
  typedef       int   (fca_translate_mpi_op_handler_t)(char *mpi_op_str);
  typedef       int   (fca_translate_mpi_dtype_handler_t)(char *mpi_dtype_str);
  typedef       int   (fca_get_dtype_size_handler_t)(enum fca_reduce_dtype_t dtype);
  typedef const char *(fca_strerror_handler_t)(int error);
  typedef fca_config_t fca_config_t_value_t;

  static FCAFunc *getInstance();
  int             Load();
  unsigned long   Get_version(void);
  const char     *Get_version_string(void);
  int             Init(struct fca_init_spec *fca_spec, fca_t **fca_context);
  void            Cleanup(fca_t *context);
  void            Progress(fca_t *context);
  void           *Get_rank_info(fca_t *context, int *buf_len);
  void            Free_rank_info(void *rank_info);
  int             Comm_new(fca_t *context, fca_comm_new_spec_t *spec,fca_comm_desc_t *comm_desc);
  int             Comm_end(fca_t *context, int comm_id);
  int             Comm_init(fca_t *context, fca_comm_init_spec_t *spec, fca_comm_t **fca_comm);
  void            Comm_destroy(fca_comm_t *comm);
  int             Comm_get_caps(fca_comm_t *comm, fca_comm_caps_t *caps);
  int             Do_reduce(fca_comm_t *comm, fca_reduce_spec_t *spec);
  int             Do_all_reduce(fca_comm_t *comm, fca_reduce_spec_t *spec);
  int             Do_bcast(fca_comm_t *comm, fca_bcast_spec_t *spec);
  int             Do_allgather(fca_comm_t *comm, fca_gather_spec_t *spec);
  int             Do_allgatherv(fca_comm_t *comm, fca_gatherv_spec_t *spec);
  int             Do_barrier(fca_comm_t *comm);
  int             Translate_mpi_op(char *mpi_op_str);
  int             Translate_mpi_dtype(char *mpi_dtype_str);
  int             Get_dtype_size(enum fca_reduce_dtype_t dtype);
  const char     *Strerror(int error);
private:
  FCAFunc();
  ~FCAFunc();
  void              * import(const char *funcname);
  void               *dlopen_file;

  static FCAFunc                    *instance;
  fca_get_version_handler_t         *get_version_handler;
  fca_get_version_string_handler_t  *get_version_string_handler;
  fca_init_handler_t                *init_handler;
  fca_cleanup_handler_t             *cleanup_handler;
  fca_progress_handler_t            *progress_handler;
  fca_get_rank_info_handler_t       *get_rank_info_handler;
  fca_free_rank_info_handler_t      *free_rank_info_handler;
  fca_comm_new_handler_t            *comm_new_handler;
  fca_comm_end_handler_t            *comm_end_handler;
  fca_comm_init_handler_t           *comm_init_handler;
  fca_comm_destroy_handler_t        *comm_destroy_handler;
  fca_comm_get_caps_handler_t       *comm_get_caps_handler;
  fca_do_reduce_handler_t           *do_reduce_handler;
  fca_do_all_reduce_handler_t       *do_all_reduce_handler;
  fca_do_bcast_handler_t            *do_bcast_handler;
  fca_do_allgather_handler_t        *do_allgather_handler;
  fca_do_allgatherv_handler_t       *do_allgatherv_handler;
  fca_do_barrier_handler_t          *do_barrier_handler;
  fca_translate_mpi_op_handler_t    *translate_mpi_op_handler;
  fca_translate_mpi_dtype_handler_t *translate_mpi_dtype_handler;
  fca_get_dtype_size_handler_t      *get_dtype_size_handler;
  fca_strerror_handler_t            *strerror_handler;
public:
  fca_config_t_value_t              *config_t_value;
};

inline FCAFunc::FCAFunc():
  dlopen_file(NULL),
  get_version_handler(NULL),
  get_version_string_handler(NULL),
  init_handler(NULL),
  cleanup_handler(NULL),
  progress_handler(NULL),
  get_rank_info_handler(NULL),
  free_rank_info_handler(NULL),
  comm_new_handler(NULL),
  comm_end_handler(NULL),
  comm_init_handler(NULL),
  comm_destroy_handler(NULL),
  comm_get_caps_handler(NULL),
  do_reduce_handler(NULL),
  do_all_reduce_handler(NULL),
  do_bcast_handler(NULL),
  do_allgather_handler(NULL),
  do_allgatherv_handler(NULL),
  do_barrier_handler(NULL),
  translate_mpi_op_handler(NULL),
  translate_mpi_dtype_handler(NULL),
  get_dtype_size_handler(NULL),
  strerror_handler(NULL),
  config_t_value(NULL)
{
}
inline FCAFunc::~FCAFunc()
{
  if (dlopen_file) {
    ::dlclose(dlopen_file);
  }
}

inline FCAFunc * FCAFunc::getInstance()
{
  if (instance == NULL)
    {
      instance = new FCAFunc();
      int rc = instance->Load();
      if(rc != 0)
        {
          delete instance;
          instance = NULL;
        }
    }
  return instance;
}


inline void * FCAFunc::import(const char * funcname)
{
  void *handle = NULL;
  char *error = NULL;
  ::dlerror();    /* Clear any existing error */
  handle = ::dlsym(dlopen_file, funcname);
  if ((error = ::dlerror()) != NULL)
    {
      ::fprintf(stderr, "Error when taking the handle of %s: %s\n", funcname, error);
      handle = NULL;
    }
  return handle;
}

inline int FCAFunc::Load()
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
  strerror_handler            = (fca_strerror_handler_t *)(uintptr_t)import(FCA_cmd_list[I_Strerror]);
  config_t_value              = (fca_config_t_value_t *)(uintptr_t)import(FCA_cmd_list[I_Default_config]);

  
#define FCA_CHECK_FN(x) if(x == NULL) return -1;
  FCA_CHECK_FN(get_version_handler);
  FCA_CHECK_FN(get_version_string_handler);
  FCA_CHECK_FN(init_handler);
  FCA_CHECK_FN(cleanup_handler);
  FCA_CHECK_FN(progress_handler);
  FCA_CHECK_FN(get_rank_info_handler);
  FCA_CHECK_FN(free_rank_info_handler);
  FCA_CHECK_FN(comm_new_handler);
  FCA_CHECK_FN(comm_end_handler);
  FCA_CHECK_FN(comm_init_handler);
  FCA_CHECK_FN(comm_destroy_handler);
  FCA_CHECK_FN(comm_get_caps_handler);
  FCA_CHECK_FN(do_reduce_handler);
  FCA_CHECK_FN(do_all_reduce_handler);
  FCA_CHECK_FN(do_bcast_handler);
  FCA_CHECK_FN(do_allgather_handler);
  FCA_CHECK_FN(do_allgatherv_handler);
  FCA_CHECK_FN(do_barrier_handler);
  FCA_CHECK_FN(translate_mpi_op_handler);
  FCA_CHECK_FN(translate_mpi_dtype_handler);
  FCA_CHECK_FN(get_dtype_size_handler);
  FCA_CHECK_FN(strerror_handler);
  // It may be OK for this to be NULL
  FCA_CHECK_FN(config_t_value);
#undef FCA_CHECK_FN
  return 0;
}


inline unsigned long FCAFunc::Get_version(void)
{
  return(get_version_handler());
}
inline const char * FCAFunc::Get_version_string(void)
{
  return(get_version_string_handler());
}
inline int FCAFunc::Init(struct fca_init_spec *fca_spec, fca_t **fca_context)
{
  return(init_handler(fca_spec,fca_context));
}
inline void FCAFunc::Cleanup(fca_t *context)
{
  return(cleanup_handler(context));
}
inline void FCAFunc::Progress(fca_t *context)
{
  return(progress_handler(context));
}
inline void *FCAFunc::Get_rank_info(fca_t *context, int *buf_len)
{
  return(get_rank_info_handler(context,buf_len));
}
inline void FCAFunc::Free_rank_info(void *rank_info)
{
  return(free_rank_info_handler(rank_info));
}
inline int FCAFunc::Comm_new(fca_t *context, fca_comm_new_spec_t *spec,fca_comm_desc_t *comm_desc)
{
  return(comm_new_handler(context,spec,comm_desc));
}
inline int FCAFunc::Comm_end(fca_t *context, int comm_id)
{
  return(comm_end_handler(context, comm_id));
}
inline int FCAFunc::Comm_init(fca_t *context, fca_comm_init_spec_t *spec, fca_comm_t **fca_comm)
{
  return(comm_init_handler(context,spec,fca_comm));
}
inline void FCAFunc::Comm_destroy(fca_comm_t *comm)
{
  return(comm_destroy_handler(comm));
}
inline int FCAFunc::Comm_get_caps(fca_comm_t *comm, fca_comm_caps_t *caps)
{
  return(comm_get_caps_handler(comm,caps));
}
inline int FCAFunc::Do_reduce(fca_comm_t *comm, fca_reduce_spec_t *spec)
{
  return(do_reduce_handler(comm,spec));
}
inline int FCAFunc::Do_all_reduce(fca_comm_t *comm, fca_reduce_spec_t *spec)
{
  return(do_all_reduce_handler(comm,spec));
}
inline int FCAFunc::Do_bcast(fca_comm_t *comm, fca_bcast_spec_t *spec)
{
  return(do_bcast_handler(comm,spec));
}
inline int FCAFunc::Do_allgather(fca_comm_t *comm, fca_gather_spec_t *spec)
{
  return(do_allgather_handler(comm,spec));
}
inline int FCAFunc::Do_allgatherv(fca_comm_t *comm, fca_gatherv_spec_t *spec)
{
  return(do_allgatherv_handler(comm,spec));
}
inline int FCAFunc::Do_barrier(fca_comm_t *comm)
{
  return(do_barrier_handler(comm));
}
inline int FCAFunc::Translate_mpi_op(char *mpi_op_str)
{
  return(translate_mpi_op_handler(mpi_op_str));
}
inline int FCAFunc::Translate_mpi_dtype(char *mpi_dtype_str)
{
  return(translate_mpi_dtype_handler(mpi_dtype_str));
}
inline int FCAFunc::Get_dtype_size(enum fca_reduce_dtype_t dtype)
{
  return(get_dtype_size_handler(dtype));
}
inline const char * FCAFunc::Strerror(int error)
{
  return(strerror_handler(error));
}

#ifdef FCA_DLOPEN

// DLOPEN + FCA ENABLED PATH
#define FCA_Dlopen(x)           FCAFunc::getInstance()
#define FCA_Get_version         FCAFunc::getInstance()->Get_version
#define FCA_Get_version_string  FCAFunc::getInstance()->Get_version_string
#define FCA_Init                FCAFunc::getInstance()->Init
#define FCA_Cleanup             FCAFunc::getInstance()->Cleanup
#define FCA_Progress            FCAFunc::getInstance()->Progress
#define FCA_Get_rank_info       FCAFunc::getInstance()->Get_rank_info
#define FCA_Free_rank_info      FCAFunc::getInstance()->Free_rank_info
#define FCA_Comm_new            FCAFunc::getInstance()->Comm_new
#define FCA_Comm_end            FCAFunc::getInstance()->Comm_end
#define FCA_Comm_init           FCAFunc::getInstance()->Comm_init
#define FCA_Comm_destroy        FCAFunc::getInstance()->Comm_destroy
#define FCA_Comm_get_caps       FCAFunc::getInstance()->Comm_get_caps
#define FCA_Do_reduce           FCAFunc::getInstance()->Do_reduce
#define FCA_Do_all_reduce       FCAFunc::getInstance()->Do_all_reduce
#define FCA_Do_bcast            FCAFunc::getInstance()->Do_bcast
#define FCA_Do_allgather        FCAFunc::getInstance()->Do_allgather
#define FCA_Do_allgatherv       FCAFunc::getInstance()->Do_allgatherv
#define FCA_Do_barrier          FCAFunc::getInstance()->Do_barrier
#define FCA_Translate_mpi_op    FCAFunc::getInstance()->Translate_mpi_op
#define FCA_Translate_mpi_dtype FCAFunc::getInstance()->Translate_mpi_dtype
#define FCA_Get_dtype_size      FCAFunc::getInstance()->Get_dtype_size
#define FCA_Strerror            FCAFunc::getInstance()->Strerror
#define FCA_Default_config      (*(FCAFunc::getInstance()->config_t_value))

#include "components/devices/fca/fcafunc.cc"

#else
// NO DLOPEN + FCA ENABLED PATH
#define FCA_Get_version         fca_get_version
#define FCA_Get_version_string  fca_get_version_string
#define FCA_Init                fca_init
#define FCA_Cleanup             fca_cleanup
#define FCA_Progress            fca_progress
#define FCA_Get_rank_info       fca_get_rank_info
#define FCA_Free_rank_info      fca_free_rank_info
#define FCA_Comm_new            fca_comm_new
#define FCA_Comm_end            fca_comm_end
#define FCA_Comm_init           fca_comm_init
#define FCA_Comm_destroy        fca_comm_destroy
#define FCA_Comm_get_caps       fca_comm_get_caps
#define FCA_Do_reduce           fca_do_reduce
#define FCA_Do_all_reduce       fca_do_all_reduce
#define FCA_Do_bcast            fca_do_bcast
#define FCA_Do_allgather        fca_do_allgather
#define FCA_Do_allgatherv       fca_do_allgatherv
#define FCA_Do_barrier          fca_do_barrier
#define FCA_Translate_mpi_op    fca_translate_mpi_op
#define FCA_Translate_mpi_dtype fca_translate_mpi_dtype
#define FCA_Get_dtype_size      fca_get_dtype_size
#define FCA_Strerror            fca_strerror
#define FCA_Default_config      fca_default_config

#endif //FCA_DLOPEN

#endif
