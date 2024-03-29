/* =============================================================================
 *
 * tm.h
 *
 * Utility defines for transactional memory
 *
 * =============================================================================
 *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Authors: Chi Cao Minh and Martin Trautmann
 *
 * =============================================================================
 *
 * For the license of bayes/sort.h and bayes/sort.c, please see the header
 * of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of kmeans, please see kmeans/LICENSE.kmeans
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of ssca2, please see ssca2/COPYRIGHT
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/mt19937ar.c and lib/mt19937ar.h, please see the
 * header of the files.
 * 
 * ------------------------------------------------------------------------
 * 
 * For the license of lib/rbtree.h and lib/rbtree.c, please see
 * lib/LEGALNOTICE.rbtree and lib/LICENSE.rbtree
 * 
 * ------------------------------------------------------------------------
 * 
 * Unless otherwise noted, the following license applies to STAMP files:
 * 
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 * 
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 */
/* Copyright (c) IBM Corp. 2014. */


#ifndef TM_H
#define TM_H 1

#ifdef __bgq__
#include <unistd.h>
#endif
/*#define USE_TLH  Moved to each benchmark's Defines.common.mk*/

#ifdef HAVE_CONFIG_H
# include "STAMP_config.h"
#endif

/* =============================================================================
 * Simulator Specific Interface
 *
 * MAIN(argc, argv)
 *     Declare the main function with argc being the identifier for the argument
 *     count and argv being the name for the argument string list
 *
 * MAIN_RETURN(int_val)
 *     Returns from MAIN function
 *
 * GOTO_SIM()
 *     Switch simulator to simulation mode
 *
 * GOTO_REAL()
 *     Switch simulator to non-simulation (real) mode
 *     Note: use in sequential region only
 *
 * IS_IN_SIM()
 *     Returns true if simulator is in simulation mode
 *
 * SIM_GET_NUM_CPU(var)
 *     Assigns the number of simulated CPUs to "var"
 *
 * P_MEMORY_STARTUP
 *     Start up the memory allocator system that handles malloc/free
 *     in parallel regions (but not in transactions)
 *
 * P_MEMORY_SHUTDOWN
 *     Shutdown the memory allocator system that handles malloc/free
 *     in parallel regions (but not in transactions)
 *
 * =============================================================================
 */
#ifdef SIMULATOR

#  include <simapi.h>

#  define MAIN(argc, argv)              void mainX (int argc, \
                                                    const char** argv, \
                                                    const char** envp)
#  define MAIN_RETURN(val)              return /* value is ignored */

#  define GOTO_SIM()                    goto_sim()
#  define GOTO_REAL()                   goto_real()
#  define IS_IN_SIM()                   (inSimulation)

#  define SIM_GET_NUM_CPU(var)          ({ \
                                            if (!IS_IN_SIM()) { \
                                                GOTO_SIM(); \
                                                var = Sim_GetNumCpus(); \
                                                GOTO_REAL(); \
                                            } else { \
                                                var = Sim_GetNumCpus(); \
                                            } \
                                            var; \
                                        })

#  define TM_PRINTF                     Sim_Print
#  define TM_PRINT0                     Sim_Print0
#  define TM_PRINT1                     Sim_Print1
#  define TM_PRINT2                     Sim_Print2
#  define TM_PRINT3                     Sim_Print3

#  include "memory.h"
#  define P_MEMORY_STARTUP(numThread)   do { \
                                            bool_t status; \
                                            status = memory_init((numThread), \
                                                                 ((1<<28) / numThread), \
                                                                 2); \
                                            assert(status); \
                                        } while (0) /* enforce comma */
#  define P_MEMORY_SHUTDOWN()           memory_destroy()

#else /* !SIMULATOR */

#  include <stdio.h>

#  define MAIN(argc, argv)              int main (int argc, char** argv)
#  define MAIN_RETURN(val)              return val

#  define GOTO_SIM()                    /* nothing */
#  define GOTO_REAL()                   /* nothing */
#  define IS_IN_SIM()                   (0)

#  define SIM_GET_NUM_CPU(var)          /* nothing */

#  define TM_PRINTF                     printf
#  define TM_PRINT0                     printf
#  define TM_PRINT1                     printf
#  define TM_PRINT2                     printf
#  define TM_PRINT3                     printf

#ifdef USE_TLH
#  include "memory.h"
#  define P_MEMORY_STARTUP(numThread)   do { \
                                            bool_t status; \
                                            status = memory_init((numThread), \
                                                                 ((1<<28) / numThread), \
                                                                 2); \
                                            assert(status); \
                                        } while (0) /* enforce comma */
#  define P_MEMORY_SHUTDOWN()           memory_destroy()
#else /* !USE_TLH */
#  define P_MEMORY_STARTUP(numThread)   /* nothing */
#  define P_MEMORY_SHUTDOWN()           /* nothing */
#endif /* !USE_TLH */

#endif /* !SIMULATOR */


/* =============================================================================
 * Transactional Memory System Interface
 *
 * TM_ARG
 * TM_ARG_ALONE
 * TM_ARGDECL
 * TM_ARGDECL_ALONE
 *     Used to pass TM thread meta data to functions (see Examples below)
 *
 * TM_STARTUP(numThread)
 *     Startup the TM system (call before any other TM calls)
 *
 * TM_SHUTDOWN()
 *     Shutdown the TM system
 *
 * TM_THREAD_ENTER()
 *     Call when thread first enters parallel region
 *
 * TM_THREAD_EXIT()
 *     Call when thread exits last parallel region
 *
 * P_MALLOC(size)
 *     Allocate memory inside parallel region
 *
 * P_FREE(ptr)
 *     Deallocate memory inside parallel region
 *
 * TM_MALLOC(size)
 *     Allocate memory inside atomic block / transaction
 *
 * TM_FREE(ptr)
 *     Deallocate memory inside atomic block / transaction
 *
 * TM_BEGIN()
 *     Begin atomic block / transaction
 *
 * TM_BEGIN_RO()
 *     Begin atomic block / transaction that only reads shared data
 *
 * TM_END()
 *     End atomic block / transaction
 *
 * TM_RESTART()
 *     Restart atomic block / transaction
 *
 * TM_EARLY_RELEASE()
 *     Remove speculatively read line from the read set
 *
 * =============================================================================
 *
 * Example Usage:
 *
 *     MAIN(argc,argv)
 *     {
 *         TM_STARTUP(8);
 *         // create 8 threads and go parallel
 *         TM_SHUTDOWN();
 *     }
 *
 *     void parallel_region ()
 *     {
 *         TM_THREAD_ENTER();
 *         subfunction1(TM_ARG_ALONE);
 *         subfunction2(TM_ARG  1, 2, 3);
 *         TM_THREAD_EXIT();
 *     }
 *
 *     void subfunction1 (TM_ARGDECL_ALONE)
 *     {
 *         TM_BEGIN_RO()
 *         // ... do work that only reads shared data ...
 *         TM_END()
 *
 *         long* array = (long*)P_MALLOC(10 * sizeof(long));
 *         // ... do work ...
 *         P_FREE(array);
 *     }
 *
 *     void subfunction2 (TM_ARGDECL  long a, long b, long c)
 *     {
 *         TM_BEGIN();
 *         long* array = (long*)TM_MALLOC(a * b * c * sizeof(long));
 *         // ... do work that may read or write shared data ...
 *         TM_FREE(array);
 *         TM_END();
 *     }
 *
 * =============================================================================
 */


/* =============================================================================
 * HTM - Hardware Transactional Memory
 * =============================================================================
 */

#ifdef HTM

#  ifndef SIMULATOR
#    error HTM requries SIMULATOR
#  endif

#  include <assert.h>
#  include <tmapi.h>
#  include "memory.h"
#  include "thread.h"
#  include "types.h"

#  define TM_ARG                        /* nothing */
#  define TM_ARG_ALONE                  /* nothing */
#  define TM_ARGDECL                    /* nothing */
#  define TM_ARGDECL_ALONE              /* nothing */
#  define TM_CALLABLE                   /* nothing */

#  define TM_STARTUP(numThread)         /* nothing */
#  define TM_SHUTDOWN()                 /* nothing */

#  define TM_THREAD_ENTER()             /* nothing */
#  define TM_THREAD_EXIT()              /* nothing */

#  define P_MALLOC(size)                memory_get(thread_getId(), size)
#  define P_FREE(ptr)                   /* TODO: thread local free is non-trivial */
#  define TM_MALLOC(size)               memory_get(thread_getId(), size)
#  define TM_FREE(ptr)                  /* TODO: thread local free is non-trivial */

#  ifdef OTM

#    define thread_getId()              omp_get_thread_num()
#    define thread_getNumThread()       omp_get_num_threads()
#    define thread_startup(numThread)   omp_set_num_threads(numThread)
#    define thread_shutdown()           /* nothing */
#    define thread_barrier_wait();      _Pragma ("omp barrier")
#    define TM_BEGIN()                  _Pragma ("omp transaction") {
#    define TM_BEGIN_RO()               _Pragma ("omp transaction") {
#    define TM_END()                    }
#    define TM_RESTART()                _TM_Abort()

#    define TM_EARLY_RELEASE(var)       TM_Release(&(var))

#  else /* !OTM */

#    define TM_BEGIN()                    TM_BeginClosed()
#    define TM_BEGIN_RO()                 TM_BeginClosed()
#    define TM_END()                      TM_EndClosed()
#    define TM_RESTART()                  _TM_Abort()
#    define TM_EARLY_RELEASE(var)         TM_Release(&(var))

#  endif /* !OTM */

#elif defined(HTM_IBM)
#include "htm_ibm.h"

#  define TM_ARG                        /* nothing */
#  define TM_ARG_ALONE                  /* nothing */
#  define TM_ARGDECL                    /* nothing */
#  define TM_ARGDECL_ALONE              /* nothing */
#  define TM_CALLABLE                   /* nothing */

#  define TM_STARTUP(numThread)         tm_startup_ibm()
#  define TM_SHUTDOWN()                 tm_shutdown_ibm()

#  define TM_THREAD_ENTER()             tm_thread_enter_ibm()
#  define TM_THREAD_EXIT()              tm_thread_exit_ibm()

#ifdef USE_TLH
#include "thread.h"
#include "memory.h"
#    define P_MALLOC(size)              memory_get(thread_getId(), size)
#    define P_FREE(ptr)                 /* TODO: thread local free is non-trivial */
#    define TM_MALLOC(size)             memory_get(thread_getId(), size)
#    define TM_FREE(ptr)                /* TODO: thread local free is non-trivial */
#else /* !USE_TLH */
#    define P_MALLOC(size)              malloc(size)
#    define P_FREE(ptr)                 free(ptr)
#    define TM_MALLOC(size)             malloc(size)
#    define TM_FREE(ptr)                if(!getenv("PREFETCHING") || thread_getId()%2==0) free(ptr)
#endif /* !USE_TLH */

#ifdef __bgq__
#    define TM_BEGIN()                    _Pragma("tm_atomic")	\
  {
#    define TM_BEGIN_ID(id)               TM_BEGIN()
#    define TM_BEGIN_RO()                 TM_BEGIN()
#    define TM_END()                      }
#    define TM_RESTART()                  write(1, "", 0)
#    define TM_EARLY_RELEASE(var)         /* nothing */
#else /* ! __bgq__ */
#define CONTINUE 1
#    define TM_BEGIN()                    if(tbegin_ibm(0)) goto tm_end0;
#    define TM_BEGIN_ID(id)               if(tbegin_ibm(id)) goto tm_end ## id;
#    define TM_BEGIN_RO()                 if(tbegin_ibm()) goto tm_end;
#    define TM_END()                      tend_ibm();  \
tm_end0:
#    define TM_END_ID(id)                      tend_ibm();  \
tm_end ## id:
#    define TM_RESTART()                  tabort_ibm()
#    define TM_EARLY_RELEASE(var)         /* nothing */
#endif /* ! __bgq__ */

/* Hardware Lock Elision */
#elif defined(HLE_INTEL)
#include "hle_intel.h"

#  define TM_ARG                        /* nothing */
#  define TM_ARG_ALONE                  /* nothing */
#  define TM_ARGDECL                    /* nothing */
#  define TM_ARGDECL_ALONE              /* nothing */
#  define TM_CALLABLE                   /* nothing */

#  define TM_STARTUP(numThread)         tm_startup_hle()
#  define TM_SHUTDOWN()                 tm_shutdown_hle()

#  define TM_THREAD_ENTER()             tm_thread_enter_hle()
#  define TM_THREAD_EXIT()              tm_thread_exit_hle()

#ifdef USE_TLH
#include "thread.h"
#include "memory.h"
#    define P_MALLOC(size)              memory_get(thread_getId(), size)
#    define P_FREE(ptr)                 /* TODO: thread local free is non-trivial */
#    define TM_MALLOC(size)             memory_get(thread_getId(), size)
#    define TM_FREE(ptr)                /* TODO: thread local free is non-trivial */
#else /* !USE_TLH */
#    define P_MALLOC(size)              malloc(size)
#    define P_FREE(ptr)                 free(ptr)
#    define TM_MALLOC(size)             malloc(size)
#    define TM_FREE(ptr)                free(ptr)
#endif /* !USE_TLH */

#    define TM_BEGIN()                    tbegin_hle(0)
#    define TM_BEGIN_ID(id)               tbegin_hle(id)
#    define TM_BEGIN_RO()                 tbegin_hle()
#    define TM_END()                      tend_hle()
#    define TM_END_ID(id)                   tend_hle()
#    define TM_RESTART()                  tabort_hle()
#    define TM_EARLY_RELEASE(var)         /* nothing */

/* Restricted Transactional Memory */
#elif defined(RTM_INTEL)
//#include "rtm_intel.h"
#include <immintrin.h>
//#include <rtmintrin.h>
#include <stdlib.h>   //Include pra usar mutex
#include "thread.h"   //Include pra usar mutex

#  define TM_ARG                        /* nothing */
#  define TM_ARG_ALONE                  /* nothing */
#  define TM_ARGDECL                    /* nothing */
#  define TM_ARGDECL_ALONE              /* nothing */
#  define TM_CALLABLE                   /* nothing */

#  define TM_STARTUP(numThread)         // 
#  define TM_SHUTDOWN()                 // nothing

#  define TM_THREAD_ENTER()             // nothing
#  define TM_THREAD_EXIT()              // nothing

#ifdef USE_TLH
#include "thread.h"
#include "memory.h"
#    define P_MALLOC(size)              memory_get(thread_getId(), size)
#    define P_FREE(ptr)                 /* TODO: thread local free is non-trivial */
#    define TM_MALLOC(size)             memory_get(thread_getId(), size)
#    define TM_FREE(ptr)                /* TODO: thread local free is non-trivial */
#else /* !USE_TLH */
#    define P_MALLOC(size)              malloc(size)
#    define P_FREE(ptr)                 free(ptr)
#    define TM_MALLOC(size)             malloc(size)
#    define TM_FREE(ptr)                free(ptr)
#endif /* !USE_TLH */

//É interessante depois melhorar esse código com coisas como impressão do numero de commits e aborts, versão com lock sem mutex, e outras coisas assim que são dadas em alguns outros códigos
#if defined(GLOBAL_LOCK)
//THREAD_MUTEX_T global_lock;
extern THREAD_MUTEX_T global_lock;
#include <stdlib.h>
#include "thread.h"

  #ifdef FALLBACK_1
  #    define TM_BEGIN()                    { int tries = 1; int execs=1, aborts=0; int status = _xbegin(); if ((status = _xbegin ()) == _XBEGIN_STARTED){ printf("Entrou\n"); if(THREAD_MUTEX_TRYLOCK(global_lock) == 0){ THREAD_MUTEX_UNLOCK(global_lock); }else{ _xabort(30); } }else{ tries--; aborts++; THREAD_MUTEX_LOCK(global_lock); }
  //#    define TM_END()                      if(tries > 0){ _xend(); printf("Commitou transacionalmente\n"); }else{ THREAD_MUTEX_UNLOCK(global_lock); } printf("Resultados:\nExecuções: %d  Aborts: %d\n",execs,aborts); }
  #    define TM_END()                      if(tries > 0){ _xend(); printf("Commitou transacionalmente\n"); }else{ THREAD_MUTEX_UNLOCK(global_lock); } }
  #elif defined(FALLBACK_2)
  #    define HTM_RETRIES                   5
  #    define TM_BEGIN()                    { int tries = HTM_RETRIES; int execs = 0, aborts = 0; while(1){ execs++; int status = _xbegin(); if ((status = _xbegin ()) == _XBEGIN_STARTED){ printf("Entrou\n"); if(THREAD_MUTEX_TRYLOCK(global_lock) == 0){ THREAD_MUTEX_UNLOCK(global_lock); break; }else{ _xabort(30); } }else{ tries--; aborts++; if(tries <= 0){ THREAD_MUTEX_LOCK(global_lock); break; } } }
  //#    define TM_END()                      if(tries > 0){ _xend(); printf("Commitou transacionalmente\n"); }else{ THREAD_MUTEX_UNLOCK(global_lock); } printf("Resultados:\nExecuções: %d  Aborts: %d\n",execs,aborts); }
  #    define TM_END()                      if(tries > 0){ _xend(); printf("Commitou transacionalmente\n"); }else{ THREAD_MUTEX_UNLOCK(global_lock); }  }
  #endif
//O caso sem global lock é utilizado para testar o funcionamento apropriado da TSX (não estar abortando sempre)
#else
  #ifdef FALLBACK_1
  #    define TM_BEGIN()                    { int aborts=0, cap=0, conf=0; do{ unsigned int status = _xbegin(); if (status == _XBEGIN_STARTED) { printf("Entrou\nAborts: %d\nCausa - Tamanho: %d  Conflito: %d\n",aborts,cap,conf); break; }else{ if(status == _XABORT_CAPACITY){ cap++; }else if(status == _XABORT_CONFLICT){ conf++; }  if(status == _XABORT_RETRY){ printf("--Nao funciona retry--\n"); } aborts++; }}while(1)
  //#    define TM_BEGIN()                    { int aborts=0, enter=0, cap=0, conf=0; do{ printf("Tenta - Entrou: %d  Aborts: %d\nCausas - Capacidade: %d  Conflito: %d\n",enter,aborts,cap,conf); unsigned int status = _xbegin(); if (status == _XBEGIN_STARTED) { printf("Entrou\n"); break; }else{ if(status == _XABORT_CAPACITY){ cap++; }  if(status == _XABORT_RETRY){ printf("--Nao funciona retry--\n"); } aborts++; }}while(1)
  #    define TM_END()                      _xend (); printf("Saiu com %d aborts\n",aborts); };
  #else
  #    define TM_BEGIN()                    if ((_xbegin ()) == _XBEGIN_STARTED) { printf("Entrou\n")
  #    define TM_END()                      _xend (); printf("Saiu\n"); } else { printf("Fallback\n"); }
  #endif
#endif

#    define TM_BEGIN_ID(id)               TM_BEGIN()
#    define TM_BEGIN_RO()                 TM_BEGIN()
#    define TM_END_ID(id)                 TM_END()
#    define TM_RESTART()                  _xabort(0xab);
#    define TM_EARLY_RELEASE(var)         /* nothing */

/// Copiando o sequencial pra ver como a biblioteca lida com mutex e outras alternativas bloqueantes
/// Estou cogitando deixar só com mutex, e ver se funciona pra n ficar com um trecho de código monumental
// #ifdef USE_MUTEX
// #  define TM_BEGIN()     do{ THREAD_MUTEX_LOCK(global_lock); }while(0)
// #  define TM_END()       do{ THREAD_MUTEX_UNLOCK(global_lock);	}while(0)
// #else /* ! USE_MUTEX */
// #  include "mfence.h"
// #  if defined(__370__)
// #    define TM_BEGIN()   do{ cs_t local_value = 0; cs_t new_value = 1; while(cs(&local_value, (cs_t *)&global_lock, new_value)){ TM_BEGIN() while (local_value = global_lock); } }while(0)
// #  elif defined(__GNUC__) || defined(__IBMC__)
// #    define TM_BEGIN()   do{ while(__sync_val_compare_and_swap(&global_lock, 0, 1)){ while(global_lock); } }while(0)
// #  else
// #    error
// #  endif
// #  define TM_END()       do{ memory_fence(); global_lock = 0; }while(0)
// #endif /* USE_MUTEX */
/// Fim do trecho de código sequencial

/* =============================================================================
 * STM - Software Transactional Memory
 * =============================================================================
 */

#elif defined(STM)

#  include <string.h>
#  include <stm.h>
#  include "thread.h"

#  if defined (OTM)

#    define TM_ARG                        /* nothing */
#    define TM_ARG_ALONE                  /* nothing */
#    define TM_ARGDECL                    /* nothing */
#    define TM_ARGDECL_ALONE              /* nothing */
#    define TM_CALLABLE                   _Pragma ("omp tm_function")

#    define thread_getId()                omp_get_thread_num()
#    define thread_getNumThread()         omp_get_num_threads()
#    define thread_startup(numThread)     omp_set_num_threads(numThread)
#    define thread_shutdown()             /* nothing */

#  else /* !OTM */

#    define TM_ARG                        STM_SELF,
#    define TM_ARG_ALONE                  STM_SELF
#    define TM_ARGDECL                    STM_THREAD_T* TM_ARG
#    define TM_ARGDECL_ALONE              STM_THREAD_T* TM_ARG_ALONE
#    define TM_CALLABLE                   /* nothing */

#endif /* !OTM */

#  ifdef SIMULATOR

#    ifdef OTM

#      define TM_STARTUP(numThread)       STM_STARTUP(); \
                                          STM_NEW_THREADS(numThread)
#      define TM_SHUTDOWN()               STM_SHUTDOWN()

#      define TM_THREAD_ENTER()           omp_set_self()
#      define TM_THREAD_EXIT()            /* Nothing */
#      define thread_barrier_wait();      _Pragma ("omp barrier")

#      define P_MALLOC(size)              memory_get(thread_getId(), size)
#      define P_FREE(ptr)                 /* TODO: thread local free is non-trivial */
#      define TM_MALLOC(size)             memory_get(thread_getId(), size)
#      define TM_FREE(ptr)                /* TODO: thread local free is non-trivial */

#    else /* !OTM */

#      define TM_STARTUP(numThread)       STM_STARTUP(); \
                                          STM_NEW_THREADS(numThread)
#      define TM_SHUTDOWN()               STM_SHUTDOWN()

#      define TM_THREAD_ENTER()           TM_ARGDECL_ALONE = \
                                              STM_GET_THREAD(thread_getId()); \
                                          STM_SET_SELF(TM_ARG_ALONE)

#      define TM_THREAD_EXIT()            STM_FREE_THREAD(TM_ARG_ALONE)

#      define P_MALLOC(size)              memory_get(thread_getId(), size)
#      define P_FREE(ptr)                 /* TODO: thread local free is non-trivial */
#      define TM_MALLOC(size)             memory_get(thread_getId(), size)
#      define TM_FREE(ptr)                /* TODO: thread local free is non-trivial */

#    endif /* !OTM */

#  else /* !SIMULATOR */

#    ifdef OTM

#      include <omp.h>
#      include "tl2.h"

#      define TM_STARTUP(numThread)     STM_STARTUP()
#      define TM_SHUTDOWN()             STM_SHUTDOWN()

#      define TM_THREAD_ENTER()         /* nothing */
#      define TM_THREAD_EXIT()          /* nothing */
#      define thread_barrier_wait();    _Pragma ("omp barrier")

#      define P_MALLOC(size)            malloc(size)
#      define P_FREE(ptr)               free(ptr)
#      define TM_MALLOC(size)           malloc(size)
#      define TM_FREE(ptr)              /* TODO: fix memory free problem with OpenTM */

#    else /* !OTM */

#      define TM_STARTUP(numThread)     STM_STARTUP()
#      define TM_SHUTDOWN()             STM_SHUTDOWN()

#      define TM_THREAD_ENTER()         TM_ARGDECL_ALONE = STM_NEW_THREAD(); \
                                        STM_INIT_THREAD(TM_ARG_ALONE, thread_getId())
#      define TM_THREAD_EXIT()          STM_FREE_THREAD(TM_ARG_ALONE)

#      define P_MALLOC(size)            malloc(size)
#      define P_FREE(ptr)               free(ptr)
#      define TM_MALLOC(size)           STM_MALLOC(size)
#      define TM_FREE(ptr)              STM_FREE(ptr)

#    endif /* !OTM */

#  endif /* !SIMULATOR */

#  ifdef OTM

#    define TM_BEGIN()                  _Pragma ("omp transaction") {
#    define TM_BEGIN_RO()               _Pragma ("omp transaction") {
#    define TM_END()                    }
#    define TM_RESTART()                omp_abort()

#    define TM_EARLY_RELEASE(var)       /* nothing */

#  else /* !OTM */

#    define TM_BEGIN()                  STM_BEGIN_WR()
#    define TM_BEGIN_RO()               STM_BEGIN_RD()
#    define TM_END()                    STM_END()
#    define TM_RESTART()                STM_RESTART()

#    define TM_EARLY_RELEASE(var)       /* nothing */

#  endif /* !OTM */

#    define TM_BEGIN_ID(id)               TM_BEGIN()
#    define TM_END_ID(id)                 TM_END()


/* =============================================================================
 * Sequential execution
 * =============================================================================
 */

#else /* SEQUENTIAL */

#  include <assert.h>

#  define TM_ARG                        /* nothing */
#  define TM_ARG_ALONE                  /* nothing */
#  define TM_ARGDECL                    /* nothing */
#  define TM_ARGDECL_ALONE              /* nothing */
#  define TM_CALLABLE                   /* nothing */

#  define TM_STARTUP(numThread)         /* nothing */
#  define TM_SHUTDOWN()                 /* nothing */

#  define TM_THREAD_ENTER()             /* nothing */
#  define TM_THREAD_EXIT()              /* nothing */

#  ifdef SIMULATOR

#    include "thread.h"

#    define P_MALLOC(size)              memory_get(thread_getId(), size)
#    define P_FREE(ptr)                 /* TODO: thread local free is non-trivial */
#    define TM_MALLOC(size)             memory_get(thread_getId(), size)
#    define TM_FREE(ptr)                /* TODO: thread local free is non-trivial */

#  else /* !SIMULATOR */

#ifdef USE_TLH
#include "thread.h"
#include "memory.h"
#    define P_MALLOC(size)              memory_get(thread_getId(), size)
#    define P_FREE(ptr)                 /* TODO: thread local free is non-trivial */
#    define TM_MALLOC(size)             memory_get(thread_getId(), size)
#    define TM_FREE(ptr)                /* TODO: thread local free is non-trivial */
#else /* !USE_TLH */
#    define P_MALLOC(size)              malloc(size)
#    define P_FREE(ptr)                 free(ptr)
#    define TM_MALLOC(size)             malloc(size)
#    define TM_FREE(ptr)                free(ptr)
#endif /* !USE_TLH */

#  endif /* !SIMULATOR */

#if defined(GLOBAL_LOCK)
#include <stdlib.h>
#include "thread.h"

#ifdef USE_MUTEX
#  define TM_BEGIN()			     \
  do {					     \
    THREAD_MUTEX_LOCK(global_lock);	     \
  } while (0)
#  define TM_END()			     \
  do {					     \
    THREAD_MUTEX_UNLOCK(global_lock);	     \
  } while (0)
#else /* ! USE_MUTEX */
#include "mfence.h"
#if defined(__370__)
#  define TM_BEGIN()						\
  do {								\
  cs_t local_value = 0; cs_t new_value = 1;			\
  while (cs(&local_value, (cs_t *)&global_lock, new_value)) {	\
    while (local_value = global_lock)				\
      ;								\
  }								\
  } while (0)
#elif defined(__GNUC__) || defined(__IBMC__)
#  define TM_BEGIN()						\
  do {								\
    while (__sync_val_compare_and_swap(&global_lock, 0, 1)) {	\
      while (global_lock)					\
	;							\
    }								\
  } while (0)
#else
#error
#endif
#  define TM_END()			     \
  do {					     \
    memory_fence();			     \
    global_lock = 0;			     \
  } while (0)
#endif /* USE_MUTEX */
#  define TM_BEGIN_ID(id) TM_BEGIN()
#  define TM_BEGIN_RO() TM_BEGIN()
#  define TM_RESTART()                  assert(0)
#  define TM_EARLY_RELEASE(var)         /* nothing */

#else /* !GLOBAL_LOCK */

#  define TM_BEGIN()                    /* nothing */
#  define TM_BEGIN_ID(id) TM_BEGIN()
#  define TM_BEGIN_RO()                 /* nothing */
#  define TM_END()                      /* nothing */
#  define TM_RESTART()                  assert(0)

#  define TM_EARLY_RELEASE(var)         /* nothing */
#endif /* GLOBAL_LOCK */

#endif /* SEQUENTIAL */


/* =============================================================================
 * Transactional Memory System interface for shared memory accesses
 *
 * There are 3 flavors of each function:
 *
 * 1) no suffix: for accessing variables of size "long"
 * 2) _P suffix: for accessing variables of type "pointer"
 * 3) _F suffix: for accessing variables of type "float"
 * =============================================================================
 */
#if defined(STM)

#if defined(OTM)

#  define TM_SHARED_READ(var)           (var)
#  define TM_SHARED_READ_P(var)         (var)
#  define TM_SHARED_READ_F(var)         (var)

#  define TM_SHARED_WRITE(var, val)     ({var = val; var;})
#  define TM_SHARED_WRITE_P(var, val)   ({var = val; var;})
#  define TM_SHARED_WRITE_F(var, val)   ({var = val; var;})

#  define TM_LOCAL_WRITE(var, val)      ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_F(var, val)    ({var = val; var;})

#else /* OTM */

#  define TM_SHARED_READ(var)           STM_READ(var)
#  define TM_SHARED_READ_P(var)         STM_READ_P(var)
#  define TM_SHARED_READ_F(var)         STM_READ_F(var)

#  define TM_SHARED_WRITE(var, val)     STM_WRITE((var), val)
#  define TM_SHARED_WRITE_P(var, val)   STM_WRITE_P((var), val)
#  define TM_SHARED_WRITE_F(var, val)   STM_WRITE_F((var), val)

#  define TM_LOCAL_WRITE(var, val)      STM_LOCAL_WRITE(var, val)
#  define TM_LOCAL_WRITE_P(var, val)    STM_LOCAL_WRITE_P(var, val)
#  define TM_LOCAL_WRITE_F(var, val)    STM_LOCAL_WRITE_F(var, val)

#endif /* !OTM */

#else /* !STM */

#ifdef HTM_CONSERVE_RWBUF
static inline void resume_tx() {
  asm volatile (".long 0x7c2005dd":::"cr0","memory");
}

static inline void suspend_tx() {
  asm volatile (".long 0x7c0005dd":::"cr0","memory");
}

static inline long resume_read_long(volatile long *varp) {
  long res;
  resume_tx();
  res=*varp;
  suspend_tx();
  return res;
}

static inline void *resume_read_ptr(volatile void **varp) {
  void *res;
  resume_tx();
  res=*varp;
  suspend_tx();
  return res;
}

static inline float resume_read_float(volatile float *varp) {
  float res;
  resume_tx();
  res=*varp;
  suspend_tx();
  return res;
}

static inline long resume_write_long(volatile long *varp, long val) {
  resume_tx();
  *varp=val;
  suspend_tx();
  return val;
}

static inline void *resume_write_ptr(volatile void **varp,void *val) {
  resume_tx();
  *varp=val;
  suspend_tx();
  return val;
}

static inline float resume_write_float(volatile float *varp,float val) {
  resume_tx();
  *varp=val;
  suspend_tx();
  return val;
}

#  define TM_SHARED_READ(var)           (resume_read_long(&(var)))
#  define TM_SHARED_READ_P(var)         (resume_read_ptr(&(var)))
#  define TM_SHARED_READ_F(var)         (resume_read_float(&(var)))

#  define TM_SHARED_WRITE(var, val)     (resume_write_long(&(var),val))
#  define TM_SHARED_WRITE_P(var, val)   (resume_write_ptr(&(var),val))
#  define TM_SHARED_WRITE_F(var, val)   (resume_write_float(&(var),val))

#  define TM_LOCAL_WRITE(var, val)      ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_F(var, val)    ({var = val; var;})

#else /* HTM_CONSERVE_RWBUF */

#  define TM_SHARED_READ(var)           (var)
#  define TM_SHARED_READ_P(var)         (var)
#  define TM_SHARED_READ_F(var)         (var)

#  define TM_SHARED_WRITE(var, val)     ({ if(!getenv("PREFETCHING") || thread_getId()%2==0) var = val; var;})
#  define TM_SHARED_WRITE_P(var, val)   ({ if(!getenv("PREFETCHING") || thread_getId()%2==0) var = val; var;})
#  define TM_SHARED_WRITE_F(var, val)   ({ if(!getenv("PREFETCHING") || thread_getId()%2==0) var = val; var;})

#  define TM_LOCAL_WRITE(var, val)      ({var = val; var;})
#  define TM_LOCAL_WRITE_P(var, val)    ({var = val; var;})
#  define TM_LOCAL_WRITE_F(var, val)    ({var = val; var;})

#endif /* !HTM_CONSERVE_RWBUF */

#endif /* !STM */


#endif /* TM_H */


/* =============================================================================
 *
 * End of tm.h
 *
 * =============================================================================
 */
