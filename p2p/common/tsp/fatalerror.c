/* ************************************************************************* */
/*                            IBM Confidential                               */
/*                          OCO Source Materials                             */
/*                      IBM XL UPC Alpha Edition, V0.9                       */
/*                                                                           */
/*                      Copyright IBM Corp. 2005, 2007.                      */
/*                                                                           */
/* The source code for this program is not published or otherwise divested   */
/* of its trade secrets, irrespective of what has been deposited with the    */
/* U.S. Copyright Office.                                                    */
/* ************************************************************************* */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include "../include/pgasrt.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* *********************************************************************** */
/*           error notification followed by immediate shutdown             */
/* *********************************************************************** */

#pragma weak __pgasrt_fatalerror=___pgasrt_fatalerror
void ___pgasrt_fatalerror (int errcode, const char * strg, ...)
{
  char buffer[120];
  va_list ap;
  va_start(ap, strg);
  vsnprintf (buffer, 119, strg, ap);
  va_end(ap);
  if (errcode==0) errcode=-1;
  fprintf(stderr, "PGAS Runtime Error %d: %s\n",errcode,buffer);

  // char * z = 0;
  // *z = 0xFF;
  _exit(1);
}

/* *********************************************************************** */
/*                            signal handler                               */
/* *********************************************************************** */

#if defined(__i386__) && defined(__linux__)
#warning "Compiling INTEL stacktrace utility"

char * strsignal (int);

void __pgasrt_sighandler (int signo)
{
  struct sigcontext *scp = (struct sigcontext *)((unsigned)&signo + 
						 sizeof(signo));  
  fprintf(stderr, "Node %d: signal %u %s\n", 
	  PGASRT_MYNODE, signo, strsignal(signo));
  fprintf(stderr, "scp  = %p\n", (void *)scp);
  fprintf(stderr, "eip  = %p\n", (void *)scp->eip);
  fprintf(stderr, "ebp  = %p\n", (void *)scp->ebp);
  fprintf(stderr, "esp  = %p\n", (void *)scp->esp);
  fprintf(stderr, "\nfunction call chain:\n");
  fprintf(stderr, " %p\n", (void *)scp->eip);
  
  unsigned frameCount, *frame = (unsigned *)scp->ebp;
  for (frameCount = 0; frameCount < 50; frameCount++)
    {
      if (!frame) break;
      fprintf(stderr, " %p\n", (void *)frame[1]);
      unsigned *next_frame = (unsigned *)frame[0];
      if (next_frame <= frame) break;
      frame = next_frame;
    }
  exit(1);
}

#elif  (defined(_POWER) || defined (_PPC) || defined (__PPC))&& defined(linux)
#warning "Compiling PowerPC/Linux signal trace utility"

void __pgasrt_sighandler (int signo, struct sigcontext *scp)
{
  fprintf(stderr, "%d: signal %u %s\n", 
	  PGASRT_MYTHREAD, signo, strsignal(signo));
  fprintf(stderr, "iar = 0x%08x\n", (unsigned)scp->regs->nip);
  fprintf(stderr, "lr  = 0x%08x\n", (unsigned)scp->regs->link);
  fprintf(stderr, "sp  = 0x%08x\n", (unsigned)scp->regs->gpr[1]);

  unsigned frameCount;
  unsigned sp =  (unsigned)scp->regs->gpr[1];
  unsigned iar = (unsigned)scp->regs->nip;

  fprintf(stderr, "\nfunction call chain:\n");
  fprintf(stderr, " 0x%08x\n", iar);
  for (frameCount = 0; frameCount < 50; frameCount++)
    {
      unsigned *frame = (unsigned *)sp;
      if (!frame) break;
      unsigned next_sp = frame[0];
      unsigned pc = frame[1] - 4;
      fprintf(stderr, " 0x%x\n", pc);
      if (next_sp <= sp)  break;
      sp = next_sp;
    }
  exit(1);
}

#elif  (defined(_POWER) || defined (_PPC) || defined (__PPC))&& defined(_AIX)
#warning "Compiling PowerPC/AIX signal trace utility"

void __pgasrt_sighandler (int signo, siginfo_t * sigInfo, void * contextInfo)
{
  ucontext_t * ctx = (ucontext_t*)contextInfo;

  fprintf(stderr, "%d: signal %u %s\n", 
	  PGASRT_MYTHREAD, signo, strsignal(signo));
  fprintf(stderr, "iar = 0x%08x\n", ctx->uc_mcontext.jmp_context.iar);
  fprintf(stderr, "lr  = 0x%08x\n", ctx->uc_mcontext.jmp_context.lr);
  fprintf(stderr, "sp  = 0x%08x\n", ctx->uc_mcontext.jmp_context.gpr[1]);

  size_t frameCount;
  size_t sp  =  (size_t)ctx->uc_mcontext.jmp_context.gpr[1];
  size_t iar = (size_t)ctx->uc_mcontext.jmp_context.iar;

  fprintf(stderr, "\nfunction call chain:\n");
  fprintf(stderr, " 0x%08x\n", iar);
  for (frameCount = 0; frameCount < 50; frameCount++)
    {
      void **frame = (void **)sp;
      if (!frame) break;
      void * next_sp = frame[0];
      void * pc      = (void*)((size_t)frame[1] - sizeof(void *));
      fprintf(stderr, " 0x%x\n", pc);
      if ((size_t)next_sp <= sp) break;
      sp = (size_t)next_sp;
    }
  exit(1);
}

#else
#warning "No stack traces for this architecture"

void __pgasrt_sighandler (int signo)
{  
  fprintf(stderr, "%d: signal %u %s\n",
          PGASRT_MYTHREAD, signo, strsignal(signo));
  exit (1);
}

#endif

#if defined(__cplusplus)
}
#endif
