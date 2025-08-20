/**
 * @file interface_dwin.h
 * @brief 迪文屏串口输入输出
 * @author Hao xaiofeng 
 * @version 1.0.0
 * @date 2025-1-06
 *
 * @Copyright (c) 2023, PLKJ Development Team, All rights reserved.
 *
 * @par Change Logs:
 * Date			Author		Notes
 * 2025-1-06	Hao xaiofeng 		初始版本
 */
#ifndef __INTERFACE_DWIN_H__
#define __INTERFACE_DWIN_H__

/*============================ INCLUDES ======================================*/
#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
#define DWIN_DATA_FRAME_MAX_LENGTH		255
#define DWIN_DATA_BYTE_COUNT_INDEX		2
#define DWIN_DATA_FRAME_ADDRESS_INDEX	4
#define DWIN_DATA_COUNT_INDEX			6
#define DWIN_DATA_OFFSET_INDEX			7

#define DWIN_WRITE_DATA_OFFSET			6

#define DWIN_COMMAND_WRITE				0x82
#define DWIN_COMMAND_READ				0x83

#define SWAP_16(val)	((((val) & 0xFF) << 8) | (((val) >> 8) & 0xFF))
/*============================ TYPES =========================================*/
typedef struct dwin_data_frame_format
{
	rt_uint16_t head;
	rt_uint8_t byte_count;
	rt_uint8_t command;
	rt_uint16_t var_address;
}dwin_data_frame_format_t;
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
void init_dwin_serial(void);
void dwin_serial_send(rt_uint8_t *buff, rt_uint32_t size);
/*============================ INCLUDES ======================================*/

#ifdef __cplusplus
}
#endif

#endif /* __INTERFACE_DWIN_H__ */
