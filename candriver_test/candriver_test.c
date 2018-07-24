/* User code: This file will not be overwritten by TASTE. */

#include "candriver_test.h"
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI
#define M_PI 3.1416
#endif

static float velocity_ref_deg = 1.f;
static asn1SccBase_commands_Joints cmd;

uint64_t getTimeInMicroseconds()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}

void candriver_test_startup()
{
    /* Write your initialization code here,
       but do not make any call to a required interface. */
    printf("[candriver_test] startup\n");
}

void candriver_test_PI_samples(const asn1SccBase_commands_Joints *IN_smpls)
{
    /* Write your code here! */
    float position_rad = IN_smpls->elements.arr[0].position;
    float position_deg = position_rad * 180.f / M_PI;
    printf("[candriver_test] Current pos: %f\n", position_deg);
    if ((velocity_ref_deg > 0.f) && (position_deg < 90.f))
    {
	velocity_ref_deg = 1.f;
    }
    if ((velocity_ref_deg < 0.f) && (position_deg > 10.f))
    {
	velocity_ref_deg = -1.f;
    }
    if ((velocity_ref_deg > 0.f) && (position_deg > 90.f))
    {
	velocity_ref_deg = -0.5f;
    }
    if ((velocity_ref_deg < 0.f) && (position_deg < 10.f))
    {
	velocity_ref_deg = 0.5f;
    }
    printf("[candriver_test] New velocity reference: %f\n", velocity_ref_deg);
    cmd.time.microseconds = getTimeInMicroseconds();
    cmd.names.nCount = 1;
    cmd.names.arr[0].nCount = snprintf((char*)cmd.names.arr[0].arr, 200, "JOINT0");
    cmd.elements.nCount = 1;
    cmd.elements.arr[0].speed = velocity_ref_deg * M_PI / 180.f;
    candriver_test_RI_commands(&cmd);
}

