/**
 * @file bll_can.c
 * @brief 业务逻辑 - CAN数据解析
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

#include "interface_can.h"
#include "interface_dwin.h"
#include "interface_curve.h"
#include "dwin_page_var.h"
#include "dispatcher_can_dwin.h"
#include "bll_can.h"
#include "bll_dwin.h"

#define DBG_LEVEL	DBG_LOG
#define DBG_TAG		"bll_can"
#include <rtdbg.h>

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
static rt_uint16_t dwin_var_list[] =
{
	0,				// 运行进程（百分比）	数据显示	输入	0x5000
	0,				// 本车加速度			数据显示	输入	0x5001
	0,				// 方向盘转角			数据显示	输入	0x5002
	0,				// 横摆角速度			数据显示	输入	0x5003
	0,				// 发动机扭矩			数据显示	输入	0x5004
	0,				// 本车车速（文本）		数据显示	输入	0x5005
	SWAP_16(25000),	// 本车质量（文本）1	数据显示	输入	0x5006
	0,				// 道路坡度（文本）1	数据显示	输入	0x5007
	SWAP_16(2),		// 指示灯				位变量图标	输入	0x5008
	SWAP_16(1),		// 挡位P				位变量图标	输入	0x5009
};

/*============================ PROTOTYPES ====================================*/
/*============================ INTERNAL IMPLEMENTATION =======================*/
/**
	CAN数据id : 0x101
	运行进程	0	8	0, 100
	本车车速	1	8	0, 100
	指示灯ABS	2	1	
	指示灯ACC	2	1	
	指示灯Slip	2	1	
	指示灯Lost	2	1	
	挡位P		3	1	
	挡位R		3	1	
	挡位D		3	1	
	挡位N		3	1	
 */
static void can_data_101_parser(rt_uint32_t id, rt_uint8_t *buff, rt_size_t size)
{
	rt_uint16_t value;
	
	value = buff[0];
	dwin_var_list[DWIN_DATA_FRAME_RUN_PROGRESS_INDEX] = SWAP_16(value);
	
	value = buff[1];
	dwin_var_list[DWIN_DATA_FRAME_SELF_SPEED_INDEX] = SWAP_16(value);
	// 添加到曲线数据队列中
	add_curve_data(CURVE_SELF_SPEED_INDEX, value);
	
	value = buff[2];
	dwin_var_list[DWIN_DATA_FRAME_LIGHT_INDEX] = SWAP_16(value);
	
	value = buff[3];
	dwin_var_list[DWIN_DATA_FRAME_GEAR_INDEX] = SWAP_16(value);
}

/*
	CAN数据id : 0x201	
	本车加速度	0	16	-20, 10	0.01
	估计加速度	2	16	-20, 10	0.01
	方向盘转角	4	16	-720, 720	
*/
static void can_data_201_parser(rt_uint32_t id, rt_uint8_t *buff, rt_size_t size)
{
	rt_uint16_t value;

	value = buff[1] | (buff[0] << 8);
	dwin_var_list[DWIN_DATA_FRAME_SELF_ACC_INDEX] = SWAP_16(value);
	// 添加到曲线数据队列
	add_curve_data(CURVE_REAL_ACC_INDEX, value);	 // 实际加速度曲线  先不颠倒 ，因为需要对数值进行转换 
	
	value = buff[3] | (buff[2] << 8);
	// 添加到曲线数据队列
	add_curve_data(CURVE_ESTI_ACC_INDEX, value);  // 估计加速度曲线 
	
	value = buff[5] | (buff[4] << 8);
	dwin_var_list[DWIN_DATA_FRAME_STEERING_INDEX] = SWAP_16(value);
}

/*
	CAN数据id : 0x301
	横摆角速度	0	16	-20, 20	0.1
	发动机扭矩	2	16	-3000, 3000	

	本车质量	4	16	0, 50000	
	道路坡度	6	8	-90, 90	
*/
static void can_data_301_parser(rt_uint32_t id, rt_uint8_t *buff, rt_size_t size)
{
	rt_uint16_t value;
	
	value = buff[1] | (buff[0] << 8);
	dwin_var_list[DWIN_DATA_FRAME_YAW_INDEX] = SWAP_16(value);
	
	value = buff[3] | (buff[2] << 8);
	dwin_var_list[DWIN_DATA_FRAME_TORQUE_INDEX] = SWAP_16(value);
}

static void page_0_show(void)
{
	// 这里实现运行进度条的绘画
	// 迪文屏复制图像命令：5A A5 17 82 变量地址 0006 0001 0002 xss yss xse yse xt yt FF00
	rt_uint8_t data_frame[] =
	{
		0x5A,
		0xA5,
		0x17,
		0x82,
		0x51,	// 显示变量地址
		0x00,
		0x00,	// 图形复制命令
		0x06,
		0x00,	// 复制一个
		0x01,
		0x00,	// 复制页面2
		0x02,
		0x01,	// xss
		0x28,
		0x00,	// yss
		0x00,
		0x01,	// xse
		0xD6,
		0x00,	// yse
		0x23,
		0x00,	// xt
		0x0E,
		0x00,	// yt
		0x0A,
		0xFF,
		0x00,
	};
	rt_uint16_t run_progress;
	rt_uint16_t xss;
	
	run_progress = SWAP_16(dwin_var_list[DWIN_DATA_FRAME_RUN_PROGRESS_INDEX]);
	xss = (1 - run_progress / 100.0) * 470;
	data_frame[12] = xss >> 8;
	data_frame[13] = xss & 0xFF;
	
	dwin_serial_send(data_frame, sizeof(data_frame));
}

/*============================ EXTERNAL IMPLEMENTATION =======================*/
void init_bll_can(void)
{
	static can_dispatcher_t can_dispatcher_pool[] =
	{
		{ 0x101, can_data_101_parser},
		{ 0x201, can_data_201_parser},
		{ 0x301, can_data_301_parser},
	};

	static one_page_info_t dwin_pages[] = 
	{
		{
			0,									// rt_uint16_t page_id
			DWIN_DATA_FRAME_RUN_PROGRESS_INDEX,	// rt_uint16_t start_index
			DWIN_DATA_RUN_PROGRESS_ADDRESS,		// rt_uint16_t var_address
			10,									// rt_uint16_t count
			page_0_show,						//  运行进程进度条展示 复制粘贴的命令   
		},
	};

	init_can(); // 建立CAN数据侦听线程
	init_can_dispatcher(can_dispatcher_pool, sizeof(can_dispatcher_pool) / sizeof(can_dispatcher_t));  // 初始化CAN数据解析器 , 具体的分发逻辑在会被CAN_RX的线程调用   CAN_RX 

	init_dwin_var(dwin_var_list, sizeof(dwin_var_list) / sizeof(rt_uint16_t), dwin_pages, sizeof(dwin_pages) / sizeof(one_page_info_t)); // 将CAN解析的数据进行迪文屏界面显示的线程 DWEIN_SHOW
}

void set_dwin_var_value(rt_uint16_t var_index, rt_uint16_t value)
{
	dwin_var_list[var_index] = value;
}

rt_uint16_t get_dwin_var_value(rt_uint16_t var_index)
{
	return dwin_var_list[var_index];
}
