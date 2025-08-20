/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-1-01     Hao xaiofeng    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define DBG_LVL		DBG_LOG
#define DBG_TAG	 	"main"
#include <rtdbg.h>

#include "bll_can.h"
#include "bll_dwin.h"

/* defined the USER LED2 pin: PA1 */
#define LED2_PIN               GET_PIN(A, 1)

// static rt_uint32_t cnt;

int main(void)
{
	init_bll_can();
	init_bll_dwin();
	
    /* set LED0 pin mode to output */
    rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);

    while (1)
    {
        rt_pin_write(LED2_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED2_PIN, PIN_LOW);
        rt_thread_mdelay(500);
		
		//LOG_I("%d main thread is running!", ++cnt);
    }
}
