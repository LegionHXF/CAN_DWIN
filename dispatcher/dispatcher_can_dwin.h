/**
 * @file dispatcher_can_dwin.h
 * @brief 代码功能简介
 * @author Hao xaiofeng 
 * @version 1.0.0
 * @date  2025-02-04
 *
 * @Copyright (c) 2023, PLKJ Development Team, All rights reserved.
 *
 * @par Change Logs:
 * Date			Author		Notes
 *  2025-02-04	Hao xaiofeng 		初始版本
 */
#ifndef __DISPATCHER_CAN_DWIN_H__
#define __DISPATCHER_CAN_DWIN_H__

/*============================ INCLUDES ======================================*/
#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/
typedef void (*can_data_parser_hook)(rt_uint32_t id, rt_uint8_t *buff, rt_size_t size);
typedef void (*dwin_data_parser_hook)(rt_uint16_t address, rt_uint8_t *buff, rt_size_t size);



typedef struct can_dispatcher
{
	rt_uint32_t id;
	can_data_parser_hook hook;
}can_dispatcher_t;

typedef struct dwin_dispatcher
{
	rt_uint32_t address;
	dwin_data_parser_hook hook;
}dwin_dispatcher_t;
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
void init_can_dispatcher(can_dispatcher_t *list, rt_size_t count);
void init_dwin_dispatcher(dwin_dispatcher_t *list, rt_size_t count);

void can_data_parser(rt_uint32_t id, rt_uint8_t *buff, rt_size_t size);
void dwin_auto_load_data_parser(rt_uint16_t address, rt_uint8_t *buff, rt_size_t size);

void default_can_data_parser(rt_uint32_t id, rt_uint8_t *buff, rt_size_t size);
void default_dwin_auto_load_data_parser(rt_uint16_t address, rt_uint8_t *buff, rt_size_t size);
/*============================ INCLUDES ======================================*/

#ifdef __cplusplus
}
#endif

#endif /* __DISPATCHER_CAN_DWIN_H__ */
