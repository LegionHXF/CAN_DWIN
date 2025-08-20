/**
 * @file dispatcher_can_dwin.c
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
/*============================ INCLUDES ======================================*/
#include <string.h>
#include <board.h>

#include "interface_dwin.h"
#include "dispatcher_can_dwin.h"

#define DBG_LEVEL	DBG_LOG
#define DBG_TAG		"dispatcher_can_dwin"
#include <rtdbg.h>

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
static struct
{
	can_dispatcher_t *list;
	rt_size_t count;
}can_dispatcher_tab;

static struct
{
	dwin_dispatcher_t *list;
	rt_size_t count;
}dwin_auto_load_dispatcher_tab;
/*============================ PROTOTYPES ====================================*/
/*============================ INTERNAL IMPLEMENTATION =======================*/
/*============================ EXTERNAL IMPLEMENTATION =======================*/
void init_can_dispatcher(can_dispatcher_t *list, rt_size_t count)
{
	can_dispatcher_tab.list = list;
	can_dispatcher_tab.count = count;
}

void init_dwin_dispatcher(dwin_dispatcher_t *list, rt_size_t count)
{
	dwin_auto_load_dispatcher_tab.list = list;
	dwin_auto_load_dispatcher_tab.count = count;
}

void can_data_parser(rt_uint32_t id, rt_uint8_t *buff, rt_size_t size)
{
	int index;

	for (index = 0; index < can_dispatcher_tab.count; index++)
	{
		if (can_dispatcher_tab.list[index].id == id)
		{
			can_dispatcher_tab.list[index].hook(id, buff, size);
			return;
		}
	}
	
	LOG_I("CAN data (%04X) parser not found!", id);
}

void dwin_auto_load_data_parser(rt_uint16_t address, rt_uint8_t *buff, rt_size_t size)
{
	int index;
	
	LOG_I("Dwin auto load data (%04X) :", address);
	address = SWAP_16(address);
	LOG_I(" SWAP_16 Dwin auto load data (%04X) :", address);
	for (index = 0; index < dwin_auto_load_dispatcher_tab.count; index++)
	{
		if (dwin_auto_load_dispatcher_tab.list[index].address == address)
		{
			dwin_auto_load_dispatcher_tab.list[index].hook(address, buff, size);
			return;
		}
	}
	
	LOG_I("Dwin auto load data (%04X) parser not found!", address);
}

void default_can_data_parser(rt_uint32_t id, rt_uint8_t *buff, rt_size_t size)
{
	char str[512] = {0};
	char tmp[8];
	int i;
	
	rt_sprintf(str, "CAN data (%04X) :", id);
	for (i = 0; i < size; i++)
	{
		rt_sprintf(tmp, " %02X", buff[i]);
		strcat(str, tmp);
	}
	
	LOG_I(str);
}

void default_dwin_auto_load_data_parser(rt_uint16_t address, rt_uint8_t *buff, rt_size_t size)
{
	char str[512] = {0};
	char tmp[8];
	int i;
	
	rt_sprintf(str, "Dwin auto load data (%04X) :", address);
	for (i = 0; i < size * 2; i++)
	{
		rt_sprintf(tmp, " %02X", buff[i]);
		strcat(str, tmp);
	}
	
	LOG_I(str);
}
