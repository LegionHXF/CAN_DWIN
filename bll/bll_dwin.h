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
#ifndef __BLL_DWIN_H__
#define __BLL_DWIN_H__

/*============================ INCLUDES ======================================*/
#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
#define CURVE_SELF_SPEED_INDEX		0   //自车速度 
#define CURVE_REAL_ACC_INDEX		1   //实际加速度 
#define CURVE_ESTI_ACC_INDEX		2   //估计加速度

#define CURVE_WINDOW_SELF_SPEED		0
#define CURVE_WINDOW_ACC			1
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
void init_bll_dwin(void);
/*============================ INCLUDES ======================================*/

#ifdef __cplusplus
}
#endif

#endif /* __BLL_DWIN_H__ */
