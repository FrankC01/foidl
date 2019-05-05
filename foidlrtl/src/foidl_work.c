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
    if(wrkref->fclass == worker_class &&
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
        Sleep(tmms);
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
    ExitThread(0);
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
    if(thrdref->fclass == worker_class &&
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
    if(wrkref->fclass == worker_class &&
        wrkref->ftype == worker_type &&
        ((PFRTWorker) wrkref)->work_state == wrk_init) {
        res = spawn_worker((PFRTWorker)wrkref);
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////////
//                      Thread Pool
///////////////////////////////////////////////////////////////////////////////

globalScalarConst(pool_running,pool_control,(void *) 0x0,1);
globalScalarConst(pool_pause,pool_control,(void *) 0x1,1);
globalScalarConst(pool_pause_block,pool_control,(void *) 0x2,1);
globalScalarConst(pool_resume,pool_control,(void *) 0x3,1);
globalScalarConst(pool_exit,pool_control,(void *) 0x4,1);

// Mutex and conditionals
static void create_pool_controls(PFRTThreadPool poolref) {
#ifdef _MSC_VER
    poolref->pool_mutex = CreateMutex(NULL,TRUE,NULL);
    InitializeCriticalSection(&poolref->run_mutex);
    InitializeConditionVariable(&poolref->run_condition);
#else
    pthread_mutex_init(&poolref->pool_mutex, NULL);
    pthread_mutex_lock(&poolref->pool_mutex);
    pthread_mutex_init(&poolref->run_mutex, NULL);
    pthread_cond_init(&poolref->run_condition,NULL);
#endif
}

static void lock_pool(PFRTThreadPool poolref) {
#ifdef _MSC_VER
    WaitForSingleObject(poolref->pool_mutex,INFINITE);
#else
    pthread_mutex_lock(&poolref->pool_mutex);
#endif
}

static void unlock_pool(PFRTThreadPool poolref) {
#ifdef _MSC_VER
    ReleaseMutex(poolref->pool_mutex);
#else
    pthread_mutex_unlock(&poolref->pool_mutex);
#endif
}

static void lock_run(PFRTThreadPool poolref) {
#ifdef _MSC_VER
    EnterCriticalSection(&poolref->run_mutex);
#else
    pthread_mutex_lock(&poolref->run_mutex);
#endif
}

static void unlock_run(PFRTThreadPool poolref) {
#ifdef _MSC_VER
    LeaveCriticalSection(&poolref->run_mutex);
#else
    pthread_mutex_unlock(&poolref->run_mutex);
#endif
}

static void wait_for_work_to_run(PFRTThreadPool poolref) {
    while(!poolref->work_list->count) {
#ifdef _MSC_VER
        SleepConditionVariableCS(&poolref->run_condition, &poolref->run_mutex,INFINITE);
#else
        pthread_cond_wait(&poolref->run_condition, &poolref->run_mutex);
#endif
    }
}

static void destroy_run(PFRTThreadPool poolref) {
#ifdef _MSC_VER
    DeleteCriticalSection(&poolref->run_condition);
    CloseHandle(poolref->run_mutex);
#else
    pthread_cond_destroy(&poolref->run_condition);
    pthread_mutex_destroy(&poolref->run_mutex);
#endif
}

static void destroy_pool(PFRTThreadPool poolref) {
    destroy_run(poolref);
#ifdef _MSC_VER
    CloseHandle(poolref->pool_mutex);
#else
    pthread_mutex_destroy(&poolref->pool_mutex);
#endif
    release_list_bang(poolref->work_list);
    release_list_bang(poolref->thread_list);
}


static PFRTAny wait_for_work(PFRTThreadPool poolref) {
    PFRTAny res = nil;
    lock_run(poolref);
    lock_pool(poolref);

    if(poolref->pause_work == true) {
        res = pool_pause;
    }
    else if(poolref->stop_work == true) {
        res = pool_exit;
    }
    unlock_pool(poolref);
    if(res == nil) {
        if(poolref->work_list->count == 0) {
            wait_for_work_to_run(poolref);
            lock_pool(poolref);
            if(poolref->pause_work == true) {
                res = pool_pause;
            }
            else if(poolref->stop_work == true) {
                res = pool_exit;
            }
            unlock_pool(poolref);
            if(res == nil) {
                res = list_first(poolref->work_list);
                list_pop_bang(poolref->work_list);
            }
        }
        else {
            res = list_first(poolref->work_list);
            list_pop_bang(poolref->work_list);
        }
    }
    unlock_run(poolref);
    return res;
}

static void run_post(PFRTThreadPool poolref) {
#ifdef _MSC_VER
    WakeConditionVariable(&poolref->run_condition);
#else
    pthread_cond_signal(&poolref->run_condition);
#endif
}

static void run_broadcast(PFRTThreadPool poolref) {
#ifdef _MSC_VER
    WakeAllConditionVariable(&poolref->run_condition);
#else
    pthread_cond_broadcast(&poolref->run_condition);
#endif
}

static void  push_task(PFRTThreadPool poolref, PFRTAny wrkref) {
    // Check state change behavior
    PFRTAny flag=false;
    lock_run(poolref);
    lock_pool(poolref);
    if(poolref->pause_work == true) {
        flag = pool_pause;
        if(poolref->block_queue == true) {
            ;   // Ignore push request
        }
        else {
            foidl_list_extend_bang(poolref->work_list,wrkref);
        }
    }
    else if(poolref->stop_work == true) {
        flag = pool_exit;   // Ignore push request
    }
    else {
        foidl_list_extend_bang(poolref->work_list,wrkref);
        run_post(poolref);
    }
    unlock_pool(poolref);
    unlock_run(poolref);
    return;
}

globalScalarConst(pthrd_init,byte_type,(void *) 0x1,1);
globalScalarConst(pthrd_idle,byte_type,(void *) 0x2,1);
globalScalarConst(pthrd_running,byte_type,(void *) 0x3,1);
globalScalarConst(pthrd_paused,byte_type,(void *) 0x4,1);
globalScalarConst(pthrd_ended,byte_type,(void *) 0x5,1);


#ifdef _MSC_VER
static DWORD WINAPI pool_worker(void* arg)
#else
static void *pool_worker(void *arg)
#endif
{
    PFRTThread  pthrd = (PFRTThread) arg;
    PFRTThreadPool poolref = (PFRTThreadPool) pthrd->pool_parent;
    lock_pool(poolref);
    poolref->active_threads++;
    pthrd->thread_state = pthrd_init;
    unlock_pool(poolref);
    for(;;) {
        // Wait for work
        pthrd->thread_state = pthrd_idle;
        PFRTAny ptsk = wait_for_work(poolref);
        // Check for end
        if(ptsk == pool_exit) {
            break;
        }
        else if(ptsk == pool_pause) {
            PFRTAny     pause_state = true;
            pthrd->thread_state = pthrd_paused;
            do {
                foidl_nap_bang(poolref->thread_pause_timer);
                lock_pool(poolref);
                pause_state = poolref->pause_work;
                unlock_pool(poolref);
            } while( pause_state == true);
        }
        else {
            pthrd->thread_state = pthrd_running;
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
    }
    printf("Shutting down thread\n");
    lock_pool(poolref);
    printf("Aquired shutdown lock\n");
    poolref->active_threads--;
    pthrd->thread_state = pthrd_ended;
    printf("    Thread %d ended\n", pthrd->thid);
    printf("Released shutdown lock\n");
    unlock_pool(poolref);
#ifdef _MSC_VER
    ExitThread(0);
    return 0;
#else
    pthread_exit((void *) poolref);
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
    create_pool_controls(poolref);
    for(ft x=0; x < poolref->count; ++x) {
        PFRTThread pthrd = create_pool_thread(poolref,x);
        foidl_list_extend_bang(poolref->thread_list,(PFRTAny)pthrd);
    }
    unlock_pool(poolref);
    while(poolref->count != poolref->active_threads) {}
    poolref->pool_state = pool_running;
    return poolref;
}

PFRTAny foidl_create_thread_pool_bang(PFRTAny thread_pause_time) {
    PFRTThreadPool poolref = allocThreadPool();
    poolref->count = getNumberOfCores();
    poolref->thread_pause_timer = thread_pause_time;
    poolref->active_threads = 0;
    return (PFRTAny) initialize_pool(poolref);
}

PFRTAny foidl_queue_work_bang(PFRTAny pool, PFRTAny wrkref) {
    if(pool->fclass == worker_class && pool->ftype == thrdpool_type) {
        push_task((PFRTThreadPool)pool, wrkref);
    }
    return wrkref;
}

PFRTAny foidl_queue_thread_bang(PFRTAny pool,PFRTAny funcref, PFRTAny argcoll) {
    if(pool->fclass == worker_class && pool->ftype == thrdpool_type) {
        PFRTAny wrkref = foidl_task_bang(funcref, argcoll);
        push_task((PFRTThreadPool)pool, wrkref);
        return wrkref;
    }
    return nil;
}


// Pauses the workers and may block new work add
PFRTAny foidl_pause_thread_pool_bang(PFRTAny pool, PFRTAny blockwork) {
    if(pool->fclass == worker_class && pool->ftype == thrdpool_type) {
        PFRTThreadPool poolref = (PFRTThreadPool) pool;
        lock_pool(poolref);
        poolref->pause_work = true;
        poolref->pool_state = pool_pause;
        if(blockwork == true) {
            poolref->block_queue = blockwork;
            poolref->pool_state = pool_pause_block;
        }
        unlock_pool(poolref);
    }
    return pool;
}

// Resumes normal operation
PFRTAny foidl_resume_thread_pool_bang(PFRTAny pool) {
    if(pool->fclass == worker_class && pool->ftype == thrdpool_type) {
        PFRTThreadPool poolref = (PFRTThreadPool) pool;
        lock_pool(poolref);
        poolref->pause_work = false;
        poolref->block_queue = false;
        poolref->pool_state = pool_running;
        unlock_pool(poolref);
    }
    else {
        printf("Unknown resume type\n");
        unknown_handler();
    }
    return pool;
}

// Graceful pool exit
PFRTAny foidl_exit_thread_pool_bang(PFRTAny pool) {
    if(pool->fclass == worker_class && pool->ftype == thrdpool_type) {
        PFRTThreadPool poolref = (PFRTThreadPool) pool;
        if(poolref->pool_state != pool_exit) {
            foidl_queue_work_bang(pool,pool_exit);
            lock_pool(poolref);
            lock_run(poolref);
            poolref->stop_work = true;
            poolref->pool_state = pool_exit;
            unlock_pool(poolref);
            run_broadcast(poolref);
            unlock_run(poolref);
            printf("Posted shutdown, waiting for active thread kill\n");
            while(poolref->active_threads > 0) {;}
            destroy_pool(poolref);
        }
        else {
            printf("Attempting to exit pool already exited\n");
            unknown_handler();
        }
    }
    else {
        printf("Unknown type passed to pool_exit\n");
        unknown_handler();
    }
    return pool;
}

PFRTAny     foidl_pool_state(PFRTAny pool) {
    PFRTAny res = nil;
    if(pool->fclass == worker_class && pool->ftype == thrdpool_type) {
        PFRTThreadPool poolref = (PFRTThreadPool) pool;
        lock_pool(poolref);
        res = poolref->pool_state;
        unlock_pool(poolref);
    }
    else {
        printf("Called pool_state but target not pool\n");
        unknown_handler();
    }
    return res;
}

PFRTAny     foidl_pool_thread_states(PFRTAny pool) {
    PFRTAny tlist = nil;
    if(pool->fclass == worker_class && pool->ftype == thrdpool_type) {
        PFRTThreadPool poolref = (PFRTThreadPool) pool;
        lock_pool(poolref);
        tlist = foidl_list_inst_bang();
        PFRTIterator itr = iteratorFor(poolref->thread_list);
        PFRTAny iNext;
        while((iNext = iteratorNext(itr)) != end) {
            PFRTThread pthrd = (PFRTThread) iNext;
            foidl_list_extend_bang(tlist,pthrd->thread_state);
        }
        foidl_xdel(itr);
        unlock_pool(poolref);
    }
    else {
        printf("Called pool_state but target not pool\n");
        unknown_handler();
    }
    return tlist;
}

void foidl_rtl_init_work() {
}

