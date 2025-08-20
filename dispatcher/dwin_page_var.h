/**
 * @file dwin_page_var.h
 * @brief 基于界面的迪文屏变量处理逻辑
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
#ifndef __DWIN_PAGE_VAR_H__
#define __DWIN_PAGE_VAR_H__

/*============================ INCLUDES ======================================*/
#include <rtthread.h>
#include <rtdevice.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================ MACROS ========================================*/
/*============================ TYPES =========================================*/
typedef void (*dwin_page_show_fun)(void);

typedef struct one_page_info
{
	rt_uint16_t page_id;			/**< 本界面id */
	rt_uint16_t start_index;		/**< 本界面第一个变量下标 */
	rt_uint16_t var_address;		/**< 本界面第一个变量地址（迪文屏地址） */
	rt_uint16_t count;				/**< 本界面显示变量个数 */
	dwin_page_show_fun show_fun;	/**< 本界面其它显示处理函数 */
}one_page_info_t;
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
void init_dwin_var(rt_uint16_t *var_list, rt_uint16_t var_count, one_page_info_t *page_list, rt_uint16_t page_count);

void set_current_page_id(rt_uint16_t current_page_id);
rt_uint16_t get_current_page_id(void);
/*============================ INCLUDES ======================================*/

#ifdef __cplusplus
}
#endif

#endif /* __DWIN_PAGE_VAR_H__ */
