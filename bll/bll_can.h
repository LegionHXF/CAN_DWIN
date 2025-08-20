/**
 * @file bll_can.h
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
#ifndef __BLL_CAN_H__
#define __BLL_CAN_H__

/*============================ INCLUDES ======================================*/
#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
#define DWIN_DATA_FRAME_RUN_PROGRESS_INDEX	0
#define DWIN_DATA_FRAME_SELF_ACC_INDEX		1
#define DWIN_DATA_FRAME_STEERING_INDEX		2
#define DWIN_DATA_FRAME_YAW_INDEX			3
#define DWIN_DATA_FRAME_TORQUE_INDEX		4
#define DWIN_DATA_FRAME_SELF_SPEED_INDEX	5
#define DWIN_DATA_FRAME_QUALITY_INDEX		6
#define DWIN_DATA_FRAME_SLOPE_INDEX			7
#define DWIN_DATA_FRAME_LIGHT_INDEX			8
#define DWIN_DATA_FRAME_GEAR_INDEX			9

#define DWIN_DATA_RUN_PROGRESS_ADDRESS		0x5000
#define DWIN_DATA_SELF_ACC_ADDRESS			0x5001
#define DWIN_DATA_STEERING_ADDRESS			0x5002 // 方向盘转角
#define DWIN_DATA_YAW_ADDRESS				0x5003
#define DWIN_DATA_TORQUE_ADDRESS			0x5004
#define DWIN_DATA_SELF_SPEED_ADDRESS		0x5005
#define DWIN_DATA_QUALITY_ADDRESS			0x5006
#define DWIN_DATA_SLOPE_ADDRESS				0x5007
#define DWIN_DATA_LIGHT_ADDRESS				0x5008
#define DWIN_DATA_GEAR_ADDRESS				0x5009
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
void init_bll_can(void);

void set_dwin_var_value(rt_uint16_t var_index, rt_uint16_t value);
rt_uint16_t get_dwin_var_value(rt_uint16_t var_index);
/*============================ INCLUDES ======================================*/

#ifdef __cplusplus
}
#endif

#endif /* __BLL_CAN_H__ */
