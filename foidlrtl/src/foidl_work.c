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
#include <stdio.h>

/*
    Sleeps for timeout seconds
*/

PFRTAny foidl_nap(PFRTAny timeout) {
    PFRTAny res = nil;
    if(timeout->fclass == scalar_class &&
        timeout->ftype == number_type) {
        #ifdef _MSC_VER
        Sleep(number_toft(timeout)*1000);
        res = zero;
        #else
        ft timo = number_toft(timeout);
        unsigned int ires = sleep(timo);
        if(ires == 0)
            res = zero;
        else
            res = foidl_reg_intnum(ires);
        #endif
    }
    else {
        unknown_handler();
    }
    return res;
}

void *worker0(void *arg) {
    PFRTWorker wrk = (PFRTWorker) arg;
    PFRTAny res = dispatch0(wrk->fnptr);
    wrk->result = res;
#ifdef _MSC_VER
#else
    pthread_exit((void *) res);
#endif
    return NULL;
}

#ifdef _MSC_VER
DWORD WINAPI wworker0(void* arg) {
    PFRTWorker wrk = (PFRTWorker) arg;
    PFRTAny res = dispatch0(wrk->fnptr);
    wrk->result = res;
  return 0;
}
#endif

PFRTAny foidl_thread(PFRTAny funcref, PFRTAny arglist) {
    PFRTWorker wrk = allocWorker((PFRTFuncRef2)funcref);

    return (PFRTAny) wrk;
}

PFRTAny foidl_thread0(PFRTAny funcref) {
    PFRTWorker wrk = allocWorker((PFRTFuncRef2)funcref);
#ifdef _MSC_VER
    HANDLE tid = CreateThread(NULL,0,wworker0, wrk, 0, NULL);
    wrk->thread_id = tid;
#else
    pthread_create(&wrk->thread_id, NULL, worker0, wrk);
#endif
    return (PFRTAny) wrk;
}

PFRTAny foidl_wait(PFRTAny thrdref) {
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