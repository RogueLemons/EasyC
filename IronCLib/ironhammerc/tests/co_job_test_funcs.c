#include "co_job_test_funcs.h"

static void first_step(void* data);
static void step(void* data);

static const ic_co_job internal_co_job_static = IC_MAKE_CO_JOB(first_step, step, step, step, step, step, step);

const ic_co_job* co_job_7_steps(void)
{
    return &internal_co_job_static;
}

ic_co_job make_co_job_5_steps(void)
{
    ic_co_job j = ic_make_co_job();
    ic_co_job_add_step(&j, first_step);
    ic_co_job_add_step(&j, step);
    ic_co_job_add_step(&j, step);
    ic_co_job_add_step(&j, step);
    ic_co_job_add_step(&j, step);
    return j;
}

static void first_step(void* data)
{
    co_job_test_data* test_data = (co_job_test_data*)data;
    *test_data = (co_job_test_data){0};

    const int index = test_data->step++;
    test_data->values[index] = test_data->step;
}

static void step(void* data)
{
    co_job_test_data* test_data = (co_job_test_data*)data;

    const int index = test_data->step++;
    test_data->values[index] = test_data->step;
}
