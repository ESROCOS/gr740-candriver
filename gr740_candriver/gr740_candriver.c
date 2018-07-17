/* User code: This file will not be overwritten by TASTE. */

#include "gr740_candriver.h"

/*
	IMPORTANT NOTE
	The driver manager is setup by the orchestrator and/or polyorb-HI-C.
        It is therefore needed that the CONFIGURE_DRIVER_AMBAPP_GAISLER_GRCAN has to be passed to the build system.
        Currently, this is done through a so called TASTE Directive which is setup as a contextual parameter.
*/

#ifndef CONFIGURE_DRIVER_AMBAPP_GAISLER_GRCAN
#error "Need GRCAN driver"
#endif

/* RTEMS include files */
#include <bsp/grcan.h>

#ifndef CAN_INTERFACE
#define CAN_INTERFACE 0
#endif

#ifndef CAN_BAUDRATE
#define CAN_BAUDRATE 1000000
#endif

/* CAN Channel select */
enum {
	CAN_CHAN_SEL_A,
	CAN_CHAN_SEL_B,
	CAN_CHAN_SEL_NUM
};

#ifndef CAN_CHANNEL
#define CAN_CHANNEL CAN_CHAN_SEL_A
#endif

static void *candev = NULL;
static const struct grcan_selection CAN_CHAN_SEL[CAN_CHAN_SEL_NUM] = {
	{
		/* Channel A */
		.selection = 0,
		.enable0 = 0,
		.enable1 = 1,
	},
	{
		/* Channel B */
		.selection = 1,
		.enable0 = 1,
		.enable1 = 0,
	},
};

/* Driver specific data */
#define CANDRIVER_MAX_READ_ATTEMPTS	10
static bool requestTransmitted = false;
static unsigned responseReadAttempts = 0;

void gr740_candriver_startup()
{
    /* Write your initialization code here,
       but do not make any call to a required interface. */
    int ret;
    int nCANDevices;
    printf("[gr740_candriver_startup] startup\n");

    nCANDevices = grcan_dev_count();
    if (!nCANDevices)
    {
	printf("[gr740_candriver_startup] No CAN devices found\n");
	return;
    }
    if (CAN_INTERFACE >= nCANDevices)
    {
	printf("[gr740_candriver_startup] CAN interface %u not available\n", CAN_INTERFACE);
	return;
    }
    printf("[gr740_candriver_startup] init CAN interface %u\n", CAN_INTERFACE);
    candev = grcan_open(CAN_INTERFACE);
    if (candev == NULL)
    {
	printf("[gr740_candriver_startup] failed to open CAN device %u\n", CAN_INTERFACE);
	return;
    }
    printf("[gr740_candriver_startup] select baudrate %u\n", CAN_BAUDRATE);
    ret = grcan_set_speed(candev, CAN_BAUDRATE);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] failed to set baudrate %u\n", CAN_BAUDRATE);
	return;
    }
    printf("[gr740_candriver_startup] select channel %u\n", CAN_CHANNEL);
    ret = grcan_set_selection(candev, &CAN_CHAN_SEL[CAN_CHANNEL]);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] failed to set channel %u\n", CAN_CHANNEL);
	return;
    }
    printf("[gr740_candriver_startup] enable txcomplete\n");
    ret = grcan_set_txcomplete(candev, 1);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] txcomplete failed\n");
	return;
    }
    printf("[gr740_candriver_startup] disable rxcomplete\n");
    ret = grcan_set_rxcomplete(candev, 0);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] rxcomplete failed\n");
	return;
    }
    printf("[gr740_candriver_startup] enable blocking write\n");
    ret = grcan_set_txblock(candev, 1);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] txblock failed\n");
	return;
    }
    printf("[gr740_candriver_startup] enable non-blocking read\n");
    ret = grcan_set_rxblock(candev, 0);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] rxblock failed\n");
	return;
    }
    printf("[gr740_candriver_startup] reset stats\n");
    ret = grcan_clr_stats(candev);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] clearing stats failed\n");
	return;
    }
    printf("[gr740_candriver_startup] start device\n");
    ret = grcan_start(candev);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] could not start device\n");
	return;
    }
    printf("[gr740_candriver_startup] initialization complete :)\n");
}

void gr740_candriver_PI_update()
{
    int cnt;
    CANMsg request;
    CANMsg response;

    /* 
        Setup CAN message to request sample updates 
	To get telemetry, we have to request it by a CAN message with ID='0x7C0', RTR=1, LEN=0
	Refer to ESROCOS-Bridget CANbus ICD
    */
    if (!requestTransmitted)
    {
        request.extended = 0;
        request.rtr = 1;
        request.id = 0x7C0;
        request.len = 0;
        printf("[gr740_candriver_PI_update] Transmitting telemetry request\n");
        cnt = grcan_write(candev, &request, 1);
        if (!cnt)
        {
            printf("[gr740_candriver_PI_update] Transmission failed\n");
            return;
        }
        requestTransmitted = true;
	responseReadAttempts = 0;
        return;
    }

    /*
	If we could transmit a request we have to wait for the DEVICE to respond.
	According to the ICD, we will get ID='0x1A0' RTR=0 LEN=6 T0,T1,P0,P1,V0,V1 where only P0 and P1 will be populated
	P0 and P1 are positions where 0 equals to 0 deg and 0xFFFF equals to 360 deg
    */
    if (requestTransmitted)
    {
        printf("[gr740_candriver_PI_update] Waiting for telemetry\n");
	cnt = grcan_read(candev, &response, 1);
        if (cnt < 1)
	{
            responseReadAttempts++;
	    if (responseReadAttempts >= CANDRIVER_MAX_READ_ATTEMPTS)
	    {
		printf("[gr740_candriver_PI_update] No response after %u read attempts. Trying to retransmit\n", CANDRIVER_MAX_READ_ATTEMPTS);
		requestTransmitted = false;
	    }
	    return;
	}
        printf("[gr740_candriver_PI_update] Got a message. Checking ...\n");
	if (response.extended != 0)
	{
	    printf("[gr740_candriver_PI_update] Extended ID not supported. Another device present?\n");
	    return;
	}
	if (response.id != 0x1A0)
	{
	    printf("[gr740_candriver_PI_update] Wrong ID. Another device present?\n");
	    return;
	}
	if (response.rtr != 0)
	{
	    printf("[gr740_candriver_PI_update] RTR set. Bad device?\n");
	    return;
	}
	if (response.len != 6)
	{
	    printf("[gr740_candriver_PI_update] Got length %u. Bad device?\n", response.len);
	    return;
	}
        printf("[gr740_candriver_PI_update] Current positions: %u %u\n", response.data[0], response.data[1]);
        requestTransmitted = false;
    }
}

void gr740_candriver_PI_commands(const asn1SccBase_commands_Joints *IN_cmds)
{
    /*
	Whenever we get an incoming joint command, we have to pass this information to the CAN device.
        According to the ICD, we have to transmit a frame with ID=0x182 RTR=0 LEN=3 MODE,DEMAND_0,DEMAND_1
	MODE is either 0 for idle or 2 vor velocity control
	DEMAND_0 and DEMAND_1 are velocity reference values where 0 means -1 rpm and 0xFFFF means +1 rpm
    */
    int cnt;
    CANMsg request;
    request.extended = 0;
    request.rtr = 0;
    request.id = 0x182;
    request.len = 3;
    request.data[0] = 2; /* Mode: Velocity control */
    request.data[1] = 0; /* Velocity 0 */
    request.data[2] = 0; /* Velocity 1 */
    printf("[gr740_candriver_PI_commands] Transmitting new commands\n");
    cnt = grcan_write(candev, &request, 1);
    if (!cnt)
    {
        printf("[gr740_candriver_PI_commands] Transmission failed\n");
        return;
    }
}

