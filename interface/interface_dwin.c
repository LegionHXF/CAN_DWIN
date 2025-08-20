/**
 * @file interface_dwin.c
 * @brief 迪文屏串口输入输出
 * @author Hao xaiofeng 	
 * @version 1.0.0
 * @date 2025-1-06	
 *
 * @Copyright (c) 2023, PLKJ Development Team, All rights reserved.
 *
 * @par Change Logs:
 * Date			Author		Notes
 * 2025-1-06	Hao xaiofeng 		初始版本
 */
/*============================ INCLUDES ======================================*/
#include <board.h>
#include <string.h>

#include "interface_dwin.h"
#include "dispatcher_can_dwin.h"

#define DBG_LEVEL	DBG_LOG
#define DBG_TAG		"interface_dwin"
#include <rtdbg.h>

/*============================ MACROS ========================================*/
#define INTERFACE_DWIN_SERIAL_NAME					"uart3"
#define INTERFACE_DWIN_SERIAL_THREAD_STACK_SIZE		1024
#define INTERFACE_DWIN_SERIAL_THREAD_PRO			20
#define INTERFACE_DWIN_SERIAL_THREAD_SEC			20
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
static struct 
{
	rt_device_t device;
	struct rt_completion cpt;
}interface_dwin_serial;

/*============================ PROTOTYPES ====================================*/
/*============================ INTERNAL IMPLEMENTATION =======================*/

static void collect_dwin_data_frame(rt_uint8_t *data_frame, rt_uint32_t size)
{
	static rt_uint8_t buffer[DWIN_DATA_FRAME_MAX_LENGTH + 1];
	static rt_uint32_t len = 0;
	rt_uint16_t address;
	rt_uint8_t data_count;
	
	memcpy(buffer + len, data_frame, size);
	len += size;
	
	if (len < 3)
	{
		return;
	}
	
	if (((rt_uint32_t) buffer[2]) + 3 == len)
	{
		// 得到完整数据帧，可以进行下一步处理
		// 将“自动上传数据”交给分发器处理。
#if 0
		{
			// 其实，现在的处理方案是存在问题的。需要先直接输出所接收到的数据，进行观察：
			int i;
			
			rt_kprintf("\nReceive auto upload data from DWIN: %d\n", len);
			for (i = 0; i < len; i++)
			{
				rt_kprintf("%02X ", buffer[i]);
			}
			rt_kprintf("\n");
			// 通过上面的验证发现，上传数据有可能被分割成不同的片段，使得程序无法完整处理所有上传数据。
		}
#endif
		address = *((rt_uint16_t *) (buffer + DWIN_DATA_FRAME_ADDRESS_INDEX));
		LOG_I("DWIN data frame address: %04X", address); // 取出来是小端模式   0A50 
		data_count = *((rt_uint8_t *) (buffer + DWIN_DATA_COUNT_INDEX));
		dwin_auto_load_data_parser(address, buffer + DWIN_DATA_OFFSET_INDEX, data_count);

		len = 0;
	}
}

static void dwin_serail_rx_dealer(void *arg)
{
	static rt_uint8_t rx_buffer[BSP_UART3_RX_BUFSIZE + 1];
	rt_uint16_t len;
	
	while (1)
	{
		rt_completion_wait(&interface_dwin_serial.cpt, RT_WAITING_FOREVER);
		len = rt_device_read(interface_dwin_serial.device, 0, rx_buffer, BSP_UART3_RX_BUFSIZE);
		// 接收迪文屏串口数据，这是“自动上传”类型的数据，需要进一步处理。
		collect_dwin_data_frame(rx_buffer, len);
		 #if 0  
        {
            int i ; 
            rt_kprintf("diwen serial rx len = %d\n",len);
            for(i=0;i<len;i++)
            {
                rt_kprintf("%02x ",rx_buffer[i]);
            }
        }
       #endif
	}
}

static rt_err_t dwin_serial_rx_callback(rt_device_t deveice, rt_size_t size)
{
	rt_completion_done(&interface_dwin_serial.cpt);
	
	return RT_EOK;
}
/*============================ EXTERNAL IMPLEMENTATION =======================*/
void init_dwin_serial(void)
{
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
	rt_err_t result;
	rt_thread_t thread;

	interface_dwin_serial.device = rt_device_find(INTERFACE_DWIN_SERIAL_NAME);
	RT_ASSERT(interface_dwin_serial.device != RT_NULL);
	result = rt_device_control(interface_dwin_serial.device, RT_DEVICE_CTRL_CONFIG, &config);
	RT_ASSERT(result == RT_EOK);
	result = rt_device_open(interface_dwin_serial.device, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING);
	RT_ASSERT(result == RT_EOK);
	
	rt_completion_init(&interface_dwin_serial.cpt);
	
	result = rt_device_set_rx_indicate(interface_dwin_serial.device, dwin_serial_rx_callback);
	RT_ASSERT(result == RT_EOK);
	
	thread = rt_thread_create("DWIN_RX", dwin_serail_rx_dealer, RT_NULL, 
			INTERFACE_DWIN_SERIAL_THREAD_STACK_SIZE,
			INTERFACE_DWIN_SERIAL_THREAD_PRO,
			INTERFACE_DWIN_SERIAL_THREAD_SEC);
	RT_ASSERT(thread != RT_NULL);
	rt_thread_startup(thread);
}

void dwin_serial_send(rt_uint8_t *buff, rt_uint32_t size)
{
	// UART3发送的串口数据，会被迪文屏所接收！
	rt_device_write(interface_dwin_serial.device, 0, buff, size);
}
