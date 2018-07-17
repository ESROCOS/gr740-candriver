/* User code: This file will not be overwritten by TASTE. */

#include "gr740_candriver.h"

/*NOTE: The following is defined in polyorb_hi_c/include/po_hi_c_common.h*/
//#include <rtems.h>
//#define CONFIGURE_INIT
//#include <bsp.h>
//
//#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
//#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
//#define CONFIGURE_RTEMS_INIT_TASK_TABLE
//#define CONFIGURE_MAXIMUM_DRIVERS 32
//#include <rtems/confdefs.h>
//
///* Enable standard drivers */
//#if defined(RTEMS_DRVMGR_STARTUP)
//#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GPTIMER
//#define CONFIGURE_DRIVER_AMBAPP_GAISLER_APBUART
//#endif

///* Enable GRCAN driver */
//#define CONFIGURE_DRIVER_AMBAPP_GAISLER_GRCAN
//#include <drvmgr/drvmgr_confdefs.h>

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
    ret = grcan_set_txcomplete(candev, 1);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] txcomplete failed\n");
	return;
    }
    ret = grcan_set_rxcomplete(candev, 0);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] rxcomplete failed\n");
	return;
    }
    ret = grcan_set_txblock(candev, 1);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] txblock failed\n");
	return;
    }
    ret = grcan_set_rxblock(candev, 0);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] rxblock failed\n");
	return;
    }
    ret = grcan_clr_stats(candev);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] clearing stats failed\n");
	return;
    }
    ret = grcan_start(candev);
    if (ret)
    {
	grcan_close(candev);
	printf("[gr740_candriver_startup] could not start device\n");
	return;
    }
}

void gr740_candriver_PI_update()
{
    /* Write your code here! */
    /* TODO: On update, pass current commands to device and asking for new samples*/
}

void gr740_candriver_PI_commands(const asn1SccBase_commands_Joints *IN_cmds)
{
    /* Write your code here! */
    /* TODO: Store new commands for next update call*/
}

