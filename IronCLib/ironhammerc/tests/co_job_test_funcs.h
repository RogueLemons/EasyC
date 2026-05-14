#ifndef IRON_HAMMER_C_TESTS_CO_JOB_STATIC_H
#define IRON_HAMMER_C_TESTS_CO_JOB_STATIC_H

#include "ironclib/ic_co_job.h"

struct co_job_test_data
{
    int step;
    int values[IC_CO_MAX_STEPS];
};
typedef struct co_job_test_data co_job_test_data; 

const ic_co_job* co_job_7_steps(void);

ic_co_job make_co_job_5_steps(void);

#endif // IRON_HAMMER_C_TESTS_CO_JOB_STATIC_H