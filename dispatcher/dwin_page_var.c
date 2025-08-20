/**
 * @file dwin_page_var.c
 * @brief 基于界面的迪文屏变量处理逻辑
 * @author Hao xaiofeng 
 * @version 1.0.0
 * @date 2025-02-04
 *
 * @Copyright (c) 2023, PLKJ Development Team, All rights reserved.
 *
 * @par Change Logs:
 * Date			Author		Notes
 *  2025-02-04	Hao xaiofeng 		初始版本
 */
/*============================ INCLUDES ======================================*/
#include <board.h>

#include "interface_dwin.h"
#include "interface_curve.h"
#include "dwin_page_var.h"

#define DBG_LEVEL	DBG_LOG
#define DBG_TAG		"dwin_page_var"
#include <rtdbg.h>

/*============================ MACROS ========================================*/
#define DWIN_VAR_SHOW_THREAD_STACK_SIZE		1024
#define DWIN_VAR_SHOW_THREAD_PRO			20
#define DWIN_VAR_SHOW_THREAD_SECTION		20
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
typedef struct dwin_var_info
{
	rt_uint16_t *var_list;			/**< 所有迪文屏显示变量数据 */
	rt_uint16_t var_count;			/**< 变量个数 */
	
	one_page_info_t *page_list;		/**< 迪文屏界面列表 */
	rt_uint16_t page_count;			/**< 界面个数 */
}dwin_var_info_t;

static dwin_var_info_t dwin_var;
static volatile rt_uint16_t page_id;	/**< 当前界面id */

static rt_uint8_t dwin_command_buff[DWIN_DATA_FRAME_MAX_LENGTH] =
{
	0x5A, 0xA5,			/**< 迪文数据头部 */
	0x00,				/**< 字节个数 */
	DWIN_COMMAND_WRITE,	/**< 写指令 */
	0x00, 0x00,			/**< 变量地址 */
};
/*============================ PROTOTYPES ====================================*/
/*============================ INTERNAL IMPLEMENTATION =======================*/
static rt_int16_t prepared_dwin_data_frame(const one_page_info_t *one_page)
{
	dwin_data_frame_format_t *dwin_data_frame_format;
	
	dwin_data_frame_format = (dwin_data_frame_format_t *) dwin_command_buff;
	dwin_data_frame_format->command = DWIN_COMMAND_WRITE;
	dwin_data_frame_format->var_address = SWAP_16(one_page->var_address);
	dwin_data_frame_format->byte_count = (rt_uint8_t) (one_page->count * 2 + 3);
	
	rt_memcpy((rt_uint8_t *) (dwin_command_buff + DWIN_WRITE_DATA_OFFSET), 
		(rt_uint8_t *) (dwin_var.var_list + one_page->start_index), one_page->count * 2);
	
	return one_page->count * 2 + 6;
}
/**
 * @brief  曲线窗口显示逻辑
			当前曲线窗口id为-1，则，结束。
			当前曲线窗口的first_show为TRUE，则，显示所有数据；
			否则，显示新到数据。
 * 
 */
static void show_current_curve_window(void)
{
	rt_int16_t current_curve_window_id;
	
	current_curve_window_id = get_current_curve_window();
	if (current_curve_window_id == -1)
	{
		return;
	}
	
	if (is_curve_window_first_show(current_curve_window_id)) 	// 如果是第一次显示 
	{
		clean_curve();
		// show curve all data  	 显示所有曲线数据 
		curve_show(current_curve_window_id, RT_TRUE);
		set_curve_window_not_first_show(current_curve_window_id);
	}
	else
	{
		// show curve new data 显示新增的曲线  
		curve_show(current_curve_window_id, RT_FALSE);
	}
}

static void dwin_var_show_thread(void *arg)
{
	int i;
	int len;

	while (1)
	{
		for (i = 0; i < dwin_var.page_count; i++)
		{
			if (page_id == dwin_var.page_list[i].page_id)
			{
				len = prepared_dwin_data_frame(&dwin_var.page_list[i]);
				dwin_serial_send(dwin_command_buff, len);
				
				dwin_var.page_list[i].show_fun();
				
				// 显示曲线
				show_current_curve_window();
			}
		}
	}
}

/*============================ EXTERNAL IMPLEMENTATION =======================*/
void init_dwin_var(rt_uint16_t *var_list, rt_uint16_t var_count, one_page_info_t *page_list, rt_uint16_t page_count)
{
	rt_thread_t thread;
	
	dwin_var.var_list = var_list;
	dwin_var.var_count = var_count;
	
	dwin_var.page_list = page_list;
	dwin_var.page_count = page_count;
	
	thread = rt_thread_create("DWIN_SHOW", dwin_var_show_thread, RT_NULL,
			DWIN_VAR_SHOW_THREAD_STACK_SIZE,
			DWIN_VAR_SHOW_THREAD_PRO + 1,
			DWIN_VAR_SHOW_THREAD_SECTION);
	if (thread == RT_NULL)
	{
		LOG_E("DWIN SHOW thread failure!");
		RT_ASSERT(0);
	}
	
	rt_thread_startup(thread);
}

void set_current_page_id(rt_uint16_t current_page_id)
{
	page_id = current_page_id;
}

rt_uint16_t get_current_page_id(void)
{
	return page_id;
}
