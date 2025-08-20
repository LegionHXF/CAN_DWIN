/**
 * @file interface_can.h
 * @brief CAN接口功能实现：CAN初始化、CAN数据侦听线程、CAN数据发送
 * @author Hao xaiofeng 
 * @version 1.0.0
 * @date2025-1-04
 *
 * @Copyright (c) 2023, PLKJ Development Team, All rights reserved.
 *
 * @par Change Logs:
 * Date			Author		Notes
 * 2025-1-04	Hao xaiofeng 		初始版本
 */
#ifndef __INTERFACE_CAN_H__
#define __INTERFACE_CAN_H__

/*============================ INCLUDES ======================================*/
#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
void init_can(void);
rt_err_t can_send(rt_uint32_t id, rt_uint8_t *buff, rt_size_t size);
/*============================ INCLUDES ======================================*/

#ifdef __cplusplus
}
#endif

#endif /* __INTERFACE_CAN_H__ */
