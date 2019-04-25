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
    wrk->thread_id = CreateThread(NULL,0,worker, wrk, 0, NULL);
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

    PFRTWorker wrk = allocWorker((PFRTFuncRef2)foidl_fref_instance(funcref));
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

///////////////////////////////////////////////////////////////////////////////
//                      Thread Pool
///////////////////////////////////////////////////////////////////////////////

static PFRTAny wait_for_work(PFRTThreadPool poolref) {
    PFRTAny res = nil;
    pthread_mutex_lock(&poolref->run_mutex);
    if(poolref->work_list->count == 0) {
        //printf("Wait: queue has no work\n");
        while(!poolref->work_list->count) {
            pthread_cond_wait(&poolref->run_condition, &poolref->run_mutex);
        }
        res = list_first(poolref->work_list);
        if(res == end) {
            printf("Last is end\n");
        }
        else {
            list_pop_bang(poolref->work_list);
        }
    }
    else {
        //printf("Nowait: queue has work\n");
        res = list_first(poolref->work_list);
        if(res == end) {
            printf("Last is end\n");
        }
        else {
            list_pop_bang(poolref->work_list);
        }
    }
    pthread_mutex_unlock(&poolref->run_mutex);
    return res;
}

static void run_post(PFRTThreadPool poolref) {
    pthread_cond_signal(&poolref->run_condition);
}

static void run_broadcast(PFRTThreadPool poolref) {
    pthread_cond_broadcast(&poolref->run_condition);
}

static void  push_task(PFRTThreadPool poolref, PFRTAny wrkref) {
    pthread_mutex_lock(&poolref->run_mutex);
    foidl_list_extend_bang(poolref->work_list,wrkref);
    run_post(poolref);
    pthread_mutex_unlock(&poolref->run_mutex);
    return;
}


#ifdef _MSC_VER
static DWORD WINAPI pool_worker(void* arg)
#else
static void *pool_worker(void *arg)
#endif
{
    PFRTThread  pthrd = (PFRTThread) arg;
    PFRTThreadPool poolref = (PFRTThreadPool) pthrd->pool_parent;
    pthread_mutex_lock(&poolref->pool_mutex);
    poolref->active_threads++;
    pthread_mutex_unlock(&poolref->pool_mutex);
    for(;;) {
        // Wait for work
        PFRTAny ptsk = wait_for_work(poolref);
        // Get worker and check for end
        if(ptsk == end) {
            printf("End of queue found\n");
            break;
        }
        else if(ptsk == nil) {
            printf("No queue value\n");
        }
        PFRTWorker wrk = (PFRTWorker) ptsk;
        PFRTFuncRef2 iref = (PFRTFuncRef2) wrk->fnptr;
        PFRTAny res = (PFRTAny) iref;
        wrk->work_state = wrk_run;
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
                    printf("iNext == end\n");
                    unknown_handler();
                }
            }
        }
        wrk->result = res;
        wrk->work_state = wrk_complete;
    }
    printf("Exiting thread\n");
#ifdef _MSC_VER
    return 0;
#else
    pthread_mutex_lock(&poolref->pool_mutex);
    poolref->active_threads--;
    pthread_mutex_unlock(&poolref->pool_mutex);
    //pthread_exit((void *) poolref);
    return pthrd;
#endif
}

static PFRTThread create_pool_thread(PFRTThreadPool poolref, int id) {
    PFRTThread  pthrd = allocThread(poolref, id);
#ifdef _MSC_VER
    pthrd->thread_id =CreateThread(NULL,0,pool_worker, pthrd, 0, NULL);
#else
    pthread_create(&pthrd->thread_id, NULL, pool_worker, pthrd);
#endif
    return pthrd;
}

static PFRTThreadPool initialize_pool(PFRTThreadPool poolref) {
    // Mutex and semaphore
#ifdef _MSC_VER
    poolref->pool_mutex = CreateMutex(NULL,FALSE,NULL);
    poolref->run_mutex = CreateMutex(NULL,FALSE,NULL);
    InitializeConditionVariable(&poolref->run_condition);
#else
    pthread_mutex_init(&poolref->pool_mutex, NULL);
    pthread_mutex_lock(&poolref->pool_mutex);
    pthread_mutex_init(&poolref->run_mutex, NULL);
    pthread_cond_init(&poolref->run_condition,NULL);
#endif
    for(ft x=0; x < poolref->count; ++x) {
        PFRTThread pthrd = create_pool_thread(poolref,x);
        foidl_list_extend_bang(poolref->thread_list,(PFRTAny)pthrd);
    }
    pthread_mutex_unlock(&poolref->pool_mutex);
    while(poolref->count != poolref->active_threads) {}
    return poolref;
}

PFRTAny foidl_create_thread_pool_bang() {
    PFRTThreadPool poolref = allocThreadPool();
    poolref->count = getNumberOfCores();
    poolref->active_threads = 0;
    return (PFRTAny) initialize_pool(poolref);
}

PFRTAny foidl_queue_work_bang(PFRTAny poolref, PFRTAny wrkref) {
    push_task((PFRTThreadPool)poolref, wrkref);
    return poolref;
}

PFRTAny foidl_queue_thread_bang(PFRTAny poolref,PFRTAny funcref, PFRTAny argcoll) {
    PFRTAny wrkref = foidl_task_bang(funcref, argcoll);
    push_task((PFRTThreadPool)poolref, wrkref);
    return poolref;
}

// Pauses the workers and may block new work add
PFRTAny foidl_pause_thread_pool_bang(PFRTAny poolref, PFRTAny blockwork) {
    return nil;
}

PFRTAny foidl_resume_thread_pool_bang(PFRTAny poolref) {
    return nil;
}

PFRTAny foidl_exit_thread_pool_bang(PFRTAny poolref) {
    return nil;
}

PFRTAny foidl_kill_thread_pool_bang(PFRTAny poolref) {
    return nil;
}
