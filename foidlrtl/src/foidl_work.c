/*
    foidl_work.c
    Concurrency functions

    Copyright Frank V. Castellucci
    All Rights Reserved
*/

#define WORK_IMPL
#include <foidlrt.h>
#include <unistd.h>
#include <stdio.h>

/*
    Sleeps for timeout seconds
*/

PFRTAny foidl_nap(PFRTAny timeout) {
    PFRTAny res = nil;
    if(timeout->fclass == scalar_class &&
        timeout->ftype == number_type) {
        ft timo = number_toft(timeout);
        unsigned int ires = sleep(timo);
        if(ires == 0)
            res = zero;
        else
            res = foidl_reg_intnum(ires);
    }
    else {
        unknown_handler();
    }
    return res;
}

void *worker0(void *arg) {
    PFRTWorker wrk = (PFRTWorker) arg;
    PFRTAny res = dispatch0(wrk->fnptr);
    pthread_exit((void *) res);
    return NULL;
}

PFRTAny foidl_thread(PFRTAny funcref, PFRTAny arglist) {
    PFRTWorker wrk = allocWorker((PFRTFuncRef2)funcref);

    return (PFRTAny) wrk;
}

PFRTAny foidl_thread0(PFRTAny funcref) {
    PFRTWorker wrk = allocWorker((PFRTFuncRef2)funcref);
    pthread_create(&wrk->thread_id, NULL, worker0, wrk);
    return (PFRTAny) wrk;
}

PFRTAny foidl_wait(PFRTAny thrdref) {
    PFRTAny res = nil;
    if(thrdref->fclass == function_class &&
        thrdref->ftype == worker_type) {
        PFRTWorker wrk = (PFRTWorker) thrdref;
        pthread_join(wrk->thread_id, (void *) &res);
    }
    else {
        unknown_handler();
    }
    return res;
}