#ifndef IC_CO_JOB_H
#define IC_CO_JOB_H

#include <stddef.h>
#include "ic_inline.h"

/*
Compatibility:
    - C99+ supported
    - IC_MAKE_CO_JOB(...) relies on variadic macro argument expansion and supports 
      up to IC_CO_MAX_STEPS entries (default 10). While this works on modern C99 
      compilers including GCC, Clang, and MSVC, some older or non-conforming 
      preprocessors may handle variadic macro expansion differently. 
    - For maximum portability, jobs can always be constructed manually with 
      ic_make_co_job and ic_co_job_add_step instead of using the macro shortcut.
*/

// ========== API ==========
//
// #ifndef IC_CO_MAX_STEPS
// #ifndef IC_CO_MAX_JOBS
// #ifndef IC_CO_BAD_CALL_CAPTURE
// #ifndef IC_CO_SCORE
//
// typedef function pointer: ic_co_step                             [ void func(void* data) ]
// typedef struct:           ic_co_job                              [ job consists of multiple steps ]
// typedef struct:           ic_co_scheduler                        [ schedules steps across multiple jobs ]
//
// IC_MAKE_CO_JOB(step1, step2, step3, ...)                         [ returns an ic_co_job, supports up to 10 steps ]
// ic_co_job ic_make_co_job(void)                                   [ returns an initialized, empty job ]
// void ic_co_job_add_step(ic_co_job* job,                          [ adds steps, in order, to a job ]
//                        ic_co_step step)                          [ less useful but more portable and safer than IC_MAKE_CO_JOB ]
// void ic_co_job_run(const ic_co_job* const job, void* const data) [ run single ic_co_job as single function ] 
// 
// ic_co_scheduler ic_make_co_scheduler(void)                       [ create a scheduler ]
// void ic_co_scheduler_add_job(ic_co_scheduler* s, 
//                              const ic_co_job* job, 
//                              void* const data,                   [ shared data lives outside the job (unless static), and in same scope as scheduler ]
//                              int priority,                       [ priority is about importance to run first ]
//                              int weight)                         [ weight is about steps per cycle ]
// void ic_co_scheduler_tick(ic_co_scheduler* s)                    [ tick to run one step in whole scheduler ]
// int ic_co_scheduler_is_done(const ic_co_scheduler* s)            [ returns 1 (true) if all jobs are finished, otherwise 0 (false)  ]
// int ic_co_scheduler_job_count(const ic_co_scheduler* s)          [ total number of jobs in this scheduler ]
// int ic_co_scheduler_credits(const ic_co_scheduler* s)            [ total number of credits (steps) ramaining for this cycle ]

// ========== CONFIG ==========

#ifndef IC_CO_MAX_STEPS
#define IC_CO_MAX_STEPS 10
#endif

#ifndef IC_CO_MAX_JOBS
#define IC_CO_MAX_JOBS 16
#endif

#ifndef IC_CO_BAD_CALL_CAPTURE
#define IC_CO_BAD_CALL_CAPTURE(msg, scheduler, job, step, priority, weight) (void)0
#endif

// Priority:    Importance to go first
// Credits:     Remaining steps to take in current cycle (equals weight at cycle start)
// State:       The step the job is on, i.e. total steps taken for job
#ifndef IC_CO_SCORE
#define IC_CO_SCORE(priority, credits, state) \
    ((priority) * (credits) - (state))
#endif


// ========== CORE TYPES ==========

typedef void (*ic_co_step)(void* data);

typedef struct ic_co_job
{
    ic_co_step UNSAFE_PRIVATE_ACCESS_steps[IC_CO_MAX_STEPS];
    int UNSAFE_PRIVATE_ACCESS_step_count;

} ic_co_job;


typedef struct PRIVATE_ic_co_scheduled_job
{
    const ic_co_job* job;
    void* data;

    int state;

    int priority;
    int weight;
    int credits;

} PRIVATE_ic_co_scheduled_job;


typedef struct ic_co_scheduler
{
    PRIVATE_ic_co_scheduled_job UNSAFE_PRIVATE_ACCESS_jobs[IC_CO_MAX_JOBS];
    int UNSAFE_PRIVATE_ACCESS_job_count;

    PRIVATE_ic_co_scheduled_job* UNSAFE_PRIVATE_ACCESS_runnable[IC_CO_MAX_JOBS];
    int UNSAFE_PRIVATE_ACCESS_runnable_count;

    int UNSAFE_PRIVATE_ACCESS_total_credits;
    unsigned char UNSAFE_PRIVATE_ACCESS_is_done; // dirty portable bool

} ic_co_scheduler;


// ========== IC_MAKE_CO_JOB ==========

#define IC_INNER_MAKE_CO_JOB_1(a) \
{ { a }, 1 }

#define IC_INNER_MAKE_CO_JOB_2(a,b) \
{ { a,b }, 2 }

#define IC_INNER_MAKE_CO_JOB_3(a,b,c) \
{ { a,b,c }, 3 }

#define IC_INNER_MAKE_CO_JOB_4(a,b,c,d) \
{ { a,b,c,d }, 4 }

#define IC_INNER_MAKE_CO_JOB_5(a,b,c,d,e) \
{ { a,b,c,d,e }, 5 }

#define IC_INNER_MAKE_CO_JOB_6(a,b,c,d,e,f) \
{ { a,b,c,d,e,f }, 6 }

#define IC_INNER_MAKE_CO_JOB_7(a,b,c,d,e,f,g) \
{ { a,b,c,d,e,f,g }, 7 }

#define IC_INNER_MAKE_CO_JOB_8(a,b,c,d,e,f,g,h) \
{ { a,b,c,d,e,f,g,h }, 8 }

#define IC_INNER_MAKE_CO_JOB_9(a,b,c,d,e,f,g,h,i) \
{ { a,b,c,d,e,f,g,h,i }, 9 }

#define IC_INNER_MAKE_CO_JOB_10(a,b,c,d,e,f,g,h,i,j) \
{ { a,b,c,d,e,f,g,h,i,j }, 10 }

#define IC_INNER_GET_MACRO( \
     _1,_2,_3,_4,_5, \
     _6,_7,_8,_9,_10, \
     NAME,...) NAME

#define IC_MAKE_CO_JOB(...) \
    IC_INNER_GET_MACRO( \
        __VA_ARGS__, \
        IC_INNER_MAKE_CO_JOB_10, \
        IC_INNER_MAKE_CO_JOB_9, \
        IC_INNER_MAKE_CO_JOB_8, \
        IC_INNER_MAKE_CO_JOB_7, \
        IC_INNER_MAKE_CO_JOB_6, \
        IC_INNER_MAKE_CO_JOB_5, \
        IC_INNER_MAKE_CO_JOB_4, \
        IC_INNER_MAKE_CO_JOB_3, \
        IC_INNER_MAKE_CO_JOB_2, \
        IC_INNER_MAKE_CO_JOB_1, \
    )(__VA_ARGS__)

// ========== CO JOB ==========

IC_HEADER_FUNC ic_co_job ic_make_co_job(void)
{
    return (ic_co_job){0};
}

IC_HEADER_FUNC void ic_co_job_add_step(ic_co_job* const job, const ic_co_step step)
{
    if ((job == NULL) || (job->UNSAFE_PRIVATE_ACCESS_step_count >= IC_CO_MAX_STEPS) || (step == NULL))
    {
        IC_CO_BAD_CALL_CAPTURE("ic_co_job_add_step", NULL, job, step, -1, -1);
        return;
    }

    job->UNSAFE_PRIVATE_ACCESS_steps[job->UNSAFE_PRIVATE_ACCESS_step_count++] = step;
}

IC_HEADER_FUNC void ic_co_job_run(const ic_co_job* const job, void* const data)
{
    if (job == NULL)
    {
        IC_CO_BAD_CALL_CAPTURE("ic_co_job_run", NULL, job, NULL, -1, -1);
        return;
    }
    for (int i = 0; i < job->UNSAFE_PRIVATE_ACCESS_step_count; i++)
    {
        if (job->UNSAFE_PRIVATE_ACCESS_steps[i] == NULL)
        {
            IC_CO_BAD_CALL_CAPTURE("ic_co_job_run, NULL job step", NULL, job, NULL, -1, -1);
            return;
        }
    }

    for (int i = 0; i < job->UNSAFE_PRIVATE_ACCESS_step_count; i++)
    {
        job->UNSAFE_PRIVATE_ACCESS_steps[i](data);
    }
}

// ========== CO SCHEDULER ==========

IC_HEADER_FUNC ic_co_scheduler ic_make_co_scheduler(void)
{
    ic_co_scheduler s = {0};
    s.UNSAFE_PRIVATE_ACCESS_is_done = 1;
    return s;
}

IC_HEADER_FUNC void ic_co_scheduler_add_job(ic_co_scheduler* const s, const ic_co_job* const job, void* const data, const int priority, const int weight)
{
    if ((s == NULL) || (s->UNSAFE_PRIVATE_ACCESS_job_count >= IC_CO_MAX_JOBS) || (job == NULL) || (job->UNSAFE_PRIVATE_ACCESS_step_count <= 0) || (priority < 0) || (weight <= 0) || (weight > IC_CO_MAX_STEPS))
    {
        IC_CO_BAD_CALL_CAPTURE("ic_co_scheduler_add_job", s, job, NULL, priority, weight);
        return;
    }
    for (int i = 0; i < job->UNSAFE_PRIVATE_ACCESS_step_count; i++)
    {
        if (job->UNSAFE_PRIVATE_ACCESS_steps[i] == NULL)
        {
            IC_CO_BAD_CALL_CAPTURE("ic_co_scheduler_add_job, NULL job step", s, job, NULL, priority, weight);
            return;
        }
    }

    PRIVATE_ic_co_scheduled_job* const j = &s->UNSAFE_PRIVATE_ACCESS_jobs[s->UNSAFE_PRIVATE_ACCESS_job_count++];

    j->job = job;
    j->data = data;

    j->state = 0;

    j->priority = priority;
    j->weight = weight;

    j->credits = 0;

    s->UNSAFE_PRIVATE_ACCESS_is_done = 0;
}

IC_HEADER_FUNC void ic_co_scheduler_tick(ic_co_scheduler* const s)
{
    // Check null
    if (s == NULL)
    {
        IC_CO_BAD_CALL_CAPTURE("ic_co_scheduler_tick", s, NULL, NULL, -1, -1);
        return;
    }

    // rebuild runnable list and refill credits
    if (s->UNSAFE_PRIVATE_ACCESS_total_credits <= 0)
    {
        s->UNSAFE_PRIVATE_ACCESS_total_credits = 0;
        s->UNSAFE_PRIVATE_ACCESS_runnable_count = 0;

        for (int i = 0; i < s->UNSAFE_PRIVATE_ACCESS_job_count; i++)
        {
            PRIVATE_ic_co_scheduled_job* const j = &s->UNSAFE_PRIVATE_ACCESS_jobs[i];

            const int remaining = j->job->UNSAFE_PRIVATE_ACCESS_step_count - j->state;

            if (remaining <= 0)
            {
                continue;
            }

            const int granted = (j->weight < remaining) ? j->weight : remaining;

            j->credits = granted;
            s->UNSAFE_PRIVATE_ACCESS_total_credits += granted;
            s->UNSAFE_PRIVATE_ACCESS_runnable[s->UNSAFE_PRIVATE_ACCESS_runnable_count++] = j;
        }
    }

    // Set next job by score
    PRIVATE_ic_co_scheduled_job* j = NULL;
    int best_score = -1; // auto updated to first element when j == NULL
    for (int i = 0; i < s->UNSAFE_PRIVATE_ACCESS_runnable_count; i++)
    {
        PRIVATE_ic_co_scheduled_job* job = s->UNSAFE_PRIVATE_ACCESS_runnable[i];

        if (job->credits <= 0)
        {
            continue;
        }

        const int score = IC_CO_SCORE(job->priority, job->credits, job->state);

        if ((j == NULL) || (score > best_score))
        {
            j = job;
            best_score = score;
        }
    }

    // If no job selected then all jobs are done
    if (j == NULL)
    {
        s->UNSAFE_PRIVATE_ACCESS_is_done = 1;
        return;
    }

    // Run this step of the job
    j->job->UNSAFE_PRIVATE_ACCESS_steps[j->state](j->data);

    j->state++;
    j->credits--;
    s->UNSAFE_PRIVATE_ACCESS_total_credits--;

    // remove from runnable immediately if finished
    // if (j->state >= j->job->step_count)
    // {
    //     for (int i = 0; i < s->runnable_count; i++)
    //     {
    //         if (s->runnable[i] == j)
    //         {
    //             s->runnable[i] = s->runnable[s->runnable_count - 1];
    //             s->runnable_count--;
    //             break;
    //         }
    //     }
    // }
}

IC_HEADER_FUNC int ic_co_scheduler_is_done(const ic_co_scheduler* const s)
{
    if (s == NULL)
    {
        IC_CO_BAD_CALL_CAPTURE("ic_co_scheduler_is_done", s, NULL, NULL, -1, -1);
        return 1;
    }

    return (int)s->UNSAFE_PRIVATE_ACCESS_is_done;
}

IC_HEADER_FUNC int ic_co_scheduler_job_count(const ic_co_scheduler* const s)
{
    if (s == NULL)
    {
        IC_CO_BAD_CALL_CAPTURE("ic_co_scheduler_job_count", s, NULL, NULL, -1, -1);
        return 0;
    }

    return s->UNSAFE_PRIVATE_ACCESS_job_count;
}

IC_HEADER_FUNC int ic_co_scheduler_credits(const ic_co_scheduler* const s)
{
    if (s == NULL)
    {
        IC_CO_BAD_CALL_CAPTURE("ic_co_scheduler_credits", s, NULL, NULL, -1, -1);
        return 0;
    }

    return s->UNSAFE_PRIVATE_ACCESS_total_credits;
}


#endif // IC_CO_JOB_H