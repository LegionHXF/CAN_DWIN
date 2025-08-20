/**
 * @file interface_curve.c
 * @brief 迪文屏曲线功能
 * @author Hao xaiofeng 
 * @version 1.0.0
 * @date 2024-01-06
 *
 * @Copyright (c) 2023, PLKJ Development Team, All rights reserved.
 *
 * @par Change Logs:
 * Date			Author		Notes
 * 2024-01-06	Hao xaiofeng 		初始版本
 */
/*============================ INCLUDES ======================================*/
#include <board.h>
#include <string.h>

#include "interface_dwin.h"
#include "interface_curve.h"

#define DBG_LEVEL	DBG_LOG
#define DBG_TAG		"interface_curve"
#include <rtdbg.h>

/*============================ MACROS ========================================*/
#define CLEAN_CURVE_CAHNNEL_INDEX	4
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/


/**4、为了能清空曲线，应该将所有“已使用”曲线通道全部记录下来。实现思路：在初始化曲线时，需要提供该曲线所用通道，然后，记录不重复的曲线通道，即可。
 */ 
static struct curve_clean_info
{
	rt_int16_t curve_channel;
	rt_bool_t used;
}curve_clean_list[DWIN_CURVE_CHANNEL_MAX_COUNT] =
{
	{DWIN_CURVE_CHANNEL1, RT_FALSE},
	{DWIN_CURVE_CHANNEL2, RT_FALSE},
	{DWIN_CURVE_CHANNEL3, RT_FALSE},
	{DWIN_CURVE_CHANNEL4, RT_FALSE},
	{DWIN_CURVE_CHANNEL5, RT_FALSE},
	{DWIN_CURVE_CHANNEL6, RT_FALSE},
	{DWIN_CURVE_CHANNEL7, RT_FALSE},
	{DWIN_CURVE_CHANNEL8, RT_FALSE},
};

static rt_uint8_t clean_curve_command[] = 
{
	0x5A,
	0xA5,
	0x05,
	0x82,
	0x03,		// 曲线通道编号  5A A5 05 82 0301 0000 曲线 0 通道会清除曲线；
	0x00,   //  01 . 03 .05. 09 .0B . 0D .0F  对应通道 
	0x00,
	0x00
};



// 曲线数据帧的的构建 ， 串口指令  数据发送 
static rt_uint8_t curve_data_frame[DWIN_DATA_FRAME_MAX_LENGTH] =
{
	0x5A,
	0xA5,
	0x00,		// 后续字节数	DWIN_DATA_BYTE_COUNT_INDEX
	0x82,
	0x03,	    // 曲线通道编号
	0x10,
	0x5A,
	0xA5,
	0x00,		// 同时写入的曲线通道个数
	0x00,		 
	// 0x00,  表示曲线通道编号
	// 0x00,  表示本次写入的数据个数 是两个字
};
#define CURVE_CHANNEL_COUNT_INDEX	8  // 偏移量   同时写入的曲线通道个数 
#define CURVE_DATA_START_INDEX		10



static rt_uint8_t one_curve_data_buff[DWIN_CURVE_DATA_MAX_COUNT * 2 + 2] =
{
	0x00,		// 曲线通道编号00~07
	0x00,		// 本次写入的数据个数
};
#define CURVE_CHANNEL_ID_INDEX		0  // 偏移量  one_curve_data_buff
#define CURVE_DATA_COUNT_INDEX		1
#define CURVE_DATA_OFFSET_INDEX		2



static curve_data_t curve_list[DWIN_CURVE_MAX_COUNT];		// 所有的曲线列表


/**
 * @brief 曲线窗口属性及其实现
 * 
 */
static struct curve_window
{   
	rt_int16_t curve_index_list[DWIN_CURVE_IN_WINDOW_MAX_COUNT]; // 一个窗口的曲线索引列表
	rt_int16_t curve_count; 	// 一个窗口的曲线个数 
	rt_bool_t first_show; 	// 是否是第一次显示 
}curve_window_list[DWIN_CURVE_WINDOW_MAX_COUNT];


static volatile rt_int16_t current_curve_window_index;  // DWIN_SHOW 线程 
static volatile rt_int16_t last_curve_window_index;
/*============================ PROTOTYPES ====================================*/
/*============================ INTERNAL IMPLEMENTATION =======================*/


/**
 * @brief Get the and adjust curve data object
 * 
 * @param curve_id 
 * @param all 
 * @return rt_uint16_t 
 */


// typedef struct curve_data
// {
// 	struct curve_data_queue queue;	// 曲线数据队列
// 	rt_uint16_t curve_channel;		// 曲线通道
// 	rt_mutex_t mutex;				// 互斥量
// 	curve_data_adjust_t adjust_fun;	// 数值转换函数
// }curve_data_t;
static rt_uint16_t get_and_adjust_curve_data(rt_uint16_t curve_id, rt_bool_t all)
{
	curve_data_t *curve = &curve_list[curve_id]; // curve_data_t *curve = curve_list  + curve_id;  这样的操作就不是单独的复制出去，而是对 curve 进行修改，会直接影响原数组里的数据。
	rt_uint16_t curve_data_count = 0; // 	最终return 返回的曲线数据个数
	rt_uint16_t *curve_data_list;
	rt_uint16_t index;
	
	curve_data_list = (rt_uint16_t *) (one_curve_data_buff + CURVE_DATA_OFFSET_INDEX);  // 取得所有数据 
	rt_mutex_take(curve->mutex, RT_WAITING_FOREVER);  // 这一步是加锁，确保同一时刻只有一个线程能访问 curve->queue。 
	if (all == RT_TRUE)  //  表示是第一次显示    
	{
		curve_data_count = curve_data_queue_get_all_data(&curve->queue, curve_data_list);
	}
	else 
	{
		curve_data_count = curve_data_queue_get_last_data(&curve->queue, curve_data_list);
	}
	rt_mutex_release(curve->mutex);
	
	if (curve_data_count == 0)
	{
		return curve_data_count;
	}
	
	one_curve_data_buff[CURVE_CHANNEL_ID_INDEX] = curve->curve_channel & 0xFF;  // 取出通道编号 
	one_curve_data_buff[CURVE_DATA_COUNT_INDEX] = curve_data_count & 0xFF;      // 取出数据个数 而不是字节数 
	
	for (index = 0; index < curve_data_count; index++)
	{
		curve_data_list[index] = curve->adjust_fun(curve_data_list[index]);
	}
	
	return curve_data_count;
}

/*============================ EXTERNAL IMPLEMENTATION =======================*/

/**
 * @brief 初始化曲线 , 迪文数据帧的构建  
 * 
 * @param curve_window_id  曲线窗口ID 
 * @param all   是否是第一次显示 
 */
void curve_show(rt_int16_t curve_window_id, rt_bool_t all)
{	
	/*
		1.检查曲线窗口的曲线条数 
		2.逐条获取曲线 所有/最新数据
		3.并构建迪文串口指令   
	*/
	rt_uint16_t show_curve_data_count = 0;  // 要显示的曲线数据个数 
	rt_uint16_t show_curve_count = 0;   // 要显示的曲线个数    


	struct curve_window *curve_window;
	curve_window = &curve_window_list[curve_window_id];


	rt_uint16_t index;
	rt_uint16_t one_curve_data_count = 0;  // 本曲线数据个数  
	rt_uint16_t curve_data_offset = CURVE_DATA_START_INDEX; // 数据位的偏移量， 这个偏移量后面会变化随着第二条曲线数数据的增加 
	
	
	show_curve_count = curve_window->curve_count;

	curve_data_frame[CURVE_CHANNEL_COUNT_INDEX] = show_curve_count & 0xFF;
	
	for (index = 0; index < show_curve_count; index++)
	{
		one_curve_data_count = get_and_adjust_curve_data(curve_window->curve_index_list[index], all);
		if (one_curve_data_count <= 0)
		{
			continue;
		}
		// 到这里说明已经取出一条曲线的相关数据，且，已经构成这条曲线的数据
		// 且所形成的曲线数据字节数为：one_curve_data_count * 2 + 2
		// 需要将上述字节内容，复制到curve_data_frame的相关位置。
		// curve_data_frame的相关位置（偏移量）初始值为：CURVE_DATA_START_INDEX
		// 以后，每增加一条curve的数据，偏移量需要加one_curve_data_count * 2 + 2
		rt_memcpy(curve_data_frame + curve_data_offset, one_curve_data_buff, one_curve_data_count * 2 + 2);
		curve_data_offset += one_curve_data_count * 2 + 2;
		
		show_curve_data_count += one_curve_data_count;
	}
	
	if (show_curve_data_count <= 0)
	{
		return;
	}
	
	curve_data_frame[DWIN_DATA_BYTE_COUNT_INDEX] = (curve_data_offset - 3) & 0xFF;
	dwin_serial_send(curve_data_frame, curve_data_offset);
	
/*
	要显示的曲线数据个数 = 0;
	要显示的曲线个数 = 窗口中的曲线个数;
	曲线数据缓存;
	
	for (i = 0; i < 曲线个数; i++)
	{
		本曲线数据个数 = 获取并adjust所有/新增曲线数据();
		if (本曲线数据个数 > 0)
		{
			构建、完善并拼凑迪文曲线命令();
			要显示的曲线数据个数 += 本曲线数据个数;
		}
	}
	
	if (要显示的曲线数据个数 > 0)
		发送迪文曲线串口命令();
		
	手工过程
	将“职责单一”的功能，用函数实现
*/
}

rt_bool_t is_curve_window_first_show(rt_int16_t curve_window_id)
{
	return curve_window_list[curve_window_id].first_show;
}

void set_curve_window_not_first_show(rt_int16_t curve_window_id)
{
	curve_window_list[curve_window_id].first_show = RT_FALSE;
}





/**
 * @brief Set the current curve window object
 * 设置当前曲线窗口对象主要功能
	1.避免重复切换
		如果当前窗口 current_curve_window_index 已经是想切换的窗口 curve_window_id，直接返回，不做任何操作。

	2.维护“第一次显示”标志
		如果 last_curve_window_index 有效（不是 -1），说明上一次窗口存在，就把它的 first_show 置为 RT_TRUE，表示
	它已经显示过一次。这样在后续绘制中，这个窗口不会再被当作第一次显示来处理。

	3.更新“上一次窗口”和“当前窗口”
		如果当前窗口有效（不是 -1），就把它记录到 last_curve_window_index 中。把 current_curve_window_index 
	更新为新的 curve_window_id。
 * @param curve_window_id 
 */
rt_int16_t get_current_curve_window(void)
{
	return current_curve_window_index;
}


void set_current_curve_window(rt_int16_t curve_window_id)
{
	if (current_curve_window_index == curve_window_id)  // 如果当前窗口就是指定的窗口，则直接返回 
	{
		return;
	}
	/*
	1. 上上一次
	*/
	if (last_curve_window_index != -1) // 如果上一次的窗口不为空曲线窗口，则将第一次显示设置为空 , 如果上一次窗口是空的则这样设置第一次显示就没有意义了
	{
		curve_window_list[last_curve_window_index].first_show = RT_TRUE; // 表示已经第一次显示过了 
	}

	if (current_curve_window_index != -1)   // 如果当前窗口不为空曲线窗口，则将第一次显示设置为空
	{
		last_curve_window_index = current_curve_window_index;
	}
	current_curve_window_index = curve_window_id;   // (更新窗口的值)设置当前窗口为指定的窗口  
}




void add_curve_data(rt_uint16_t curve_id, rt_uint16_t data)
{
	if (curve_id >= DWIN_CURVE_MAX_COUNT)
	{
		LOG_W("curve index (%d) too large!", curve_id);
		RT_ASSERT(0);
	}
	
	rt_mutex_take(curve_list[curve_id].mutex, RT_WAITING_FOREVER); 		//获取互斥锁  保证多线程安全   互斥锁，是用于保护共享资源的，当多个线程同时访问共享资源时，互斥锁可以保证同一时间只有一个线程访问共享资源。 
	curve_data_queue_add_data(&curve_list[curve_id].queue, data);
	rt_mutex_release(curve_list[curve_id].mutex);
}






void add_curve_to_window(rt_uint16_t curve_id, rt_uint16_t curve_window_id) // 将曲线id添加到曲线id窗口中  
{
	rt_uint16_t curve_count;
	
	if (curve_id >= DWIN_CURVE_MAX_COUNT)
	{
		LOG_W("curve index (%d) too large!", curve_id);
		RT_ASSERT(0);
	}
	
	if (curve_window_id >= DWIN_CURVE_WINDOW_MAX_COUNT)
	{
		LOG_W("curve window index (%d) too large!", curve_window_id);
		RT_ASSERT(0);
	}
	
	curve_count = curve_window_list[curve_window_id].curve_count; // 获取曲线窗口中的曲线个数 
	curve_window_list[curve_window_id].curve_index_list[curve_count] = curve_id; // 将曲线id添加到曲线窗口中
	++curve_window_list[curve_window_id].curve_count; 	//曲线窗口中的曲线个数加1
}





void clean_curve(void)
{
	int index;
	
	for (index = 0; index < DWIN_CURVE_CHANNEL_MAX_COUNT; index++)
	{
		if (curve_clean_list[index].used == RT_TRUE)
		{
			clean_curve_command[CLEAN_CURVE_CAHNNEL_INDEX] = curve_clean_list[index].curve_channel & 0xFF;
			clean_curve_command[CLEAN_CURVE_CAHNNEL_INDEX + 1] = (curve_clean_list[index].curve_channel >> 8) & 0xFF;
#if 0			
			{
				int i;
				char str[128];
				char tmp[16];
				
				rt_sprintf(str, "clean command :");
				for (i = 0; i < sizeof(clean_curve_command); i++)
				{
					rt_sprintf(tmp, " %02X", clean_curve_command[i]);
					strcat(str, tmp);
				} 
				LOG_I(str); 
			}
#endif
			dwin_serial_send(clean_curve_command, sizeof(clean_curve_command));
			// 串口数据的发送，实际上是通过rt_thread提供的底层操作完成的。
			// 有可能是通过线程实现的，也有可能是通过中断实现的；
			// 串口数据发送是耗时的，因此，需要适当的延时。
			rt_thread_mdelay(1);
		}
	}
}







void init_curve(rt_uint16_t curve_id, rt_uint16_t curve_channel, curve_data_adjust_t adjust_fun)
{	
	//  curve_id 要在 0 - DWIN_CURVE_MAX_COUNT 之间     curve_channel 要在 DWIN_CURVE_CHANNEL1 - DWIN_CURVE_CHANNEL8 之间 
	rt_uint16_t index;
	
	if (curve_id >= DWIN_CURVE_MAX_COUNT)
	{
		LOG_W("curve index (%d) too large!", curve_id);
		RT_ASSERT(0);
	}
	
	if (!IS_DWIN_CURVE_CHANNEL(curve_channel))
	{
		LOG_W("error curve channel : %04X", curve_channel);
		RT_ASSERT(0);
	}
	
	for (index = 0; index < DWIN_CURVE_CHANNEL_MAX_COUNT; index++)
	{
		if (curve_clean_list[index].curve_channel == curve_channel)
		{
			curve_clean_list[index].used = RT_TRUE;
			break;
		}
	}
	
	curve_data_queue_init(&curve_list[curve_id].queue);   // 初始化曲线数据队列
	curve_list[curve_id].curve_channel = ((curve_channel & 0x0F) - 1) >> 1; // 初始化曲线通道 0309 - 3 通道 的转换   
	curve_list[curve_id].mutex = rt_mutex_create("CURVE", RT_IPC_FLAG_PRIO); // 初始化曲线数据队列互斥锁
	
	if (adjust_fun == RT_NULL)
	{
		adjust_fun = default_curve_data_adjust;
	}
	curve_list[curve_id].adjust_fun = adjust_fun;
}





rt_uint16_t default_curve_data_adjust(rt_uint16_t data)
{
	return data;
}
