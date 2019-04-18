/*
    foidl_work.c
    Concurrency functions

    Copyright Frank V. Castellucci
    All Rights Reserved
*/

#define WORK_IMPL
#include <foidlrt.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <time.h>
#include <stdio.h>

#define NANO_SECOND_MULTIPLIER  1000000
/*
    Sleeps for timeout milliseconds
*/

PFRTAny foidl_nap_bang(PFRTAny timeout) {
    PFRTAny res = nil;
    if(timeout->fclass == scalar_class &&
        timeout->ftype == number_type) {
        ft  tmms = number_toft(timeout);
        #ifdef _MSC_VER
        Sleep(number_toft(tmms);
        res = zero;
        #else
        struct timespec req;
        if(tmms > 999) {
            req.tv_sec = (int)(tmms / 1000);                            /* Must be Non-Negative */
            req.tv_nsec = (tmms - ((long)req.tv_sec * 1000)) * NANO_SECOND_MULTIPLIER;
        }
        else {
            req.tv_sec = 0;
            req.tv_nsec = tmms * NANO_SECOND_MULTIPLIER;
        }
        int ires = nanosleep(&req , NULL);
        if(ires == 0) {
            res = zero;
        }
        else {
            res = foidl_reg_intnum(ires);
        }
        #endif
    }
    else {
        unknown_handler();
    }
    return res;
}

/*
    worker0
    Invokes function with no arguments
*/
#ifdef _MSC_VER
static DWORD WINAPI worker0(void* arg)
#else
static void *worker0(void *arg)
#endif
{
    PFRTWorker wrk = (PFRTWorker) arg;
    PFRTAny res = dispatch0(wrk->fnptr);
    wrk->result = res;
#ifdef _MSC_VER
    return 0;
#else
    pthread_exit((void *) res);
    return NULL;
#endif
}

static PFRTAny spawn_worker0(PFRTWorker wrk) {
#ifdef _MSC_VER
    HANDLE tid = CreateThread(NULL,0,worker0, wrk, 0, NULL);
    wrk->thread_id = tid;
#else
    pthread_create(&wrk->thread_id, NULL, worker0, wrk);
#endif
    return (PFRTAny) wrk;
}

/*
    worker
    Invokes function with arbitrary arguments
*/

#ifdef _MSC_VER
static DWORD WINAPI worker(void* arg)
#else
static void *worker(void *arg)
#endif
{
    PFRTWorker wrk = (PFRTWorker) arg;
    PFRTFuncRef2 iref = (PFRTFuncRef2) wrk->fnptr;
    PFRTIterator itr = iteratorFor(wrk->vector_args);
    PFRTAny res = (PFRTAny) iref;
    while(res == (PFRTAny) iref) {
        PFRTAny iNext = iteratorNext(itr);
        if(iNext != end) {
            res = foidl_imbue((PFRTAny) iref,iNext);
        }
        else {
            unknown_handler();
        }
    }
    wrk->result = res;
#ifdef _MSC_VER
    return 0;
#else
    pthread_exit((void *) res);
    return NULL;
#endif
}

static PFRTAny spawn_worker(PFRTWorker wrk) {
#ifdef _MSC_VER
    HANDLE tid = CreateThread(NULL,0,worker, wrk, 0, NULL);
    wrk->thread_id = tid;
#else
    pthread_create(&wrk->thread_id, NULL, worker, wrk);
#endif
    return (PFRTAny) wrk;
}

///////////////////////////////////////////////////////////////////////////////
//                      Threads
///////////////////////////////////////////////////////////////////////////////

/*
    foidl_thread
    Entry point for spawning a worker with arguments
*/

PFRTAny foidl_thread_bang(PFRTAny funcref, PFRTAny argvector) {
    PFRTWorker wrk = allocWorker((PFRTFuncRef2)funcref);
    wrk->vector_args = argvector;
    return spawn_worker(wrk);
}

/*
    foidl_thread0
    Entry point for spawning a worker with no arguments
*/
PFRTAny foidl_thread0_bang(PFRTAny funcref) {
    PFRTWorker wrk = allocWorker((PFRTFuncRef2)funcref);
    return spawn_worker0(wrk);
}

/*
    foidl_wait
    Blocks for worker indefinitly
*/

PFRTAny foidl_wait_bang(PFRTAny thrdref) {
    PFRTAny res = nil;
    if(thrdref->fclass == function_class &&
        thrdref->ftype == worker_type) {
        PFRTWorker wrk = (PFRTWorker) thrdref;
#ifdef _MSC_VER
        WaitForSingleObject(wrk->thread_id, INFINITE);
        res = wrk->result;
#else
        pthread_join(wrk->thread_id, (void *) &res);
#endif
    }
    else {
        unknown_handler();
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////
//                      OTHER?
///////////////////////////////////////////////////////////////////////////////

