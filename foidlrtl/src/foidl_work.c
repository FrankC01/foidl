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
    #ifdef __APPLE__
        #include <sys/param.h>
        #include <sys/sysctl.h>
    #endif
#endif
#include <time.h>
#include <stdio.h>

#define NANO_SECOND_MULTIPLIER  1000000

globalScalarConst(wrk_alloc,byte_type,(void *) 0x0,1);
globalScalarConst(wrk_init,byte_type,(void *) 0x1,1);
globalScalarConst(wrk_create,byte_type,(void *) 0x2,1);
globalScalarConst(wrk_run,byte_type,(void *) 0x3,1);
globalScalarConst(wrk_complete,byte_type,(void *) 0x4,1);
globalScalarConst(not_work,byte_type,(void *) 0xF,1);


int getNumberOfCores() {
#ifdef _MSC_VER
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif __APPLE__
    int nm[2];
    size_t len = 4;
    uint32_t count;
    printf("In mac code\n");
    nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
    sysctl(nm, 2, &count, &len, NULL, 0);
    if(count < 1) {
        nm[1] = HW_NCPU;
        sysctl(nm, 2, &count, &len, NULL, 0);
        if(count < 1) { count = 1; }
    }
    return count;
#else
    return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

/*
    Gets the state of work
*/

PFRTAny foidl_work_state(PFRTAny wrkref) {
    PFRTAny res = not_work;
    if(wrkref->fclass == function_class &&
        wrkref->ftype == worker_type) {
            res = ((PFRTWorker) wrkref)->work_state;
        }
    return res;
}


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
    worker
    Invokes function with arbitrary arguments
    Worker states include
        running
        completed
*/

#ifdef _MSC_VER
static DWORD WINAPI worker(void* arg)
#else
static void *worker(void *arg)
#endif
{
    PFRTWorker wrk = (PFRTWorker) arg;
    wrk->work_state = wrk_run;
    PFRTFuncRef2 iref = (PFRTFuncRef2) wrk->fnptr;
    PFRTAny res = (PFRTAny) iref;
    if(foidl_empty_qmark(wrk->argcollection) == true) {
        res = dispatch0(wrk->fnptr);
    }
    else {
        PFRTIterator itr = iteratorFor(wrk->argcollection);
        while(res == (PFRTAny) iref) {
            PFRTAny iNext = iteratorNext(itr);
            if(iNext != end) {
                res = foidl_imbue((PFRTAny) iref,iNext);
            }
            else {
                unknown_handler();
            }
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

//  Spawn a worker
//  Worker state includes:
//      created

static PFRTAny spawn_worker(PFRTWorker wrk) {
    wrk->work_state = wrk_create;
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
    Worker states include:
        initiated
*/

PFRTAny foidl_thread_bang(PFRTAny funcref, PFRTAny argcoll) {
    PFRTWorker wrk = allocWorker((PFRTFuncRef2)funcref);
    wrk->argcollection = argcoll;
    wrk->work_state = wrk_init;
    return spawn_worker(wrk);
}

/*
    foidl_wait
    Blocks for worker indefinitly
    Worker state includes:
        completed
*/

PFRTAny foidl_wait_bang(PFRTAny thrdref) {
    PFRTAny res = nil;
    if(thrdref->fclass == function_class &&
        thrdref->ftype == worker_type) {
        PFRTWorker wrk = (PFRTWorker) thrdref;
        if(wrk->work_state != wrk_complete) {
#ifdef _MSC_VER
            WaitForSingleObject(wrk->thread_id, INFINITE);
            res = wrk->result;
#else
            pthread_join(wrk->thread_id, (void *) &res);
#endif
            wrk->work_state = wrk_complete;
        }
        else {
            res = wrk->result;
        }
    }
    else {
        unknown_handler();
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////
//                      Tasks
///////////////////////////////////////////////////////////////////////////////

//  Worker state includes:
//      initiated

PFRTAny foidl_task_bang(PFRTAny funcref, PFRTAny argcoll) {
    PFRTWorker wrk = allocWorker((PFRTFuncRef2)funcref);
    wrk->work_state = wrk_init;
    wrk->argcollection = argcoll;
    return (PFRTAny) wrk;
}

PFRTAny foidl_start_bang(PFRTAny wrkref) {
    PFRTAny res = wrkref;
    if(wrkref->fclass == function_class &&
        wrkref->ftype == worker_type &&
        ((PFRTWorker) wrkref)->work_state == wrk_init) {
        res = spawn_worker((PFRTWorker)wrkref);
    }
    return res;
}

