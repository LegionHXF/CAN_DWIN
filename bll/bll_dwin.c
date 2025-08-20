/**
 * @file bll_dwin.c
 * @brief 业务逻辑 - Dwin数据解析
 * @author Hao xaiofeng 
 * @version 1.0.0
 * @date 2024-03-06
 *
 * @Copyright (c) 2023, PLKJ Development Team, All rights reserved.
 *
 * @par Change Logs:
 * Date			Author		Notes
 * 2024-03-06	Hao xaiofeng 		初始版本
 */
/*============================ INCLUDES ======================================*/
#include <board.h>

#include "bll_dwin.h"
#include "bll_can.h"
#include "interface_can.h"
#include "interface_dwin.h"
#include "interface_curve.h"
#include "dispatcher_can_dwin.h"
#include "dwin_page_var.h"

#define DBG_LEVEL	DBG_LOG
#define DBG_TAG		"bll_dwin"
#include <rtdbg.h>

/*============================ MACROS ========================================*/
#define DWIN_AUTO_LOAD_DATA_SELF_QUALITY	0x500A
#define DWIN_AUTO_LOAD_DATA_SLOPE			0x500B
#define DWIN_AUTO_LOAD_DATA_SELECT_PAGE		0x500C
#define DWIN_AUTO_LOAD_DATA_CURVE_BUTTON	0x500D
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
static rt_uint8_t auto_upload_data[] =
{
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
};

static rt_uint16_t lasted_curve_window_id;
/*============================ PROTOTYPES ====================================*/
/*============================ INTERNAL IMPLEMENTATION =======================*/

static void self_quality_parser(rt_uint16_t address, rt_uint8_t *buff, rt_size_t size) // 本车质量 
{
	rt_uint16_t value;
	
	value = (buff[0] << 8) | buff[1];
	// 滑动刻度取值范围为：0~1000
	// 本车质量取值范围为：0~50000
	value *= 50;
	set_dwin_var_value(DWIN_DATA_FRAME_QUALITY_INDEX, SWAP_16(value));  // 将获取到的值添加到dwin_var_list中，用于更新CAN数据
	// 按CAN格式发送数据到上位机
	
	auto_upload_data[4] = (value >> 8) & 0xFF;
	auto_upload_data[5] = value & 0xFF;
	
	can_send(0x301, auto_upload_data, sizeof(auto_upload_data));
}

static void road_slope_parser(rt_uint16_t address, rt_uint8_t *buff, rt_size_t size)
{
	rt_int16_t value;
	
	value = (buff[0] << 8) | buff[1];
	// 滑动刻度取值范围为：0~1000
	// 道路坡度取值范围为：-90~90
	value = (value - 500) / 500.0 * 90;
	set_dwin_var_value(DWIN_DATA_FRAME_SLOPE_INDEX, SWAP_16(value));
	// 按CAN格式发送数据到上位机
	
	auto_upload_data[6] = value & 0xFF;
	
	can_send(0x301, auto_upload_data, sizeof(auto_upload_data));
}

static void select_page_parser(rt_uint16_t address, rt_uint8_t *buff, rt_size_t size)
{
	rt_uint16_t value;

	value = (buff[0] << 8) | buff[1];

	set_current_page_id(value);
	if (value == 1)    
	{
		set_current_curve_window(-1); // 第二页没有任何曲线  
	}
	else if (value == 0)
	{
		set_current_curve_window(lasted_curve_window_id);
	}
}

static void dwin_cruve_selected(rt_uint16_t address, rt_uint8_t *buff, rt_size_t size)
{
	rt_uint16_t value;

	value = (buff[0] << 8) | buff[1];
	if (value == 1)
	{
		lasted_curve_window_id = CURVE_WINDOW_SELF_SPEED;
		set_current_curve_window(CURVE_WINDOW_SELF_SPEED);
	}
	else if (value == 2)
	{
		lasted_curve_window_id = CURVE_WINDOW_ACC;
		set_current_curve_window(CURVE_WINDOW_ACC);
	}
}

static rt_uint16_t self_speed_adjust(rt_uint16_t value)
{
	value *= 10;
	value = SWAP_16(value);
	
	return value;
}

static rt_uint16_t real_acc_adjust(rt_uint16_t value)
{
	rt_int16_t data = (rt_uint16_t) value;
	data = (data + 2000) / 3;
	data = data > 1000 ? 1000 : data;
	data = SWAP_16(data);
	
	return (rt_uint16_t) data;
}

static rt_uint16_t esti_acc_adjust(rt_uint16_t value)
{
	rt_int16_t data = (rt_uint16_t) value;
	data = (data + 2000) / 3;
	data = data > 1000 ? 1000 : data;
	data = SWAP_16(data);
	
	return (rt_uint16_t) data;
}
/*============================ EXTERNAL IMPLEMENTATION =======================*/
void init_bll_dwin(void)
{
	static dwin_dispatcher_t dwin_dispatcher_pool[] =
	{
		{ DWIN_AUTO_LOAD_DATA_SELF_QUALITY, self_quality_parser},
		{ DWIN_AUTO_LOAD_DATA_SLOPE, 		road_slope_parser},
		{ DWIN_AUTO_LOAD_DATA_SELECT_PAGE, 	select_page_parser},
		{ DWIN_AUTO_LOAD_DATA_CURVE_BUTTON, dwin_cruve_selected},
	};

	init_curve(CURVE_SELF_SPEED_INDEX, 	DWIN_CURVE_CHANNEL1, self_speed_adjust);  // 初始化曲线 队列， 互斥锁 ， 通道 和 曲线数据范围调整函数 
	init_curve(CURVE_REAL_ACC_INDEX, 	DWIN_CURVE_CHANNEL1, real_acc_adjust);
	init_curve(CURVE_ESTI_ACC_INDEX, 	DWIN_CURVE_CHANNEL2, esti_acc_adjust);
	
	add_curve_to_window(CURVE_SELF_SPEED_INDEX,	CURVE_WINDOW_SELF_SPEED);
	add_curve_to_window(CURVE_REAL_ACC_INDEX,	CURVE_WINDOW_ACC);
	add_curve_to_window(CURVE_ESTI_ACC_INDEX,	CURVE_WINDOW_ACC);
	
	set_current_curve_window(CURVE_WINDOW_SELF_SPEED);
	
	init_dwin_serial(); // 创建 DWIN_RX 线程  
	init_dwin_dispatcher(dwin_dispatcher_pool, sizeof(dwin_dispatcher_pool) / sizeof(dwin_dispatcher_t));
}
