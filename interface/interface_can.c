/**
 * @file interface_can.c
 * @brief CAN接口功能实现：CAN初始化、CAN数据侦听线程、CAN数据发送
 * @author Hao xaiofeng 
 * @version 1.0.0
 * @date 2025-1-04
 *
 * @Copyright (c) 2023, PLKJ Development Team, All rights reserved.
 *
 * @par Change Logs:
 * Date			Author		Notes
 * 2025-1-04 	Hao xaiofeng 		初始版本
 */
/*============================ INCLUDES ======================================*/
#include <rtthread.h>
#include <rtdevice.h>

#include "interface_can.h"
#include "dispatcher_can_dwin.h"

#define DBG_LEVEL	DBG_LOG
#define DBG_TAG		"interface_can"
#include <rtdbg.h>

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
static struct 
{
	rt_device_t device;
	struct rt_completion cpt;
}interface_can;
/*============================ PROTOTYPES ====================================*/
/*============================ INTERNAL IMPLEMENTATION =======================*/
static rt_err_t can_rx_callback(rt_device_t dev, rt_size_t size)
{
	rt_completion_done(&interface_can.cpt);
	
	return RT_EOK;
}

static void can_rx_thread(void *parameter)
{
	static struct rt_can_msg can_receive_msg;
	
	while (1)
	{
		/*====================== 不过滤 ==========================*/
		can_receive_msg.hdr_index = -1;
		/*====================== 阻塞等待接收完成 ==========================*/
		rt_completion_wait(&interface_can.cpt, RT_WAITING_FOREVER);
		rt_device_read(interface_can.device, 0, &can_receive_msg, sizeof(struct rt_can_msg));
		// 将CAN数据帧中的数据，交给“分发器”处理
		can_data_parser(can_receive_msg.id, can_receive_msg.data, can_receive_msg.len);
#if 0		
		{
			int i;
			// 回环测试
			can_send(can_receive_msg.id, can_receive_msg.data, can_receive_msg.len);
			rt_kprintf("Received CAN data frame:(%04X) ->", can_receive_msg.id);
			for (i = 0; i < can_receive_msg.len; i++)
			{
				rt_kprintf(" %02X", can_receive_msg.data[i]);
			}
		}
#endif
	}
}

/*============================ EXTERNAL IMPLEMENTATION =======================*/
void init_can(void)
{
	rt_err_t res;
	rt_thread_t thread;
	
	interface_can.device = rt_device_find(INTERFACE_CFG_CAN_NAME);
	if (interface_can.device == RT_NULL)
	{
		LOG_E("%s not found!", INTERFACE_CFG_CAN_NAME);
		RT_ASSERT(0);
	}
	res = rt_device_open(interface_can.device, RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_INT_RX);
	if (res != RT_EOK)
	{
		LOG_E("CAN open failure!");
		RT_ASSERT(0);
	}
	res = rt_device_control(interface_can.device, RT_CAN_CMD_SET_BAUD, (void *)CAN500kBaud);
	if (res != RT_EOK)
	{
		LOG_E("CAN config failure!");
		RT_ASSERT(0);
	}
	rt_device_set_rx_indicate(interface_can.device, can_rx_callback);
	
	rt_completion_init(&interface_can.cpt);
	
	thread = rt_thread_create("CAN_RX", can_rx_thread, RT_NULL, 
			INTERFACE_CFG_CAN_THREAD_SIZE,
			INTERFACE_CFG_CAN_THREAD_PRO,
			INTERFACE_CFG_CAN_THREAD_CPU_SECTION);
	if (thread == RT_NULL)
	{
		LOG_E("CAN listener thread failure!");
		RT_ASSERT(0);
	}
	rt_thread_startup(thread);
}

rt_err_t can_send(rt_uint32_t id, rt_uint8_t *buff, rt_size_t size)
{
	static struct rt_can_msg can_msg;
	int i;
	rt_uint16_t len;
	rt_uint16_t length = sizeof(struct rt_can_msg);
	
	can_msg.id = id;
	can_msg.ide = RT_CAN_STDID;     /**< 标准格式 */
	can_msg.rtr = RT_CAN_DTR;       /**< 数据帧 */
	can_msg.len = size;
	
	for (i = 0; i < size; i++)
	{
		can_msg.data[i] = buff[i];
	}
	
	len = rt_device_write(interface_can.device, 0, &can_msg, length);
	
	if (len != length)
	{
		LOG_I("can data %d send failure (%d / %d)!", id, len, length);
		return RT_ERROR;
	}
	
	return RT_EOK;
}
