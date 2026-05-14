#ifndef IRON_HAMMER_C_TESTS_CO_JOB_TEST_H
#define IRON_HAMMER_C_TESTS_CO_JOB_TEST_H

struct ic_co_scheduler;
struct ic_co_job;
typedef struct BadCaptureData
{
    const char* msg;
    const struct ic_co_scheduler* scheduler;
    const struct ic_co_job* job;
    void (*step)(void* data);
    int priority;
    int weight;

} BadCaptureData;
static BadCaptureData bad_capture_data;

#ifdef IC_CO_BAD_CALL_CAPTURE
#error Wrong order of includes
#else
#define IC_CO_BAD_CALL_CAPTURE(msg_, scheduler_, job_, step_, priority_, weight_) do \
{ \
    bad_capture_data.msg = msg_; \
    bad_capture_data.scheduler = scheduler_; \
    bad_capture_data.job = job_; \
    bad_capture_data.step = step_; \
    bad_capture_data.priority = priority_; \
    bad_capture_data.weight = weight_; \
} while(0);
#endif

#include "ic_hammer.h"
#include "ironclib/ic_co_job.h"
#include "co_job_test_funcs.h"

static void do_nothing_step(void* data)
{
    (void)data;
}

IHC_TEST(verify_co_job_add_detects_nullptr_and_too_many_steps)
{
    ic_co_job job = ic_make_co_job();

    bad_capture_data.job = &job;
    ic_co_job_add_step(NULL, do_nothing_step);
    IHC_ASSERT(bad_capture_data.job == NULL);

    bad_capture_data.step = do_nothing_step;
    ic_co_job_add_step(&job, NULL);
    IHC_ASSERT(bad_capture_data.step == NULL);

    bad_capture_data.job = NULL;
    bad_capture_data.step = NULL;
    for (int i = 0; i < IC_CO_MAX_STEPS; i++)
    {
        ic_co_job_add_step(&job, do_nothing_step);
    }
    IHC_CHECK(bad_capture_data.job == NULL);
    IHC_CHECK(bad_capture_data.step == NULL);

    ic_co_job_add_step(&job, do_nothing_step);
    IHC_ASSERT(bad_capture_data.job == &job);
    IHC_CHECK(bad_capture_data.step == do_nothing_step);

    // Maybe add ic_co_job_step_count function and use it here to verify rejections as well as detections
}


static int shared_int = 0;
static void increase_shared_int(void* data)
{
    (void)data;
    shared_int++;
}

IHC_TEST(verify_co_job_runs_correct_number_of_steps)
{
    shared_int = 0;
    ic_co_job job = ic_make_co_job();
    const int number_of_steps = 3;
    for (int i = 0; i < number_of_steps; i++)
    {
        ic_co_job_add_step(&job, increase_shared_int);
    }

    ic_co_job_run(&job, NULL);
    IHC_CHECK(shared_int == number_of_steps);
}

static void increment_int_by_1(void* data)
{
    int* i = (int*)data;
    (*i)++;
}

IHC_TEST(verify_data_transfer_in_job_run)
{
    int i = 0;
    ic_co_job job = IC_MAKE_CO_JOB(increment_int_by_1, increment_int_by_1, increment_int_by_1);
    ic_co_job_run(&job, &i);
    IHC_CHECK(i == 3);
}

IHC_TEST(verify_co_scheduler_initial_getter_values)
{
    ic_co_scheduler sched = ic_make_co_scheduler();
    IHC_CHECK(ic_co_scheduler_job_count(&sched) == 0);
    IHC_CHECK(ic_co_scheduler_credits(&sched) == 0);
    IHC_CHECK(ic_co_scheduler_is_done(&sched) == 1);
}

IHC_TEST(verify_co_scheduler_getters_detects_null)
{
    ic_co_scheduler sched = ic_make_co_scheduler();

    bad_capture_data.scheduler = &sched;
    IHC_CHECK(ic_co_scheduler_job_count(NULL) == 0);
    IHC_CHECK(bad_capture_data.scheduler == NULL);

    bad_capture_data.scheduler = &sched;
    IHC_CHECK(ic_co_scheduler_credits(NULL) == 0);
    IHC_CHECK(bad_capture_data.scheduler == NULL);

    bad_capture_data.scheduler = &sched;
    IHC_CHECK(ic_co_scheduler_is_done(NULL) == 1);
    IHC_CHECK(bad_capture_data.scheduler == NULL);
}

IHC_TEST(verify_co_scheduler_updates_done_status_after_job_entry)
{
    ic_co_scheduler sched = ic_make_co_scheduler();
    IHC_CHECK(ic_co_scheduler_job_count(&sched) == 0);
    
    ic_co_job job = IC_MAKE_CO_JOB(do_nothing_step);
    ic_co_scheduler_add_job(&sched, &job, NULL, 1, 1);
    IHC_CHECK(ic_co_scheduler_job_count(&sched) == 1);
}

IHC_TEST(verify_co_scheduler_add_detects_nullptr_and_too_many_jobs)
{
    ic_co_scheduler sched = ic_make_co_scheduler();
    ic_co_job job = IC_MAKE_CO_JOB(do_nothing_step);

    bad_capture_data.scheduler = &sched;
    ic_co_scheduler_add_job(NULL, &job, NULL, 1, 1);
    IHC_ASSERT(bad_capture_data.scheduler == NULL);

    bad_capture_data.job = &job;
    ic_co_scheduler_add_job(&sched, NULL, NULL, 1, 1);
    IHC_ASSERT(bad_capture_data.job == NULL);
    
    for (int i = 0; i < IC_CO_MAX_JOBS; i++)
    {
        IHC_CHECK(ic_co_scheduler_job_count(&sched) == i);
        ic_co_scheduler_add_job(&sched, &job, NULL, 1, 1);
    }
    IHC_CHECK(ic_co_scheduler_job_count(&sched) == IC_CO_MAX_JOBS);

    bad_capture_data.scheduler = NULL;
    bad_capture_data.job = NULL;
    ic_co_scheduler_add_job(&sched, &job, NULL, 1, 1);
    IHC_CHECK(ic_co_scheduler_job_count(&sched) == IC_CO_MAX_JOBS);
    IHC_CHECK(bad_capture_data.scheduler == &sched);
    IHC_CHECK(bad_capture_data.job == &job);
}

IHC_TEST(verify_co_scheduler_rejects_empty_job)
{
    ic_co_scheduler sched = ic_make_co_scheduler();
    IHC_ASSERT(ic_co_scheduler_job_count(&sched) == 0);
    ic_co_job job = ic_make_co_job();
    bad_capture_data.job = NULL;
    ic_co_scheduler_add_job(&sched, &job, NULL, 1, 1);
    IHC_CHECK(ic_co_scheduler_job_count(&sched) == 0);
    IHC_CHECK(bad_capture_data.job == &job);
}

IHC_TEST(verify_co_scheduler_rejects_bad_arguments)
{
    ic_co_scheduler sched = ic_make_co_scheduler();
    IHC_ASSERT(ic_co_scheduler_job_count(&sched) == 0);
    ic_co_job job = IC_MAKE_CO_JOB(do_nothing_step);
    ic_co_scheduler_add_job(&sched, &job, NULL, 1, 1);
    bad_capture_data.job = NULL;
    IHC_ASSERT(ic_co_scheduler_job_count(&sched) == 1);
    IHC_ASSERT(bad_capture_data.job == NULL);
    
    bad_capture_data.priority = 3;
    int job_count_before_add = ic_co_scheduler_job_count(&sched);
    ic_co_scheduler_add_job(&sched, &job, NULL, -5, 1);
    IHC_CHECK(ic_co_scheduler_job_count(&sched) == job_count_before_add);
    IHC_CHECK(bad_capture_data.priority == -5);

    bad_capture_data.weight = 3;
    job_count_before_add = ic_co_scheduler_job_count(&sched);
    ic_co_scheduler_add_job(&sched, &job, NULL, 1, -5);
    IHC_CHECK(ic_co_scheduler_job_count(&sched) == job_count_before_add);
    IHC_CHECK(bad_capture_data.weight == -5);

    bad_capture_data.weight = 3;
    job_count_before_add = ic_co_scheduler_job_count(&sched);
    const int too_big_weight = IC_CO_MAX_STEPS + 1;
    ic_co_scheduler_add_job(&sched, &job, NULL, 1, too_big_weight);
    IHC_CHECK(ic_co_scheduler_job_count(&sched) == job_count_before_add);
    IHC_CHECK(bad_capture_data.weight == too_big_weight);
}

static co_job_test_data make_bad_test_data(void)
{
    co_job_test_data test_data;
    test_data.step = -1;
    for (int i = 0; i < IC_CO_MAX_STEPS; i++)
    {
        test_data.values[i] = -1;
    }
    return test_data;
}

IHC_TEST(verify_premade_jobs_hidden_in_source_files_can_be_run)
{
    co_job_test_data test_data = make_bad_test_data();
    ic_co_job_run(co_job_7_steps(), &test_data);

    IHC_CHECK(test_data.step == 7);
    for (int i = 0; i < IC_CO_MAX_STEPS; i++)
    {
        const int expected_value = (i >= 7) ? 0 : i + 1;
        IHC_CHECK(test_data.values[i] == expected_value);
    }
}

IHC_TEST(verify_job_setup_can_be_done_in_source_file)
{
    co_job_test_data test_data = make_bad_test_data();
    ic_co_job job = make_co_job_5_steps();
    ic_co_job_run(&job, &test_data);

    IHC_CHECK(test_data.step == 5);
    for (int i = 0; i < IC_CO_MAX_STEPS; i++)
    {
        const int expected_value = (i >= 5) ? 0 : i + 1;
        IHC_CHECK(test_data.values[i] == expected_value);
    }
}

IHC_TEST(verify_scheduler_tick_detects_nullptr)
{
    ic_co_scheduler sched = ic_make_co_scheduler();
    bad_capture_data.scheduler = &sched;
    ic_co_scheduler_tick(NULL);
    IHC_CHECK(bad_capture_data.scheduler == NULL);
}

IHC_TEST(verify_scheduler_correctlty_ticks_and_runs_one_job)
{
    ic_co_scheduler sched = ic_make_co_scheduler();

    co_job_test_data test_data = make_bad_test_data();
    const int weight = 3;
    const int expected_credits[] = {2, 1, 0, 2, 1, 0, 0};
    ic_co_scheduler_add_job(&sched, co_job_7_steps(), &test_data, 1, weight);

    int step = 0;
    while (!ic_co_scheduler_is_done(&sched))
    {
        ic_co_scheduler_tick(&sched);
        
        
        if (!ic_co_scheduler_is_done(&sched))
        {
            IHC_CHECK(ic_co_scheduler_credits(&sched) == expected_credits[step]);
            step++;
            IHC_CHECK(test_data.step == step);
        }

    }

    IHC_CHECK(test_data.step == 7);
    for (int i = 0; i < IC_CO_MAX_STEPS; i++)
    {
        const int expected_value = (i >= 7) ? 0 : i + 1;
        IHC_CHECK(test_data.values[i] == expected_value);
    }

    IHC_CHECK(ic_co_scheduler_is_done(&sched));
}

typedef struct job_entry
{
    char tag;
    int step;
} job_entry;

#define SCHED_SHARED_DATA_MAX_ENTRIES (3 * IC_CO_MAX_STEPS)
typedef struct sched_shared_data
{
    job_entry entries[SCHED_SHARED_DATA_MAX_ENTRIES];
    int current_step;
} sched_shared_data;

static sched_shared_data make_sched_shared_data(void)
{
    sched_shared_data data = {0};
    return data;
}

static void add_shared_entry(sched_shared_data* shared_data, char tag, int step)
{
    if ((shared_data == NULL) || (shared_data->current_step >= SCHED_SHARED_DATA_MAX_ENTRIES))
    {
        return;
    }

    job_entry entry;
    entry.tag = tag;
    entry.step = step;
    shared_data->entries[shared_data->current_step++] = entry;
}

typedef struct job_data
{
    int step;
    sched_shared_data* shared_data;
} job_data;

static job_data make_job_data(sched_shared_data* shared_data)
{
    job_data data;
    data.step = 0;
    data.shared_data = shared_data;
    return data;
}

static unsigned char job_X_step_error = 0;

static void job_X_step(char tag, void* data_)
{
    job_data* data = (job_data*)data_;

    switch (data->step)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            add_shared_entry(data->shared_data, tag, data->step);
            data->step++;
            break;
        default:
            job_X_step_error++;
    }
}

static void job_a_step(void* data_)
{
    job_X_step('a', data_);
}

static void job_b_step(void* data_)
{
    job_X_step('b', data_);
}

static void job_c_step(void* data_)
{
    job_X_step('c', data_);
}

IHC_TEST(verify_scheduler_prioritizes_steps_by_priority)
{
    // Setup
    job_X_step_error = 0;
    sched_shared_data shared_data = make_sched_shared_data();
    ic_co_scheduler sched = ic_make_co_scheduler();

    ic_co_job job_a = IC_MAKE_CO_JOB(job_a_step, job_a_step, job_a_step);
    job_data job_a_data = make_job_data(&shared_data);
    ic_co_scheduler_add_job(&sched, &job_a, &job_a_data, 3, 1);
    
    ic_co_job job_b = IC_MAKE_CO_JOB(job_b_step, job_b_step);
    job_data job_b_data = make_job_data(&shared_data);
    ic_co_scheduler_add_job(&sched, &job_b, &job_b_data, 2, 1);
    
    ic_co_job job_c = IC_MAKE_CO_JOB(job_c_step, job_c_step, job_c_step, job_c_step);
    job_data job_c_data = make_job_data(&shared_data);
    ic_co_scheduler_add_job(&sched, &job_c, &job_c_data, 1, 1);

    // Run jobs in schedule
    while (!ic_co_scheduler_is_done(&sched))
    {
        ic_co_scheduler_tick(&sched);
    }

    // verify full runs
    IHC_CHECK(job_a_data.step == 3);
    IHC_CHECK(job_b_data.step == 2);
    IHC_CHECK(job_c_data.step == 4);
    IHC_ASSERT(job_X_step_error == 0);

    // Setup expected results
    job_entry expected_entries[SCHED_SHARED_DATA_MAX_ENTRIES] = 
    {
        {'a', 0},
        {'b', 0},
        {'c', 0},
        {'a', 1},
        {'b', 1},
        {'c', 1},
        {'a', 2},
        {'c', 2},
        {'c', 3},
    };

    // Check results
    for (int i = 0; i < SCHED_SHARED_DATA_MAX_ENTRIES; i++)
    {
        job_entry expected = expected_entries[i];
        job_entry result = shared_data.entries[i];
        IHC_CHECK(expected.tag == result.tag);
        IHC_CHECK(expected.step == result.step);
    }
}

IHC_TEST(verify_scheduler_runs_steps_in_order_for_weighted_jobs)
{
    // Setup
    job_X_step_error = 0;
    sched_shared_data shared_data = make_sched_shared_data();
    ic_co_scheduler sched = ic_make_co_scheduler();

    ic_co_job job_a = IC_MAKE_CO_JOB(job_a_step, job_a_step, job_a_step);
    job_data job_a_data = make_job_data(&shared_data);
    ic_co_scheduler_add_job(&sched, &job_a, &job_a_data, 300, 1);
    
    ic_co_job job_b = IC_MAKE_CO_JOB(job_b_step, job_b_step);
    job_data job_b_data = make_job_data(&shared_data);
    ic_co_scheduler_add_job(&sched, &job_b, &job_b_data, 20, 2);
    
    ic_co_job job_c = IC_MAKE_CO_JOB(job_c_step, job_c_step, job_c_step, job_c_step);
    job_data job_c_data = make_job_data(&shared_data);
    ic_co_scheduler_add_job(&sched, &job_c, &job_c_data, 1, 3);

    // Run jobs in schedule
    while (!ic_co_scheduler_is_done(&sched))
    {
        ic_co_scheduler_tick(&sched);
    }

    // verify full runs
    IHC_CHECK(job_a_data.step == 3);
    IHC_CHECK(job_b_data.step == 2);
    IHC_CHECK(job_c_data.step == 4);
    IHC_ASSERT(job_X_step_error == 0);

    // Setup expected results
    job_entry expected_entries[SCHED_SHARED_DATA_MAX_ENTRIES] = 
    {
        {'a', 0},
        {'b', 0},
        {'b', 1},
        {'c', 0},
        {'c', 1},
        {'c', 2},
        {'a', 1},
        {'c', 3},
        {'a', 2},
    };

    // Check results
    for (int i = 0; i < SCHED_SHARED_DATA_MAX_ENTRIES; i++)
    {
        job_entry expected = expected_entries[i];
        job_entry result = shared_data.entries[i];
        IHC_CHECK(expected.tag == result.tag);
        IHC_CHECK(expected.step == result.step);
    }
}

IHC_TEST(verify_scheduler_interleaves_jobs_with_default_scoring_system)
{
    // Setup
    job_X_step_error = 0;
    sched_shared_data shared_data = make_sched_shared_data();
    ic_co_scheduler sched = ic_make_co_scheduler();

    ic_co_job job_a = IC_MAKE_CO_JOB(job_a_step, job_a_step, job_a_step);
    job_data job_a_data = make_job_data(&shared_data);
    ic_co_scheduler_add_job(&sched, &job_a, &job_a_data, 3, 1);
    
    ic_co_job job_b = IC_MAKE_CO_JOB(job_b_step, job_b_step);
    job_data job_b_data = make_job_data(&shared_data);
    ic_co_scheduler_add_job(&sched, &job_b, &job_b_data, 2, 2);
    
    ic_co_job job_c = IC_MAKE_CO_JOB(job_c_step, job_c_step, job_c_step, job_c_step);
    job_data job_c_data = make_job_data(&shared_data);
    ic_co_scheduler_add_job(&sched, &job_c, &job_c_data, 1, 3);

    // Run jobs in schedule
    while (!ic_co_scheduler_is_done(&sched))
    {
        ic_co_scheduler_tick(&sched);
    }

    // verify full runs
    IHC_CHECK(job_a_data.step == 3);
    IHC_CHECK(job_b_data.step == 2);
    IHC_CHECK(job_c_data.step == 4);
    IHC_ASSERT(job_X_step_error == 0);

    // Setup expected results
    job_entry expected_entries[SCHED_SHARED_DATA_MAX_ENTRIES] = 
    {
        {'b', 0},
        {'a', 0},
        {'c', 0},
        {'b', 1},
        {'c', 1},
        {'c', 2},
        {'a', 1},
        {'c', 3},
        {'a', 2},
    };

    // Check results
    for (int i = 0; i < SCHED_SHARED_DATA_MAX_ENTRIES; i++)
    {
        job_entry expected = expected_entries[i];
        job_entry result = shared_data.entries[i];
        IHC_CHECK(expected.tag == result.tag);
        IHC_CHECK(expected.step == result.step);
    }
}

#endif // IRON_HAMMER_C_TESTS_CO_JOB_TEST_H