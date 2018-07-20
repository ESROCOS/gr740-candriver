/* User code: This file will not be overwritten by TASTE. */

#include "candriver_test.h"
#include <stdint.h>
#include <time.h>

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
    float position_deg = IN_smpls->elements.arr[0].position;
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
    cmd.names.arr[0].nCount = 7;
    strncpy(cmd.names.arr[0].arr, "myJoint", 7);
    cmd.elements.nCount = 1;
    cmd.elements.arr[0].speed = velocity_ref_deg;
    candriver_test_RI_commands(&cmd);
}

