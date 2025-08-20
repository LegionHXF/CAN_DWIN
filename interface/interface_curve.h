/**
 * @file interface_curve.h
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
#ifndef __INTERFACE_CURVE_H__
#define __INTERFACE_CURVE_H__

/*============================ INCLUDES ======================================*/
#include <rtthread.h>
#include <rtdevice.h>

#define DWIN_CURVE_DATA_MAX_COUNT	20   // 曲线允许的最大数据量  20个2字节 
#define MAX_COUNT DWIN_CURVE_DATA_MAX_COUNT
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
#define DWIN_CURVE_MAX_COUNT			32  // 曲线 最大 数量  
#define DWIN_CURVE_WINDOW_MAX_COUNT		16 // 曲线窗口 最大 数量
#define DWIN_CURVE_IN_WINDOW_MAX_COUNT	8  // 单曲线窗口中曲线 最大 数量
#define DWIN_CURVE_CHANNEL_MAX_COUNT	8 	// 曲线通道 最大 数量

#define DWIN_CURVE_CHANNEL1		0x0301
#define DWIN_CURVE_CHANNEL2		0x0303
#define DWIN_CURVE_CHANNEL3		0x0305
#define DWIN_CURVE_CHANNEL4		0x0307
#define DWIN_CURVE_CHANNEL5		0x0309
#define DWIN_CURVE_CHANNEL6		0x030B
#define DWIN_CURVE_CHANNEL7		0x030D
#define DWIN_CURVE_CHANNEL8		0x030F

#define IS_DWIN_CURVE_CHANNEL(val)	(DWIN_CURVE_CHANNEL1 == (val) \
	|| DWIN_CURVE_CHANNEL2 == (val) \
	|| DWIN_CURVE_CHANNEL3 == (val) \
	|| DWIN_CURVE_CHANNEL4 == (val) \
	|| DWIN_CURVE_CHANNEL5 == (val) \
	|| DWIN_CURVE_CHANNEL6 == (val) \
	|| DWIN_CURVE_CHANNEL7 == (val) \
	|| DWIN_CURVE_CHANNEL8 == (val) \
	)
/*============================ TYPES =========================================*/
typedef rt_uint16_t (*curve_data_adjust_t)(rt_uint16_t value); //曲线的纵坐标的取值范围 和 曲线数值不相同 ， 所以进行数值转换  


/**
 * @brief 曲线属性及其实现 
 * 
 */
typedef struct curve_data
{
	struct curve_data_queue queue;	// 曲线数据队列
	rt_uint16_t curve_channel;		// 曲线通道
	rt_mutex_t mutex;				// 互斥量
	curve_data_adjust_t adjust_fun;	// 数值转换函数
}curve_data_t;


/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/


void init_curve(rt_uint16_t curve_id, rt_uint16_t curve_channel, curve_data_adjust_t adjust_fun); 	// 初始化曲线  
void add_curve_to_window(rt_uint16_t curve_id, rt_uint16_t curve_window_id); 	// 添加曲线到曲线窗口 
void add_curve_data(rt_uint16_t curve_id, rt_uint16_t data); 	// 添加曲线数据

void set_current_curve_window(rt_int16_t curve_window_id);
rt_int16_t get_current_curve_window(void);

rt_bool_t is_curve_window_first_show(rt_int16_t curve_window_id);
void set_curve_window_not_first_show(rt_int16_t curve_window_id);

void curve_show(rt_int16_t curve_window_id, rt_bool_t all_data);

rt_uint16_t default_curve_data_adjust(rt_uint16_t data);

void clean_curve(void);


/*============================ INCLUDES ======================================*/

#ifdef __cplusplus
}
#endif

#endif /* __INTERFACE_CURVE_H__ */
